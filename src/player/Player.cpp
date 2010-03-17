//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2008 Ulrich von Zadow
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  Current versions can be found at www.libavg.de
//

#include "Player.h"

#include "../avgconfigwrapper.h"
#include "AVGNode.h"
#include "DivNode.h"
#include "WordsNode.h"
#include "VideoNode.h"
#include "CameraNode.h"
#include "ImageNode.h"
#include "PanoImageNode.h"
#include "SoundNode.h"
#include "LineNode.h"
#include "RectNode.h"
#include "CurveNode.h"
#include "PolyLineNode.h"
#include "PolygonNode.h"
#include "CircleNode.h"
#include "MeshNode.h"
#include "NodeDefinition.h"
#include "PluginManager.h"
#include "TextEngine.h"
#include "TestHelper.h"
#include "MainScene.h"
#include "OffscreenScene.h"
#include "TrackerEventSource.h"
#include "SDLDisplayEngine.h"

#include "../base/FileHelper.h"
#include "../base/StringHelper.h"
#include "../base/OSHelper.h"
#include "../base/Exception.h"
#include "../base/Logger.h"
#include "../base/ConfigMgr.h"
#include "../base/XMLHelper.h"
#include "../base/Profiler.h"
#include "../base/ScopeTimer.h"
#include "../base/TimeSource.h"
#include "../base/MathHelper.h"

#include "../imaging/Camera.h"

#include "../audio/SDLAudioEngine.h"

#undef HAVE_TEMPNAM
#include <Magick++.h>

#include <libxml/xmlmemory.h>

#ifdef _WIN32
#include <direct.h>
#define getcwd _getcwd
#endif

#include <iostream>

using namespace std;
using namespace boost;

namespace avg {
    
Player * Player::s_pPlayer=0;

Player::Player()
    : m_pDisplayEngine(0),
      m_pAudioEngine(0),
      m_bAudioEnabled(true),
      m_pTracker(0),
      m_bInHandleTimers(false),
      m_bCurrentTimeoutDeleted(false),
      m_bStopOnEscape(true),
      m_bIsPlaying(false),
      m_bFakeFPS(false),
      m_FakeFPS(0),
      m_FrameTime(0),
      m_Volume(1),
      m_dtd(0),
      m_bPythonAvailable(true),
      m_EventHookPyFunc(Py_None)
{
    if (s_pPlayer) {
        throw Exception(AVG_ERR_UNKNOWN, "Player has already been instantiated.");
    }
    ThreadProfilerPtr pProfiler = ThreadProfiler::get();
    pProfiler->setName("main");
    Profiler::get().registerThreadProfiler(pProfiler);
    initConfig();

    // Register all node types
    registerNodeType(AVGNode::createDefinition());
    registerNodeType(SceneNode::createDefinition());
    registerNodeType(DivNode::createDefinition());
    registerNodeType(ImageNode::createDefinition());
    registerNodeType(WordsNode::createDefinition());
    registerNodeType(VideoNode::createDefinition());
    registerNodeType(CameraNode::createDefinition());
    registerNodeType(PanoImageNode::createDefinition());
    registerNodeType(SoundNode::createDefinition());
    registerNodeType(LineNode::createDefinition());
    registerNodeType(RectNode::createDefinition());
    registerNodeType(CurveNode::createDefinition());
    registerNodeType(PolyLineNode::createDefinition());
    registerNodeType(PolygonNode::createDefinition());
    registerNodeType(CircleNode::createDefinition());
    registerNodeType(MeshNode::createDefinition());
    
    m_pTestHelper = new TestHelper();

    // Early initialization of TextEngine singletons (dualton? ;-))
    // to avoid locale clashes with Magick (bug 54)
    TextEngine::get(true);
    TextEngine::get(false);

#ifdef _WIN32
    Magick::InitializeMagick((getAvgLibPath()+"magick\\").c_str());
#endif

    s_pPlayer = this;

    m_CurDirName = getCWD();
}

void deletePlayer()
{
    delete Player::s_pPlayer;
    Player::s_pPlayer = 0;
}

Player::~Player()
{
    if (m_pDisplayEngine) {
        delete m_pDisplayEngine;
    }
    if (m_pAudioEngine) {
        delete m_pAudioEngine;
    }
    if (m_dtd) {
        xmlFreeDtd(m_dtd);
    }
    delete m_pTestHelper;
}

Player* Player::get()
{
    if (!s_pPlayer) {
        s_pPlayer = new Player();
        atexit(deletePlayer);
    }
    return s_pPlayer;
}

bool Player::exists()
{
    return s_pPlayer != 0;
}

void Player::setResolution(bool bFullscreen, int width, int height, int bpp)
{
    m_DP.m_bFullscreen = bFullscreen;
    if (bpp) {
        m_DP.m_BPP = bpp;
    }
    if (width) {
        m_DP.m_WindowSize.x = width;
    }
    if (height) {
        m_DP.m_WindowSize.y = height;
    }
}

void Player::setWindowPos(int x, int y)
{
    m_DP.m_Pos.x = x;
    m_DP.m_Pos.y = y;
}

void Player::setOGLOptions(bool bUsePOTTextures, bool bUseShaders, 
                bool bUsePixelBuffers, int MultiSampleSamples)
{
    m_GLConfig.m_bUsePOTTextures = bUsePOTTextures;
    m_GLConfig.m_bUseShaders = bUseShaders;
    m_GLConfig.m_bUsePixelBuffers = bUsePixelBuffers;
    m_GLConfig.m_MultiSampleSamples = MultiSampleSamples;
}

void Player::setMultiSampleSamples(int MultiSampleSamples)
{
    m_GLConfig.m_MultiSampleSamples = MultiSampleSamples;
}

void Player::enableAudio(bool bEnable)
{
    if (m_bIsPlaying) {
        throw Exception(AVG_ERR_UNSUPPORTED, 
                "Player.enableAudio must be called before Player.play().");
    }
    m_bAudioEnabled = bEnable;
}

void Player::setAudioOptions(int samplerate, int channels)
{
    m_AP.m_SampleRate = samplerate;
    m_AP.m_Channels = channels;
}

ScenePtr Player::loadFile(const string& sFilename)
{
    NodePtr pNode = loadMainNodeFromFile(sFilename);
    if (m_pMainScene) {
        cleanup();
    }

    m_pMainScene = MainScenePtr(new MainScene(this, pNode));
    m_DP.m_Size = m_pMainScene->getSize();
    return m_pMainScene;
}

ScenePtr Player::loadString(const string& sAVG)
{
    if (m_pMainScene) {
        cleanup();
    }

    NodePtr pNode = loadMainNodeFromString(sAVG);
    m_pMainScene = MainScenePtr(new MainScene(this, pNode));
    m_DP.m_Size = m_pMainScene->getSize();
    return m_pMainScene;
}

OffscreenScenePtr Player::loadSceneFile(const string& sFilename)
{
    NodePtr pNode = loadMainNodeFromFile(sFilename);
    return registerOffscreenScene(pNode);
}

OffscreenScenePtr Player::loadSceneString(const string& sAVG)
{
    NodePtr pNode = loadMainNodeFromString(sAVG);
    return registerOffscreenScene(pNode);
}

void Player::deleteScene(const string& sID)
{
    vector<OffscreenScenePtr>::iterator it;
    for (it = m_pScenes.begin(); it != m_pScenes.end(); ++it) {
        if ((*it)->getID() == sID) {
            m_pScenes.erase(it);
            return;
        }
    }
    throw(Exception(AVG_ERR_OUT_OF_RANGE, 
            string("deleteScene: Scene with id ")+sID+"does not exist."));
}

ScenePtr Player::getMainScene() const
{
    return m_pMainScene;
}

OffscreenScenePtr Player::getScene(const string& sID) const
{
    OffscreenScenePtr pScene = findScene(sID);
    if (pScene) {
        return pScene;
    } else {
        throw (Exception(AVG_ERR_INVALID_ARGS, 
                string("Player::getScene(): No scene with id '")+sID+"' exists."));
    }
}

NodePtr Player::loadMainNodeFromFile(const string& sFilename)
{
    string RealFilename;
    try {
        AVG_TRACE(Logger::MEMORY, std::string("Player::loadFile(") + sFilename + ")");

        // When loading an avg file, assets are loaded from a directory relative
        // to the file.
        char szBuf[1024];
        char * pBuf = getcwd(szBuf, 1024);
        if (sFilename[0] == '/') {
            RealFilename = sFilename; 
        } else {
            m_CurDirName = string(pBuf)+"/";
            RealFilename = m_CurDirName+sFilename;
        }
        m_CurDirName = RealFilename.substr(0, RealFilename.rfind('/')+1);

        string sAVG;
        readWholeFile(RealFilename, sAVG);
        NodePtr pNode = internalLoad(sAVG);

        // Reset the directory to load assets from to the current dir.
        m_CurDirName = string(pBuf)+"/";
        return pNode;
    } catch (Exception& ex) {
        switch (ex.GetCode()) {
            case AVG_ERR_XML_PARSE:
                throw (Exception(AVG_ERR_XML_PARSE, 
                        string("Error parsing xml document ")+RealFilename));
                break;
            case AVG_ERR_XML_VALID:
                throw (Exception(AVG_ERR_XML_VALID, 
                        RealFilename + " does not validate."));
                break;
            default:
                throw;
        }
    }
}

NodePtr Player::loadMainNodeFromString(const string& sAVG)
{
    try {
        AVG_TRACE(Logger::MEMORY, "Player::loadString()");
        
        string sEffectiveDoc = removeStartEndSpaces(sAVG);
        NodePtr pNode = internalLoad(sEffectiveDoc);
        return pNode;
    } catch (Exception& ex) {
        switch (ex.GetCode()) {
            case AVG_ERR_XML_PARSE:
                throw Exception(AVG_ERR_XML_PARSE, "Error parsing xml string.");
                break;
            case AVG_ERR_XML_VALID:
                throw Exception(AVG_ERR_XML_VALID, "Error validating xml string.");
                break;
            default:
                throw;
        }
    }
}

void Player::play()
{
    try {
        if (!m_pMainScene) {
            throw Exception(AVG_ERR_NO_NODE, "Play called, but no xml file loaded.");
        }
        initPlayback();
        try {
            for (unsigned i=0; i< m_pScenes.size(); ++i) {
                m_pScenes[i]->render();
            }
            m_pMainScene->render();
            if (m_pDisplayEngine->wasFrameLate()) {
                ThreadProfiler::get()->dumpFrame();
            }
            ThreadProfiler::get()->start();

            while (!m_bStopping) {
                doFrame();
            }
        } catch (...) {
            cleanup();
            throw;
        }
        cleanup();
        AVG_TRACE(Logger::PLAYER, "Playback ended.");
    } catch (Exception& ex) {
        m_bIsPlaying = false;
        AVG_TRACE(Logger::ERROR, ex.GetStr());
        throw;
    }
}

void Player::stop()
{
    if (m_bIsPlaying) {
        m_bStopping = true;
    } else {
        cleanup();
    }
}

bool Player::isStopping()
{
    return m_bStopping;
}
 
void Player::initPlayback()
{
    m_bIsPlaying = true;
    AVG_TRACE(Logger::PLAYER, "Playback started.");
    initGraphics();
    if (m_bAudioEnabled) {
        initAudio();
    }
    try {
        for (unsigned i=0; i<m_pScenes.size(); ++i) {
            m_pScenes[i]->initPlayback(m_pDisplayEngine, m_pAudioEngine, m_pTestHelper);
        }
        m_pMainScene->initPlayback(m_pDisplayEngine, m_pAudioEngine, m_pTestHelper);
    } catch (Exception&) {
        m_pDisplayEngine = 0;
        m_pAudioEngine = 0;
        throw;
    }
    m_pDisplayEngine->initRender();
    m_bStopping = false;
    if (m_pTracker) {
        m_pTracker->start();
    }

    m_FrameTime = 0;
    m_NumFrames = 0;
}

bool Player::isPlaying()
{
    return m_bIsPlaying;
}

void Player::setFramerate(double rate)
{
    if (m_bIsPlaying) {
        m_pDisplayEngine->setFramerate(rate);
    } else {
        m_DP.m_Framerate = rate;
        m_DP.m_VBRate = 0;
    }
}

void Player::setVBlankFramerate(int rate)
{
    if (m_bIsPlaying) {
        m_pDisplayEngine->setVBlankRate(rate);
    } else {
        m_DP.m_Framerate = 0;
        m_DP.m_VBRate = rate;
    }
}
        
double Player::getEffectiveFramerate()
{
    if (m_bIsPlaying) {
        return m_pDisplayEngine->getEffectiveFramerate();
    } else {
        return 0;
    }
}

TestHelper * Player::getTestHelper()
{
    return m_pTestHelper;
}

void Player::setFakeFPS(double fps)
{
    if (fabs(fps + 1.0) < 0.0001) {
        // fps = -1
        m_bFakeFPS = false;
    } else {
        m_bFakeFPS = true;
        m_FakeFPS = fps;
    }

    if (m_pAudioEngine) {
        m_pAudioEngine->setAudioEnabled(!m_bFakeFPS);
    }
}

long long Player::getFrameTime()
{
    return m_FrameTime;
}

TrackerEventSource * Player::addTracker()
{
    TrackerConfig Config;
    Config.load();
    CameraPtr pCamera;

    string sDriver = Config.getParam("/camera/driver/@value");
    string sDevice = Config.getParam("/camera/device/@value");
    bool bFW800 = Config.getBoolParam("/camera/fw800/@value");
    IntPoint CaptureSize(Config.getPointParam("/camera/size/"));
    string sCaptureFormat = Config.getParam("/camera/format/@value");
    double FrameRate = Config.getDoubleParam("/camera/framerate/@value");

    if (!m_pMainScene) {
        throw Exception(AVG_ERR_UNSUPPORTED, 
                "You must use loadFile() before addTracker().");
    }

    PixelFormat camPF = Bitmap::stringToPixelFormat(sCaptureFormat);
    if (camPF == NO_PIXELFORMAT) {
        throw Exception(AVG_ERR_INVALID_ARGS,
                "Unknown camera pixel format "+sCaptureFormat+".");
    }
    
    AVG_TRACE(Logger::CONFIG, "Trying to create a Tracker for " << sDriver
            << " Camera: " << sDevice << " Size: " << CaptureSize << "format: "
            << sCaptureFormat);
    pCamera = createCamera(sDriver, sDevice, -1, bFW800, CaptureSize, camPF, I8, 
            FrameRate);
    AVG_TRACE(Logger::CONFIG, "Got Camera " << pCamera->getDevice() << " from driver: " 
            << pCamera->getDriverName());
    m_pTracker = new TrackerEventSource(pCamera, Config, m_DP.m_Size, true);
    m_pMainScene->addEventSource(m_pTracker);
    if (m_bIsPlaying) {
        m_pTracker->start();
    }

    return m_pTracker;
}

TrackerEventSource * Player::getTracker()
{
    return m_pTracker;
}

int Player::setInterval(int time, PyObject * pyfunc)
{
    Timeout *t = new Timeout(time, pyfunc, true, getFrameTime());
    if (m_bInHandleTimers) {
        m_NewTimeouts.push_back(t);
    } else {
        addTimeout(t);
    }
    return t->GetID();
}

int Player::setTimeout(int time, PyObject * pyfunc)
{
    Timeout *t = new Timeout(time, pyfunc, false, getFrameTime());
    if (m_bInHandleTimers) {
        m_NewTimeouts.push_back(t);
    } else {
        addTimeout(t);
    }
    return t->GetID();
}

int Player::setOnFrameHandler(PyObject * pyfunc) 
{
    return setInterval(0, pyfunc);
}

bool Player::clearInterval(int id)
{
    vector<Timeout*>::iterator it;
    for (it=m_PendingTimeouts.begin(); it!=m_PendingTimeouts.end(); it++) {
        if (id == (*it)->GetID()) {
            if (it == m_PendingTimeouts.begin() && m_bInHandleTimers) {
                m_bCurrentTimeoutDeleted = true;
            }
            delete *it;
            m_PendingTimeouts.erase(it);
            return true;
        }
    }
    for (it=m_NewTimeouts.begin(); it!=m_NewTimeouts.end(); it++) {
        if (id == (*it)->GetID()) {
            delete *it;
            m_NewTimeouts.erase(it);
            return true;
        }
    }
    return false;
}

MouseEventPtr Player::getMouseState() const
{
    if (m_pMainScene) {
        return m_pMainScene->getMouseState();
    } else {
        return MouseEventPtr(new MouseEvent(Event::CURSORMOTION, false, false, false, 
                IntPoint(0, 0), MouseEvent::NO_BUTTON, DPoint(0,0)));
    }
}

void Player::setMousePos(const IntPoint& pos)
{
    m_pDisplayEngine->setMousePos(pos);
}

int Player::getKeyModifierState() const
{
    return m_pDisplayEngine->getKeyModifierState();
}

BitmapPtr Player::screenshot()
{
    return m_pDisplayEngine->screenshot();
}

void Player::showCursor(bool bShow)
{
    if (m_pDisplayEngine) {
        m_pDisplayEngine->showCursor(bShow);
    } else {
        m_DP.m_bShowCursor = bShow;
    }
}

void Player::setCursor(const Bitmap* pBmp, IntPoint hotSpot)
{
    IntPoint size = pBmp->getSize();
    if (size.x % 8 != 0 || size.y % 8 != 0 || pBmp->getPixelFormat() != R8G8B8A8) {
        throw Exception(AVG_ERR_INVALID_ARGS, 
                "setCursor: Bitmap size must be divisible by 8 and in RGBA format.");
    }
    int i = -1;
    unsigned char * pData = new unsigned char[size.x*size.y/8];
    unsigned char * pMask = new unsigned char[size.x*size.y/8];
    Pixel32 * pLine = (Pixel32*)(pBmp->getPixels());
    int stride = pBmp->getStride()/4;
    for (int y=0; y<size.y; ++y) {
        Pixel32 * pPixel = pLine;
        for (int x=0; x<size.x; ++x) {
            if (x % 8 == 0) {
                i++;
                pData[i] = 0;
                pMask[i] = 0;
            } else {
                pData[i] <<= 1;
                pMask[i] <<= 1;
            }
            if (pPixel->getA() > 127) {
                pMask[i] |= 0x01;
                if (pPixel->getR() < 128) {
                    // Black Pixel
                    pData[i] |= 0x01;
                }
            }
            pPixel++;
        }
        pLine += stride;
    }
    SDL_Cursor * pCursor = SDL_CreateCursor(pData, pMask, size.x, size.y, 
            hotSpot.x, hotSpot.y);
    SDL_SetCursor(pCursor);
    delete pData;
    delete pMask;
}

NodePtr Player::getElementByID(const std::string& id)
{
    if (m_pMainScene) {
        return m_pMainScene->getElementByID(id);
    } else {
        return NodePtr();
    }
}
        
AVGNodePtr Player::getRootNode()
{
    if (m_pMainScene) {
        return m_pMainScene->getRootNode();
    } else {
        return AVGNodePtr();
    }
}

string Player::getCurDirName()
{
    return m_CurDirName;
}
        
std::string Player::getRootMediaDir()
{
    string sMediaDir;
    if (m_pMainScene) {
        sMediaDir = m_pMainScene->getRootNode()->getEffectiveMediaDir();
    } else {
        sMediaDir = m_CurDirName;
    }
    return sMediaDir;
}

const NodeDefinition& Player::getNodeDef(const std::string& sType)
{
    return m_NodeRegistry.getNodeDef(sType);
}

void Player::disablePython()
{
    m_bPythonAvailable = false;
}

bool Player::isAudioEnabled() const
{
    return m_bAudioEnabled;
}

void Player::registerFrameEndListener(IFrameEndListener* pListener)
{
    m_pMainScene->registerFrameEndListener(pListener);
}

void Player::unregisterFrameEndListener(IFrameEndListener* pListener)
{
    if (m_pMainScene) {
        m_pMainScene->unregisterFrameEndListener(pListener);
    }
}

void Player::registerPlaybackEndListener(IPlaybackEndListener* pListener)
{
    m_pMainScene->registerPlaybackEndListener(pListener);
}

void Player::unregisterPlaybackEndListener(IPlaybackEndListener* pListener)
{
    if (m_pMainScene) {
        m_pMainScene->unregisterPlaybackEndListener(pListener);
    }
}

void Player::registerPreRenderListener(IPreRenderListener* pListener)
{
    m_pMainScene->registerPreRenderListener(pListener);
}

void Player::unregisterPreRenderListener(IPreRenderListener* pListener)
{
    if (m_pMainScene) {
        m_pMainScene->unregisterPreRenderListener(pListener);
    }
}

static ProfilingZone MainProfilingZone("Player - Total frame time");
static ProfilingZone TimersProfilingZone("Player - handleTimers");

void Player::doFrame()
{
    {
        ScopeTimer Timer(MainProfilingZone);
        if (m_bFakeFPS) {
            m_NumFrames++;
            m_FrameTime = (long long)((m_NumFrames*1000.0)/m_FakeFPS);
        } else {
            m_FrameTime = m_pDisplayEngine->getDisplayTime();
        }
        {
            ScopeTimer Timer(TimersProfilingZone);
            handleTimers();
        }
        for (unsigned i=0; i< m_pScenes.size(); ++i) {
            m_pScenes[i]->doFrame(m_bPythonAvailable);
        }
        m_pMainScene->doFrame(m_bPythonAvailable);
    }
    if (m_pDisplayEngine->wasFrameLate()) {
        ThreadProfiler::get()->dumpFrame();
    }
    
/*
    long FrameTime = long(MainProfilingZone.getUSecs()/1000);
    long TargetTime = long(1000/m_pDisplayEngine->getFramerate());
    if (FrameTime > TargetTime+2) {
        AVG_TRACE(Logger::PROFILE_LATEFRAMES, "frame too late by " <<
                FrameTime-TargetTime << " ms.");
        Profiler::get().dumpFrame();
    }
*/
    ThreadProfiler::get()->reset();
}

double Player::getFramerate()
{
    if (!m_pDisplayEngine) {
        return m_DP.m_Framerate;
    }
    return m_pDisplayEngine->getFramerate();
}

double Player::getVideoRefreshRate()
{
    if (!m_pDisplayEngine) {
        return 0;
    }
    return m_pDisplayEngine->getRefreshRate();
}

void Player::setGamma(double Red, double Green, double Blue)
{
    if (m_pDisplayEngine) {
        m_pDisplayEngine->setGamma(Red, Green, Blue);
    } else {
        m_DP.m_Gamma[0] = Red;
        m_DP.m_Gamma[1] = Green;
        m_DP.m_Gamma[2] = Blue;
    }
}

void Player::initConfig()
{
    // Get data from config files.
    ConfigMgr* pMgr = ConfigMgr::get();
    
    m_DP.m_BPP = atoi(pMgr->getOption("scr", "bpp")->c_str());
    if (m_DP.m_BPP != 15 && m_DP.m_BPP != 16 && m_DP.m_BPP != 24 && m_DP.m_BPP != 32) {
        AVG_TRACE(Logger::ERROR, 
                "BPP must be 15, 16, 24 or 32. Current value is " 
                << m_DP.m_BPP << ". Aborting." );
        exit(-1);
    }
    m_DP.m_bFullscreen = pMgr->getBoolOption("scr", "fullscreen", false);

    m_DP.m_WindowSize.x = atoi(pMgr->getOption("scr", "windowwidth")->c_str());
    m_DP.m_WindowSize.y = atoi(pMgr->getOption("scr", "windowheight")->c_str());

    if (m_DP.m_bFullscreen && (m_DP.m_WindowSize != IntPoint(0, 0))) {
        AVG_TRACE(Logger::ERROR, 
                "Can't set fullscreen and window size at once. Aborting.");
        exit(-1);
    }
    if (m_DP.m_WindowSize.x != 0 && m_DP.m_WindowSize.y != 0) {
        AVG_TRACE(Logger::ERROR, "Can't set window width and height at once");
        AVG_TRACE(Logger::ERROR, 
                "(aspect ratio is determined by avg file). Aborting.");
        exit(-1);
    }

    m_AP.m_Channels = atoi(pMgr->getOption("aud", "channels")->c_str());
    m_AP.m_SampleRate = atoi(pMgr->getOption("aud", "samplerate")->c_str());
    m_AP.m_OutputBufferSamples = atoi(pMgr->getOption("aud", "outputbuffersamples")->c_str());

    m_GLConfig.m_bUsePOTTextures = pMgr->getBoolOption("scr", "usepow2textures", false);
    m_GLConfig.m_bUseShaders = pMgr->getBoolOption("scr", "useshaders", true);

    m_GLConfig.m_bUsePixelBuffers = pMgr->getBoolOption("scr", "usepixelbuffers", true);
    m_GLConfig.m_MultiSampleSamples = pMgr->getIntOption("scr", "multisamplesamples", 1);
    pMgr->getGammaOption("scr", "gamma", m_DP.m_Gamma);
}

void Player::initGraphics()
{
    // Init display configuration.
    AVG_TRACE(Logger::CONFIG, "Display bpp: " << m_DP.m_BPP);

    if (!m_pDisplayEngine) {
        AVG_TRACE(Logger::CONFIG, "Requested OpenGL configuration: ");
        m_GLConfig.log();

        m_pDisplayEngine = new SDLDisplayEngine();
    }
    SDLDisplayEngine * pSDLDisplayEngine = 
            dynamic_cast<SDLDisplayEngine*>(m_pDisplayEngine);
    if (pSDLDisplayEngine) {
        pSDLDisplayEngine->setOGLOptions(m_GLConfig);
    }
    m_pDisplayEngine->init(m_DP);
}

void Player::initAudio()
{
    if (!m_pAudioEngine) {
        m_pAudioEngine = new SDLAudioEngine();
    }
    m_pAudioEngine->init(m_AP, m_Volume);
    m_pAudioEngine->setAudioEnabled(!m_bFakeFPS);
    m_pAudioEngine->play();
}

void Player::updateDTD()
{
    // Find and parse dtd.
    registerDTDEntityLoader("avg.dtd", m_NodeRegistry.getDTD().c_str());
    string sDTDFName = "avg.dtd";
    m_dtd = xmlParseDTD(NULL, (const xmlChar*) sDTDFName.c_str());
    assert (m_dtd);
    m_bDirtyDTD = false;
}

NodePtr Player::internalLoad(const string& sAVG)
{
    xmlDocPtr doc = 0;
    try {
        xmlPedanticParserDefault(1);
        xmlDoValidityCheckingDefaultValue=0;

        doc = xmlParseMemory(sAVG.c_str(), sAVG.length());
        if (!doc) {
            throw (Exception(AVG_ERR_XML_PARSE, ""));
        }

        if (m_bDirtyDTD) {
            updateDTD();
        }

        xmlValidCtxtPtr cvp = xmlNewValidCtxt();
        cvp->error = xmlParserValidityError;
        cvp->warning = xmlParserValidityWarning;
        int valid=xmlValidateDtd(cvp, doc, m_dtd);  
        xmlFreeValidCtxt(cvp);
        if (!valid) {
            throw (Exception(AVG_ERR_XML_VALID, ""));
        }
        xmlNodePtr xmlNode = xmlDocGetRootElement(doc);
        NodePtr pNode = createNodeFromXml(doc, xmlNode, DivNodePtr());
        if (!pNode) {
            throw (Exception(AVG_ERR_XML_PARSE, 
                    "Root node of an avg tree needs to be an <avg> node."));
        }
        if (dynamic_pointer_cast<DivNode>(pNode)->getSize() == DPoint(0, 0)) {
            throw (Exception(AVG_ERR_OUT_OF_RANGE,
                    "<avg> and <scene> node width and height attributes are mandatory."));
        }
        xmlFreeDoc(doc);
        return pNode;
    } catch (Exception& ex) {
        if (doc) {
            xmlFreeDoc(doc);
        }
        throw;
    } catch (Magick::Exception & ex) {
        AVG_TRACE(Logger::ERROR, ex.what());
        if (doc) {
            xmlFreeDoc(doc);
        }
        throw;
    }
}

void Player::registerNodeType(NodeDefinition Def, const char* pParentNames[])
{
    m_NodeRegistry.registerNodeType(Def);

    if (pParentNames) {
       string sChildArray[1];
       sChildArray[0] = Def.getName();
       vector<string> sChildren = vectorFromCArray(1, sChildArray);
        const char **pCurrParentName = pParentNames;

        while(*pCurrParentName) {
            NodeDefinition nodeDefinition = m_NodeRegistry.getNodeDef(*pCurrParentName);
            nodeDefinition.addChildren(sChildren);
            m_NodeRegistry.updateNodeDefinition(nodeDefinition);
            
            ++pCurrParentName;
        }
    }
    m_bDirtyDTD = true;
}

NodePtr Player::createNode(const string& sType, const boost::python::dict& params)
{
    DivNodePtr pParentNode;
    boost::python::dict attrs = params;
    if (params.has_key("parent")) {
        boost::python::object parent = params["parent"];
        attrs.attr("__delitem__")("parent");
        pParentNode = boost::python::extract<DivNodePtr>(parent);
    }
    NodePtr pNode = m_NodeRegistry.createNode(sType, attrs);
    if (pParentNode) {
        pParentNode->appendChild(pNode);
    }
    return pNode;
}

NodePtr Player::createNodeFromXmlString(const string& sXML)
{
    xmlPedanticParserDefault(1);
    xmlDoValidityCheckingDefaultValue =0;

    xmlDocPtr doc;
    doc = xmlParseMemory(sXML.c_str(), int(sXML.length()));
    if (!doc) {
        throw (Exception(AVG_ERR_XML_PARSE, 
                    string("Error parsing xml:\n  ")+sXML));
    }
    NodePtr pNode = createNodeFromXml(doc, xmlDocGetRootElement(doc), DivNodePtr());

    if (m_bDirtyDTD)
        updateDTD();

    xmlValidCtxtPtr cvp = xmlNewValidCtxt();
    cvp->error = xmlParserValidityError;
    cvp->warning = xmlParserValidityWarning;
    int valid=xmlValidateDtd(cvp, doc, m_dtd);  
    xmlFreeValidCtxt(cvp);
    if (!valid) {
        throw (Exception(AVG_ERR_XML_PARSE, 
                    "Could not validate '"+sXML+"'"));
    }

    xmlFreeDoc(doc);
    return pNode;
}

NodePtr Player::createNodeFromXml(const xmlDocPtr xmlDoc, 
        const xmlNodePtr xmlNode, DivNodeWeakPtr pParent)
{
    NodePtr curNode;
    const char * nodeType = (const char *)xmlNode->name;
    
    if (!strcmp (nodeType, "text") || 
        !strcmp (nodeType, "comment")) {
        // Ignore whitespace & comments
        return NodePtr();
    }
    curNode = m_NodeRegistry.createNode(nodeType, xmlNode);
    if (!strcmp (nodeType, "words")) {
        // TODO: This is an end-run around the generic serialization mechanism
        // that will probably break at some point.
        string s = getXmlChildrenAsString(xmlDoc, xmlNode);
        boost::dynamic_pointer_cast<WordsNode>(curNode)->setTextFromNodeValue(s);
    }

    // If this is a container, recurse into children
    DivNodePtr curGroup = boost::dynamic_pointer_cast<DivNode>(curNode);
    if (curGroup) {
        xmlNodePtr curXmlChild = xmlNode->xmlChildrenNode;
        while (curXmlChild) {
            NodePtr curChild = createNodeFromXml(xmlDoc, curXmlChild, curGroup);
            if (curChild) {
                curGroup->appendChild(curChild);
            }
            curXmlChild = curXmlChild->next;
        }
    }
    return curNode;
}

OffscreenScenePtr Player::registerOffscreenScene(NodePtr pNode)
{
    OffscreenScenePtr pScene(new OffscreenScene(this, pNode));
    if (findScene(pScene->getID())) {
        throw (Exception(AVG_ERR_INVALID_ARGS, 
                string("Duplicate scene id ")+pScene->getID()));
    }
    m_pScenes.push_back(pScene);
    if (m_bIsPlaying) {
        pScene->initPlayback(m_pDisplayEngine, m_pAudioEngine, m_pTestHelper);
    }
    return pScene;
}

OffscreenScenePtr Player::findScene(const std::string& sID) const
{
    for (unsigned i=0; i<m_pScenes.size(); ++i) {
        if (m_pScenes[i]->getID() == sID) {
            return m_pScenes[i];
        }
    }
    return OffscreenScenePtr();
}

void Player::handleTimers()
{
    vector<Timeout *>::iterator it;
    m_bInHandleTimers = true;
   
    it = m_PendingTimeouts.begin();
    while (it != m_PendingTimeouts.end() && (*it)->IsReady(getFrameTime()) 
            && !m_bStopping)
    {
        (*it)->Fire(getFrameTime());
        if (m_bCurrentTimeoutDeleted) {
            it = m_PendingTimeouts.begin();
        } else {
            if ((*it)->IsInterval()) {
                Timeout* pTempTimeout = *it;
                it = m_PendingTimeouts.erase(it);
                m_NewTimeouts.push_back(pTempTimeout);
            } else {
                delete *it;
                it = m_PendingTimeouts.erase(it);
            }
        }
        m_bCurrentTimeoutDeleted = false;
    }
    for (it = m_NewTimeouts.begin(); it != m_NewTimeouts.end(); ++it) {
        addTimeout(*it);
    }
    m_NewTimeouts.clear();
    m_bInHandleTimers = false;
    
}

DisplayEngine * Player::getDisplayEngine() const
{
    return m_pDisplayEngine;
}

void Player::setStopOnEscape(bool bStop)
{
    m_bStopOnEscape = bStop;
}

bool Player::getStopOnEscape() const
{
    return m_bStopOnEscape;
}

void Player::setVolume(double volume)
{
    m_Volume = volume;
    if (m_pAudioEngine) {
        m_pAudioEngine->setVolume(m_Volume);
    }
}

double Player::getVolume() const
{
    return m_Volume;
}

OffscreenScenePtr Player::getSceneFromURL(const std::string& sURL)
{
    if (sURL.substr(0, 6) != "scene:") {
        throw Exception(AVG_ERR_CANT_PARSE_STRING, 
                string("Invalid scene url :'")+sURL+"'");
    }
    string sSceneID = sURL.substr(6);
    for (unsigned i=0; i < m_pScenes.size(); ++i) {
        if (m_pScenes[i]->getID() == sSceneID) {
            return m_pScenes[i];
        }
    }
    throw Exception(AVG_ERR_CANT_PARSE_STRING, 
            string("Scene with url '")+sURL+"' not found.");
}

void Player::cleanup() 
{
    // Kill all timeouts.
    vector<Timeout*>::iterator it;
    for (it=m_PendingTimeouts.begin(); it!=m_PendingTimeouts.end(); it++) {
        delete *it;
    }
    m_PendingTimeouts.clear();
    Profiler::get().dumpStatistics();
    if (m_pMainScene) {
        m_pMainScene = MainScenePtr();
    }

    if (m_pTracker) {
        delete m_pTracker;
        m_pTracker = 0;
    }
    m_pScenes.clear();

    if (m_pDisplayEngine) {
        m_pDisplayEngine->deinitRender();
        m_pDisplayEngine->teardown();
    }
    if (m_pAudioEngine) {
        m_pAudioEngine->teardown();
    }
    
    m_FrameTime = 0;
    m_bIsPlaying = false;

    m_CurDirName = getCWD();
}

int Player::addTimeout(Timeout* pTimeout)
{
    vector<Timeout*>::iterator it=m_PendingTimeouts.begin();
    while (it != m_PendingTimeouts.end() && (**it)<*pTimeout) {
        it++;
    }
    m_PendingTimeouts.insert(it, pTimeout);
    return pTimeout->GetID();
}


void Player::removeTimeout(Timeout* pTimeout)
{
    delete pTimeout;
    vector<Timeout*>::iterator it=m_PendingTimeouts.begin();
    while (*it != pTimeout) {
        it++;
    }
    m_PendingTimeouts.erase(it);
}

void Player::setPluginPath(const string& newPath)
{
    PluginManager::get().setSearchPath(newPath);
}

string Player::getPluginPath() const 
{
    return  PluginManager::get().getSearchPath();
}

void Player::loadPlugin(const std::string& name)
{
    PluginManager::get().loadPlugin(name);
}

void Player::setEventHook(PyObject * pyfunc)
{
    if (m_EventHookPyFunc != Py_None) {
        Py_DECREF(m_EventHookPyFunc);
    }

    if (pyfunc != Py_None) {
        Py_INCREF(pyfunc);
    }

    m_EventHookPyFunc = pyfunc;
}

PyObject * Player::getEventHook() const
{
    return m_EventHookPyFunc;
}

}
