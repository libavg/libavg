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
#include "MainCanvas.h"
#include "OffscreenCanvas.h"
#include "TrackerEventSource.h"
#include "SDLDisplayEngine.h"
#include "MultitouchEventSource.h"
#include "TUIOEventSource.h"
#ifdef __APPLE__
    #include "AppleTrackpadEventSource.h"
#endif
#if defined(_WIN32) && defined(SM_DIGITIZER)
    #include "Win7TouchEventSource.h"
#endif
#ifdef AVG_ENABLE_MTDEV
    #include "LibMTDevEventSource.h"
#endif
#include "../base/FileHelper.h"
#include "../base/StringHelper.h"
#include "../base/OSHelper.h"
#include "../base/Exception.h"
#include "../base/Logger.h"
#include "../base/ConfigMgr.h"
#include "../base/XMLHelper.h"
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

#ifdef __linux
#include <fenv.h>
#endif

using namespace std;
using namespace boost;

namespace avg {
    
Player * Player::s_pPlayer=0;

Player::Player()
    : m_pDisplayEngine(0),
      m_pAudioEngine(0),
      m_bAudioEnabled(true),
      m_pTracker(0),
      m_pMultitouchEventSource(0),
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
#ifdef __linux
// Turning this on causes fp exceptions in the linux nvidia drivers.
//    feenableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW); 
#endif
    if (s_pPlayer) {
        throw Exception(AVG_ERR_UNKNOWN, "Player has already been instantiated.");
    }
    ThreadProfilerPtr pProfiler = ThreadProfiler::get();
    pProfiler->setName("main");
    initConfig();

    // Register all node types
    registerNodeType(AVGNode::createDefinition());
    registerNodeType(OffscreenCanvasNode::createDefinition());
    registerNodeType(CanvasNode::createDefinition());
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
    m_pDisplayEngine = new SDLDisplayEngine();

    s_pPlayer = this;

    m_CurDirName = getCWD();
    string sDummy;
    if (getEnv("AVG_BREAK_ON_IMPORT", sDummy)) {
        debugBreak();
    }
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
#ifndef _WIN32
    // This causes libavg progams started under cmd to crash on system shutdown and
    // when cmd is closed, so it isn't done under windows.
    if (m_pAudioEngine) {
        delete m_pAudioEngine;
    }
#endif
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
    errorIfPlaying("Player.setResolution");
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

void Player::setWindowFrame(bool bHasWindowFrame)
{
    errorIfPlaying("Player.setWindowFrame");
    m_DP.m_bHasWindowFrame = bHasWindowFrame;
}

void Player::setWindowPos(int x, int y)
{
    errorIfPlaying("Player.setWindowPos");
    m_DP.m_Pos.x = x;
    m_DP.m_Pos.y = y;
}

void Player::setOGLOptions(bool bUsePOTTextures, bool bUseShaders, 
                bool bUsePixelBuffers, int multiSampleSamples)
{
    errorIfPlaying("Player.setOGLOptions");
    m_GLConfig.m_bUsePOTTextures = bUsePOTTextures;
    m_GLConfig.m_bUseShaders = bUseShaders;
    m_GLConfig.m_bUsePixelBuffers = bUsePixelBuffers;
    m_GLConfig.m_MultiSampleSamples = multiSampleSamples;
}

void Player::setMultiSampleSamples(int multiSampleSamples)
{
    errorIfPlaying("Player.setMultiSampleSamples");
    m_GLConfig.m_MultiSampleSamples = multiSampleSamples;
}

void Player::enableAudio(bool bEnable)
{
    errorIfPlaying("Player.enableAudio");
    m_bAudioEnabled = bEnable;
}

void Player::setAudioOptions(int samplerate, int channels)
{
    errorIfPlaying("Player.setAudioOptions");
    m_AP.m_SampleRate = samplerate;
    m_AP.m_Channels = channels;
}

DPoint Player::getScreenResolution() const
{
    errorIfPlaying("Player.getScreenResolution");
    return DPoint(dynamic_cast<SDLDisplayEngine *>(m_pDisplayEngine)->
            getScreenResolution());
}

CanvasPtr Player::loadFile(const string& sFilename)
{
    errorIfPlaying("Player.loadFile");
    NodePtr pNode = loadMainNodeFromFile(sFilename);
    m_pEventDispatcher = EventDispatcherPtr(new EventDispatcher);
    if (m_pMainCanvas) {
        cleanup();
    }

    m_pMainCanvas = MainCanvasPtr(new MainCanvas(this));
    m_pMainCanvas->setRoot(pNode);
    m_DP.m_Size = m_pMainCanvas->getSize();
    return m_pMainCanvas;
}

CanvasPtr Player::loadString(const string& sAVG)
{
    errorIfPlaying("Player.loadString");
    if (m_pMainCanvas) {
        cleanup();
    }

    NodePtr pNode = loadMainNodeFromString(sAVG);
    m_pEventDispatcher = EventDispatcherPtr(new EventDispatcher);
    m_pMainCanvas = MainCanvasPtr(new MainCanvas(this));
    m_pMainCanvas->setRoot(pNode);
    m_DP.m_Size = m_pMainCanvas->getSize();
    return m_pMainCanvas;
}

OffscreenCanvasPtr Player::loadCanvasFile(const string& sFilename)
{
    NodePtr pNode = loadMainNodeFromFile(sFilename);
    return registerOffscreenCanvas(pNode);
}

OffscreenCanvasPtr Player::loadCanvasString(const string& sAVG)
{
    NodePtr pNode = loadMainNodeFromString(sAVG);
    return registerOffscreenCanvas(pNode);
}

void Player::deleteCanvas(const string& sID)
{
    vector<OffscreenCanvasPtr>::iterator it;
    for (it = m_pCanvases.begin(); it != m_pCanvases.end(); ++it) {
        if ((*it)->getID() == sID) {
            if ((*it)->getNumDependentCanvases() > 0) {
                throw (Exception(AVG_ERR_INVALID_ARGS,
                        string("deleteCanvas: Canvas with id ")+sID
                        +" is still referenced."));
            }
            (*it)->stopPlayback();
            m_pCanvases.erase(it);
            return;
        }
    }
    throw(Exception(AVG_ERR_OUT_OF_RANGE, 
            string("deleteCanvas: Canvas with id ")+sID+" does not exist."));
}

CanvasPtr Player::getMainCanvas() const
{
    return m_pMainCanvas;
}

OffscreenCanvasPtr Player::getCanvas(const string& sID) const
{
    OffscreenCanvasPtr pCanvas = findCanvas(sID);
    if (pCanvas) {
        return pCanvas;
    } else {
        throw (Exception(AVG_ERR_INVALID_ARGS, 
                string("Player::getCanvas(): No canvas with id '")+sID+"' exists."));
    }
}

void Player::newCanvasDependency(const OffscreenCanvasPtr pCanvas)
{
    OffscreenCanvasPtr pNewCanvas;
    unsigned i;
    for (i = 0; i < m_pCanvases.size(); ++i) {
        if (pCanvas == m_pCanvases[i]) {
            pNewCanvas = m_pCanvases[i];
            m_pCanvases.erase(m_pCanvases.begin()+i);
            continue;
        }
    }
    assert(pNewCanvas);
    bool bFound = false;
    for (i = 0; i < m_pCanvases.size(); ++i) {
        if (pNewCanvas->hasDependentCanvas(m_pCanvases[i])) {
            bFound = true;
            break;
        }
    }
    if (bFound) {
        for (unsigned j = i; j < m_pCanvases.size(); ++j) {
            if (m_pCanvases[j]->hasDependentCanvas(pNewCanvas)) {
                throw Exception(AVG_ERR_INVALID_ARGS,
                        "Circular dependency between canvases.");
            }
        }
        m_pCanvases.insert(m_pCanvases.begin()+i, pNewCanvas);
    } else {
        AVG_ASSERT(pNewCanvas->hasDependentCanvas(m_pMainCanvas));
        m_pCanvases.push_back(pNewCanvas);
    }
/*    
    for (unsigned k=0; k<m_pCanvases.size(); ++k) {
        m_pCanvases[k]->dump();
    }
*/    
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
        if (!m_pMainCanvas) {
            throw Exception(AVG_ERR_NO_NODE, "Play called, but no xml file loaded.");
        }
        initPlayback();
        try {
            ThreadProfiler::get()->start();
            doFrame(true);
            while (!m_bStopping) {
                doFrame(false);
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
        for (unsigned i = 0; i < m_pCanvases.size(); ++i) {
            m_pCanvases[i]->initPlayback(
                    dynamic_cast<SDLDisplayEngine *>(m_pDisplayEngine), m_pAudioEngine);
        }
        m_pMainCanvas->initPlayback(dynamic_cast<SDLDisplayEngine *>(m_pDisplayEngine),
                m_pAudioEngine);
    } catch (Exception&) {
        m_pDisplayEngine->teardown();
        m_pDisplayEngine = 0;
        if (m_pAudioEngine) {
            m_pAudioEngine->teardown();
        }
        throw;
    }
    m_pEventDispatcher->addSource(dynamic_cast<SDLDisplayEngine *>(m_pDisplayEngine));
    m_pEventDispatcher->addSource(m_pTestHelper);
    m_pEventDispatcher->addSink(this);

    m_pDisplayEngine->initRender();
    m_bStopping = false;
    if (m_pTracker) {
        m_pTracker->start();
    }
    if (m_pMultitouchEventSource) {
        m_pMultitouchEventSource->start();
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

void Player::addEventSource(IEventSource* pSource)
{
    if (!m_pEventDispatcher) {
        throw Exception(AVG_ERR_UNSUPPORTED,
                "You must use loadFile() before addEventSource().");
    }
    m_pEventDispatcher->addSource(pSource);
}

long long Player::getFrameTime()
{
    return m_FrameTime;
}

double Player::getFrameDuration()
{
    if (!m_bIsPlaying) {
        throw Exception(AVG_ERR_UNSUPPORTED,
                "Must call Player.play() before getFrameDuration().");
    }
    double framerate = m_pDisplayEngine->getEffectiveFramerate();
    if (framerate > 0) {
        return 1000./framerate;
    } else {
        return 0;
    }
}

TrackerEventSource * Player::addTracker()
{
    TrackerConfig config;
    config.load();
    CameraPtr pCamera;

    string sDriver = config.getParam("/camera/driver/@value");
    string sDevice = config.getParam("/camera/device/@value");
    bool bFW800 = config.getBoolParam("/camera/fw800/@value");
    IntPoint captureSize(config.getPointParam("/camera/size/"));
    string sCaptureFormat = config.getParam("/camera/format/@value");
    double frameRate = config.getDoubleParam("/camera/framerate/@value");

    if (!m_pMainCanvas) {
        throw Exception(AVG_ERR_UNSUPPORTED, 
                "You must use loadFile() before addTracker().");
    }

    PixelFormat camPF = stringToPixelFormat(sCaptureFormat);
    if (camPF == NO_PIXELFORMAT) {
        throw Exception(AVG_ERR_INVALID_ARGS,
                "Unknown camera pixel format "+sCaptureFormat+".");
    }
    
    AVG_TRACE(Logger::CONFIG, "Trying to create a Tracker for " << sDriver
            << " Camera: " << sDevice << " Size: " << captureSize << "format: "
            << sCaptureFormat);
    pCamera = createCamera(sDriver, sDevice, -1, bFW800, captureSize, camPF, I8, 
            frameRate);
    AVG_TRACE(Logger::CONFIG, "Got Camera " << pCamera->getDevice() << " from driver: " 
            << pCamera->getDriverName());
    m_pTracker = new TrackerEventSource(pCamera, config, m_DP.m_Size, true);
    addEventSource(m_pTracker);
    if (m_bIsPlaying) {
        m_pTracker->start();
    }

    return m_pTracker;
}

TrackerEventSource * Player::getTracker()
{
    return m_pTracker;
}

void Player::enableMultitouch()
{
    string sDriver("TUIO");
    getEnv("AVG_MULTITOUCH_DRIVER", sDriver);
    if (sDriver == "TUIO") {
        m_pMultitouchEventSource = new TUIOEventSource;
#if defined(_WIN32) && defined(SM_DIGITIZER)
    } else if (sDriver == "WIN7TOUCH") {
        m_pMultitouchEventSource = new Win7TouchEventSource;
#endif
#ifdef AVG_ENABLE_MTDEV
    } else if (sDriver == "LINUXMTDEV") {
        m_pMultitouchEventSource = new LibMTDevEventSource;
#endif
#ifdef __APPLE__
    } else if (sDriver == "APPLETRACKPAD") {
        m_pMultitouchEventSource = new AppleTrackpadEventSource;
#endif
    } else {
        throw Exception(AVG_ERR_UNSUPPORTED, string("Unsupported multitouch driver '")+
                sDriver +"'.");
    }
    addEventSource(m_pMultitouchEventSource);
    if (m_bIsPlaying) {
        m_pMultitouchEventSource->start();
    }
}

void Player::setEventCapture(VisibleNodePtr pNode, int cursorID=MOUSECURSORID)
{
    std::map<int, VisibleNodeWeakPtr>::iterator it = m_pEventCaptureNode.find(cursorID);
    if (it != m_pEventCaptureNode.end() && !it->second.expired()) {
        throw Exception(AVG_ERR_INVALID_CAPTURE, "setEventCapture called for '"
                + pNode->getID() + "', but cursor already captured by '"
                + it->second.lock()->getID() + "'.");
    } else {
        m_pEventCaptureNode[cursorID] = pNode;
    }
}

void Player::releaseEventCapture(int cursorID)
{
    std::map<int, VisibleNodeWeakPtr>::iterator it = m_pEventCaptureNode.find(cursorID);
    if (it == m_pEventCaptureNode.end() || (it->second.expired()) ) {
        throw Exception(AVG_ERR_INVALID_CAPTURE,
                "releaseEventCapture called, but cursor not captured.");
    } else {
        m_pEventCaptureNode.erase(cursorID);
    }
}

int Player::setInterval(int time, PyObject * pyfunc)
{
    Timeout* pTimeout = new Timeout(time, pyfunc, true, getFrameTime());
    if (m_bInHandleTimers) {
        m_NewTimeouts.push_back(pTimeout);
    } else {
        addTimeout(pTimeout);
    }
    return pTimeout->GetID();
}

int Player::setTimeout(int time, PyObject * pyfunc)
{
    Timeout* pTimeout = new Timeout(time, pyfunc, false, getFrameTime());
    if (m_bInHandleTimers) {
        m_NewTimeouts.push_back(pTimeout);
    } else {
        addTimeout(pTimeout);
    }
    return pTimeout->GetID();
}

int Player::setOnFrameHandler(PyObject * pyfunc) 
{
    return setInterval(0, pyfunc);
}

bool Player::clearInterval(int id)
{
    vector<Timeout*>::iterator it;
    for (it = m_PendingTimeouts.begin(); it != m_PendingTimeouts.end(); it++) {
        if (id == (*it)->GetID()) {
            if (it == m_PendingTimeouts.begin() && m_bInHandleTimers) {
                m_bCurrentTimeoutDeleted = true;
            }
            delete *it;
            m_PendingTimeouts.erase(it);
            return true;
        }
    }
    for (it = m_NewTimeouts.begin(); it != m_NewTimeouts.end(); it++) {
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
    return m_MouseState.getLastEvent();
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
    for (int y = 0; y < size.y; ++y) {
        Pixel32 * pPixel = pLine;
        for (int x = 0; x < size.x; ++x) {
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

VisibleNodePtr Player::getElementByID(const std::string& sID)
{
    if (m_pMainCanvas) {
        return m_pMainCanvas->getElementByID(sID);
    } else {
        return VisibleNodePtr();
    }
}
        
AVGNodePtr Player::getRootNode()
{
    if (m_pMainCanvas) {
        return dynamic_pointer_cast<AVGNode>(m_pMainCanvas->getRootNode());
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
    if (m_pMainCanvas) {
        sMediaDir = m_pMainCanvas->getRootNode()->getEffectiveMediaDir();
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
    m_pMainCanvas->registerFrameEndListener(pListener);
}

void Player::unregisterFrameEndListener(IFrameEndListener* pListener)
{
    if (m_pMainCanvas) {
        m_pMainCanvas->unregisterFrameEndListener(pListener);
    }
}

void Player::registerPlaybackEndListener(IPlaybackEndListener* pListener)
{
    m_pMainCanvas->registerPlaybackEndListener(pListener);
}

void Player::unregisterPlaybackEndListener(IPlaybackEndListener* pListener)
{
    if (m_pMainCanvas) {
        m_pMainCanvas->unregisterPlaybackEndListener(pListener);
    }
}

void Player::registerPreRenderListener(IPreRenderListener* pListener)
{
    m_pMainCanvas->registerPreRenderListener(pListener);
}

void Player::unregisterPreRenderListener(IPreRenderListener* pListener)
{
    if (m_pMainCanvas) {
        m_pMainCanvas->unregisterPreRenderListener(pListener);
    }
}

bool Player::handleEvent(EventPtr pEvent)
{
    AVG_ASSERT(pEvent);
  
    PyObject * pEventHook = getEventHook();
    if (pEventHook != Py_None) {
        // If the catchall returns true, stop processing the event
        if (boost::python::call<bool>(pEventHook, pEvent)) {
            return true;
        }
    }
    if (MouseEventPtr pMouseEvent = boost::dynamic_pointer_cast<MouseEvent>(pEvent)) {
        m_MouseState.setEvent(pMouseEvent);
        pMouseEvent->setLastDownPos(m_MouseState.getLastDownPos());
    }
    
    if (CursorEventPtr pCursorEvent = boost::dynamic_pointer_cast<CursorEvent>(pEvent)) {
        if (pEvent->getType() == Event::CURSOROUT || 
                pEvent->getType() == Event::CURSOROVER)
        {
            pEvent->trace();
            pEvent->getElement()->handleEvent(pEvent);
        } else {
            handleCursorEvent(pCursorEvent);
        }
    }
    else if (KeyEventPtr pKeyEvent = boost::dynamic_pointer_cast<KeyEvent>(pEvent))
    {
        pEvent->trace();
        getRootNode()->handleEvent(pKeyEvent);
        if (getStopOnEscape() && pEvent->getType() == Event::KEYDOWN
                && pKeyEvent->getKeyCode() == avg::key::KEY_ESCAPE)
        {
            stop();
        }
    } else {
        switch (pEvent->getType()) {
            case Event::QUIT:
                stop();
                break;
            default:
                AVG_TRACE(Logger::ERROR, "Unknown event type in Player::handleEvent.");
                break;
        }
    }
    return true; 
}

static ProfilingZoneID MainProfilingZone("Player - Total frame time");
static ProfilingZoneID TimersProfilingZone("Player - handleTimers");
static ProfilingZoneID EventsProfilingZone("Dispatch events");

void Player::doFrame(bool bFirstFrame)
{
    {
        ScopeTimer Timer(MainProfilingZone);
        if (!bFirstFrame) {
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
            {
                ScopeTimer Timer(EventsProfilingZone);
                m_pEventDispatcher->dispatch();
                sendFakeEvents();
            }
        }
        for (unsigned i = 0; i < m_pCanvases.size(); ++i) {
            dispatchOffscreenRendering(m_pCanvases[i].get());
        }
        m_pMainCanvas->doFrame(m_bPythonAvailable);
        if (m_bPythonAvailable) {
            Py_BEGIN_ALLOW_THREADS;
            try {
                endFrame();
            } catch(...) {
                Py_BLOCK_THREADS;
                throw;
            }
            Py_END_ALLOW_THREADS;
        } else {
            endFrame();
        }
    }
    if (m_pDisplayEngine->wasFrameLate()) {
        ThreadProfiler::get()->dumpFrame();
    }
    
    ThreadProfiler::get()->reset();
}

void Player::endFrame()
{
    m_pDisplayEngine->frameWait();
    m_pDisplayEngine->swapBuffers();
    m_pDisplayEngine->checkJitter();
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

bool Player::isUsingShaders()
{
    if (!m_pDisplayEngine) {
        throw Exception(AVG_ERR_UNSUPPORTED, 
                "Player.isUsingShaders must be called after Player.play().");
    }
    return m_pDisplayEngine->isUsingShaders();
}

void Player::setGamma(double red, double green, double blue)
{
    if (m_pDisplayEngine) {
        m_pDisplayEngine->setGamma(red, green, blue);
    } else {
        m_DP.m_Gamma[0] = red;
        m_DP.m_Gamma[1] = green;
        m_DP.m_Gamma[2] = blue;
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
    m_AP.m_OutputBufferSamples = 
            atoi(pMgr->getOption("aud", "outputbuffersamples")->c_str());

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

    SDLDisplayEngine * pSDLDisplayEngine = 
            dynamic_cast<SDLDisplayEngine*>(m_pDisplayEngine);
    if (pSDLDisplayEngine) {
        AVG_TRACE(Logger::CONFIG, "Requested OpenGL configuration: ");
        m_GLConfig.log();
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
        NodePtr pNode = createNodeFromXml(doc, xmlNode);
        if (!pNode) {
            throw (Exception(AVG_ERR_XML_PARSE, 
                    "Root node of an avg tree needs to be an <avg> node."));
        }
        if (dynamic_pointer_cast<DivNode>(pNode)->getSize() == DPoint(0, 0)) {
            throw (Exception(AVG_ERR_OUT_OF_RANGE,
                    "<avg> and <canvas> node width and height attributes are mandatory."));
        }
        xmlFreeDoc(doc);
        return pNode;
    } catch (Exception&) {
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

void Player::registerNodeType(NodeDefinition def, const char* pParentNames[])
{
    m_NodeRegistry.registerNodeType(def);

    if (pParentNames) {
        string sChildArray[1];
        sChildArray[0] = def.getName();
        vector<string> sChildren = vectorFromCArray(1, sChildArray);
        const char **ppCurParentName = pParentNames;

        while (*ppCurParentName) {
            NodeDefinition nodeDefinition = m_NodeRegistry.getNodeDef(*ppCurParentName);
            nodeDefinition.addChildren(sChildren);
            m_NodeRegistry.updateNodeDefinition(nodeDefinition);
            
            ++ppCurParentName;
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
    NodePtr pNode = createNodeFromXml(doc, xmlDocGetRootElement(doc));

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
        const xmlNodePtr xmlNode)
{
    NodePtr curNode;
    const char * nodeType = (const char *)xmlNode->name;
    
    if (!strcmp (nodeType, "text") || 
        !strcmp (nodeType, "comment")) {
        // Ignore whitespace & comments
        return NodePtr();
    }
    curNode = m_NodeRegistry.createNode(nodeType, xmlNode);
    if (!strcmp(nodeType, "words")) {
        // TODO: This is an end-run around the generic serialization mechanism
        // that will probably break at some point.
        string s = getXmlChildrenAsString(xmlDoc, xmlNode);
        boost::dynamic_pointer_cast<WordsNode>(curNode)->setTextFromNodeValue(s);
    } else {
        // If this is a container, recurse into children
        if (curNode->getDefinition()->hasChildren()) {
            xmlNodePtr curXmlChild = xmlNode->xmlChildrenNode;
            while (curXmlChild) {
                NodePtr curChild = createNodeFromXml(xmlDoc, curXmlChild);
                if (curChild) {
                    curNode->appendChild(curChild);
                }
                curXmlChild = curXmlChild->next;
            }
        }
    }
    return curNode;
}

OffscreenCanvasPtr Player::registerOffscreenCanvas(NodePtr pNode)
{
    OffscreenCanvasPtr pCanvas(new OffscreenCanvas(this));
    pCanvas->setRoot(pNode);
    if (findCanvas(pCanvas->getID())) {
        throw (Exception(AVG_ERR_INVALID_ARGS, 
                string("Duplicate canvas id ")+pCanvas->getID()));
    }
    m_pCanvases.push_back(pCanvas);
    if (m_bIsPlaying) {
        try {
            pCanvas->initPlayback(dynamic_cast<SDLDisplayEngine *>(m_pDisplayEngine), 
                    m_pAudioEngine);
        } catch (...) {
            m_pCanvases.pop_back();
            throw;
        }
    }
    return pCanvas;
}

OffscreenCanvasPtr Player::findCanvas(const string& sID) const
{
    for (unsigned i=0; i<m_pCanvases.size(); ++i) {
        if (m_pCanvases[i]->getID() == sID) {
            return m_pCanvases[i];
        }
    }
    return OffscreenCanvasPtr();
}

void Player::sendFakeEvents()
{
    std::map<int, CursorStatePtr>::iterator it;
    for (it = m_pLastCursorStates.begin(); it != m_pLastCursorStates.end(); ++it) {
        CursorStatePtr state = it->second;
        handleCursorEvent(state->getLastEvent(), true);
    }
}

void Player::sendOver(const CursorEventPtr pOtherEvent, Event::Type type, 
        VisibleNodePtr pNode)
{
    if (pNode) {
        EventPtr pNewEvent = pOtherEvent->cloneAs(type);
        pNewEvent->setElement(pNode);
        m_pEventDispatcher->sendEvent(pNewEvent);
    }
}

void Player::handleCursorEvent(CursorEventPtr pEvent, bool bOnlyCheckCursorOver)
{
    DPoint pos(pEvent->getXPosition(), pEvent->getYPosition());
    int cursorID = pEvent->getCursorID();
    // Find all nodes under the cursor.
    vector<VisibleNodeWeakPtr> pCursorNodes = m_pMainCanvas->getElementsByPos(pos);

    // Determine the nodes the event should be sent to.
    vector<VisibleNodeWeakPtr> pDestNodes = pCursorNodes;
    bool bIsCapturing = false;
    if (m_pEventCaptureNode.find(cursorID) != m_pEventCaptureNode.end()) {
        VisibleNodeWeakPtr pEventCaptureNode = m_pEventCaptureNode[cursorID];
        if (pEventCaptureNode.expired()) {
            m_pEventCaptureNode.erase(cursorID);
        } else {
            pDestNodes = pEventCaptureNode.lock()->getParentChain();
        }
    } 

    vector<VisibleNodeWeakPtr> pLastCursorNodes;
    {
        map<int, CursorStatePtr>::iterator it;
        it = m_pLastCursorStates.find(cursorID);
        if (it != m_pLastCursorStates.end()) {
            pLastCursorNodes = it->second->getNodes();
        }
    }

    // Send out events.
    vector<VisibleNodeWeakPtr>::const_iterator itLast;
    vector<VisibleNodeWeakPtr>::iterator itCur;
    for (itLast = pLastCursorNodes.begin(); itLast != pLastCursorNodes.end(); ++itLast) {
        VisibleNodePtr pLastNode = itLast->lock();
        for (itCur = pCursorNodes.begin(); itCur != pCursorNodes.end(); ++itCur) {
            if (itCur->lock() == pLastNode) {
                break;
            }
        }
        if (itCur == pCursorNodes.end()) {
            if (!bIsCapturing || pLastNode == pDestNodes.begin()->lock()) {
                sendOver(pEvent, Event::CURSOROUT, pLastNode);
            }
        }
    } 

    // Send over events.
    for (itCur = pCursorNodes.begin(); itCur != pCursorNodes.end(); ++itCur) {
        VisibleNodePtr pCurNode = itCur->lock();
        for (itLast = pLastCursorNodes.begin(); itLast != pLastCursorNodes.end(); 
                ++itLast) 
        {
            if (itLast->lock() == pCurNode) {
                break;
            }
        }
        if (itLast == pLastCursorNodes.end()) {
            if (!bIsCapturing || pCurNode == pDestNodes.begin()->lock()) {
                sendOver(pEvent, Event::CURSOROVER, pCurNode);
            }
        }
    } 

    if (!bOnlyCheckCursorOver) {
        // Iterate through the nodes and send the event to all of them.
        vector<VisibleNodeWeakPtr>::iterator it;
        for (it = pDestNodes.begin(); it != pDestNodes.end(); ++it) {
            VisibleNodePtr pNode = (*it).lock();
            if (pNode) {
                CursorEventPtr pNodeEvent = boost::dynamic_pointer_cast<CursorEvent>(
                        pEvent->cloneAs(pEvent->getType()));
                pNodeEvent->setElement(pNode);
                if (pNodeEvent->getType() != Event::CURSORMOTION) {
                    pNodeEvent->trace();
                }
                if (pNode->handleEvent(pNodeEvent) == true) {
                    // stop bubbling
                    break;
                }
            }
        }
    }
    if (pEvent->getType() == Event::CURSORUP && pEvent->getSource() != Event::MOUSE) {
        // Cursor has disappeared: send out events.
        if (bIsCapturing) {
            VisibleNodePtr pNode = pDestNodes.begin()->lock();
            sendOver(pEvent, Event::CURSOROUT, pNode);
        } else {
            vector<VisibleNodeWeakPtr>::iterator it;
            for (it = pCursorNodes.begin(); it != pCursorNodes.end(); ++it) {
                VisibleNodePtr pNode = it->lock();
                sendOver(pEvent, Event::CURSOROUT, pNode);
            } 
        }
        m_pLastCursorStates.erase(cursorID);
    } else {
        // Update list of nodes under cursor
        if (m_pLastCursorStates.find(cursorID) != m_pLastCursorStates.end()) {
            m_pLastCursorStates[cursorID]->setInfo(pEvent, pCursorNodes);
        } else {
            m_pLastCursorStates[cursorID] =
                    CursorStatePtr(new CursorState(pEvent, pCursorNodes));
        }
    }
}

void Player::dispatchOffscreenRendering(OffscreenCanvas* pOffscreenCanvas)
{
    if (!pOffscreenCanvas->getAutoRender()) {
        return;
    }
    if (pOffscreenCanvas->hasRegisteredCamera()) {
        pOffscreenCanvas->updateCameraImage();
        while (pOffscreenCanvas->isCameraImageAvailable()) {
            pOffscreenCanvas->doFrame(m_bPythonAvailable);
            pOffscreenCanvas->updateCameraImage();
        }
    } else {
        pOffscreenCanvas->doFrame(m_bPythonAvailable);
        return;
    }
}

void Player::errorIfPlaying(const std::string& sFunc) const
{
    if (m_bIsPlaying) {
        throw Exception(AVG_ERR_UNSUPPORTED, 
                sFunc + " must be called before Player.play().");
    }
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
                m_NewTimeouts.insert(m_NewTimeouts.begin(), pTempTimeout);
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

OffscreenCanvasPtr Player::getCanvasFromURL(const std::string& sURL)
{
    if (sURL.substr(0, 7) != "canvas:") {
        throw Exception(AVG_ERR_CANT_PARSE_STRING, 
                string("Invalid canvas url :'")+sURL+"'");
    }
    string sCanvasID = sURL.substr(7);
    for (unsigned i=0; i < m_pCanvases.size(); ++i) {
        if (m_pCanvases[i]->getID() == sCanvasID) {
            return m_pCanvases[i];
        }
    }
    throw Exception(AVG_ERR_CANT_PARSE_STRING, 
            string("Canvas with url '")+sURL+"' not found.");
}

void Player::cleanup() 
{
    // Kill all timeouts.
    vector<Timeout*>::iterator it;
    for (it = m_PendingTimeouts.begin(); it != m_PendingTimeouts.end(); it++) {
        delete *it;
    }
    m_PendingTimeouts.clear();
    m_pEventCaptureNode.clear();
    m_pLastCursorStates.clear();
    ThreadProfiler::get()->dumpStatistics();
    if (m_pMainCanvas) {
        m_pMainCanvas->stopPlayback();
        m_pMainCanvas = MainCanvasPtr();
    }

    if (m_pTracker) {
        delete m_pTracker;
        m_pTracker = 0;
    }
    if (m_pMultitouchEventSource) {
        delete m_pMultitouchEventSource;
        m_pMultitouchEventSource = 0;
    }
    for (unsigned i = 0; i < m_pCanvases.size(); ++i) {
        m_pCanvases[i]->stopPlayback();
    }
    m_pCanvases.clear();

    if (m_pDisplayEngine) {
        m_pDisplayEngine->deinitRender();
        m_pDisplayEngine->teardown();
    }
    if (m_pAudioEngine) {
        m_pAudioEngine->teardown();
    }
    m_pEventDispatcher = EventDispatcherPtr();
    m_MouseState = MouseState();
    
    m_FrameTime = 0;
    m_bIsPlaying = false;

    m_CurDirName = getCWD();
}

int Player::addTimeout(Timeout* pTimeout)
{
    vector<Timeout*>::iterator it = m_PendingTimeouts.begin();
    while (it != m_PendingTimeouts.end() && (**it)<*pTimeout) {
        it++;
    }
    m_PendingTimeouts.insert(it, pTimeout);
    return pTimeout->GetID();
}


void Player::removeTimeout(Timeout* pTimeout)
{
    delete pTimeout;
    vector<Timeout*>::iterator it = m_PendingTimeouts.begin();
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
