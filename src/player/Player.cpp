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
#include "CanvasNode.h"
#include "Words.h"
#include "Video.h"
#include "CameraNode.h"
#include "Image.h"
#include "PanoImage.h"
#include "Sound.h"
#include "LineNode.h"
#include "RectNode.h"
#include "CurveNode.h"
#include "PolyLineNode.h"
#include "NodeDefinition.h"
#include "PluginManager.h"

#include "TrackerEventSource.h"
#include "Event.h"
#include "MouseEvent.h"
#include "KeyEvent.h"

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

#include "../imaging/FWCamera.h"
#ifdef AVG_ENABLE_V4L2
#include "../imaging/V4LCamera.h"
#endif
#ifdef AVG_ENABLE_DSHOW
#include "../imaging/DSCamera.h"
#endif
#include "../imaging/FakeCamera.h"

#include "../audio/SDLAudioEngine.h"

#undef HAVE_TEMPNAM
#include <Magick++.h>

#include <libxml/xmlmemory.h>

#ifdef _WIN32
#include <direct.h>
#define getcwd _getcwd
#endif

#include <algorithm>
#include <iostream>
#include <sstream>
#include <assert.h>
#include <math.h>

using namespace std;

namespace avg {
    
Player * Player::s_pPlayer=0;

Player::Player()
    : m_pRootNode(),
      m_pDisplayEngine(0),
      m_pAudioEngine(0),
      m_pTracker(0),
      m_bInHandleTimers(false),
      m_bCurrentTimeoutDeleted(false),
      m_pEventCaptureNode(),
      m_bUseFakeCamera(false),
      m_MaxGPUMemUsed(0),
      m_bStopOnEscape(true),
      m_bIsPlaying(false),
      m_bFakeFPS(false),
      m_FakeFPS(0),
      m_FrameTime(0),
      m_PlayStartTime(0),
      m_Volume(1),
      m_bPythonAvailable(true)
{
    if (s_pPlayer) {
        throw Exception(AVG_ERR_UNKNOWN, "Player has already been instantiated.");
    }
    ThreadProfilerPtr pThreadProfiler = ThreadProfilerPtr(new ThreadProfiler("Main"));
    Profiler::get().registerThreadProfiler(pThreadProfiler);
    initConfig();

    // Register all node types
    registerNodeType(AVGNode::createDefinition());
    registerNodeType(DivNode::createDefinition());
    registerNodeType(CanvasNode::createDefinition());
    registerNodeType(Image::createDefinition());
    registerNodeType(Words::createDefinition());
    registerNodeType(Video::createDefinition());
    registerNodeType(CameraNode::createDefinition());
    registerNodeType(PanoImage::createDefinition());
    registerNodeType(Sound::createDefinition());
    registerNodeType(LineNode::createDefinition());
    registerNodeType(RectNode::createDefinition());
    registerNodeType(CurveNode::createDefinition());
    registerNodeType(PolyLineNode::createDefinition());
    
    m_pTestHelper = new TestHelper(this);

#ifdef _WIN32
    Magick::InitializeMagick((getAvgLibPath()+"magick\\").c_str());
#endif

    s_pPlayer = this;
}

void Player::updateDTD()
{
    // Find and parse dtd.
    registerDTDEntityLoader("avg.dtd", m_NodeRegistry.getDTD().c_str());
    string sDTDFName = "avg.dtd";
    m_dtd = xmlParseDTD(NULL, (const xmlChar*) sDTDFName.c_str());
    if (!m_dtd) {
        AVG_TRACE(Logger::WARNING, 
                "DTD not found at " << sDTDFName << ". Not validating xml files.");
    }
    m_bDirtyDTD = false;
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
    }
    return s_pPlayer;
}

void Player::setResolution(bool bFullscreen, int width, int height, int bpp)
{
    m_DP.m_bFullscreen = bFullscreen;
    if (bpp) {
        m_DP.m_BPP = bpp;
    }
    if (width) {
        m_DP.m_WindowWidth = width;
    }
    if (height) {
        m_DP.m_WindowHeight = height;
    }
}

void Player::setWindowPos(int x, int y)
{
    m_DP.m_x = x;
    m_DP.m_y = y;
}

void Player::setOGLOptions(bool bUsePOW2Textures, YCbCrMode DesiredYCbCrMode, 
                bool bUsePixelBuffers, int MultiSampleSamples)
{
    m_bUsePOW2Textures = bUsePOW2Textures;
    m_YCbCrMode = DesiredYCbCrMode;
    m_bUsePixelBuffers = bUsePixelBuffers;
    m_MultiSampleSamples = MultiSampleSamples;
}

void Player::setMultiSampleSamples(int MultiSampleSamples)
{
    m_MultiSampleSamples = MultiSampleSamples;
}

void Player::setAudioOptions(int samplerate, int channels)
{
    m_AP.m_SampleRate = samplerate;
    m_AP.m_Channels = channels;
}

void Player::loadFile (const std::string& sFilename)
{
    string RealFilename;
    try {
        AVG_TRACE(Logger::MEMORY, 
                std::string("Player::loadFile(") + sFilename + ")");

        // When loading an avg file, assets are loaded from a directory relative
        // to the file.
        char szBuf[1024];
        if (sFilename[0] == '/') {
            RealFilename = sFilename; 
        } else {
            getcwd(szBuf, 1024);
            m_CurDirName = string(szBuf)+"/";
            RealFilename = m_CurDirName+sFilename;
        }
        m_CurDirName = RealFilename.substr(0, RealFilename.rfind('/')+1);

        string sAVG;
        readWholeFile(RealFilename, sAVG);
        internalLoad(sAVG);

        // Reset the directory to load assets from to the current dir.
        getcwd(szBuf, 1024);
        m_CurDirName = string(szBuf)+"/";
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

void Player::loadString(const std::string& sAVG)
{
    try {
        AVG_TRACE(Logger::MEMORY, "Player::loadString()");
        char szBuf[1024];
        getcwd(szBuf, 1024);
        m_CurDirName = string(szBuf)+"/";

        string sEffectiveDoc = removeStartEndSpaces(sAVG);
        internalLoad(sEffectiveDoc);
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
        initPlayback();
        try {
            m_pDisplayEngine->render(m_pRootNode);
            if (m_pDisplayEngine->wasFrameLate()) {
                ThreadProfiler::get()->dumpFrame();
            }
            ThreadProfiler::get()->start();

            while (!m_bStopping) {
                doFrame();
            }
        } catch (...) {
            cleanup();
            m_bIsPlaying = false;
            throw;
        }
        cleanup();

    } catch (Exception& ex) {
        AVG_TRACE(Logger::ERROR, ex.GetStr());
        throw;
    }
}

void Player::stop ()
{
    if (m_bIsPlaying) {
        m_bStopping = true;
    } else {
        cleanup();
    }
}
        
void Player::initPlayback()
{
    m_bIsPlaying = true;
    if (!m_pRootNode) {
        throw Exception(AVG_ERR_NO_NODE, "Play called, but no xml file loaded.");
    }
    
    initGraphics();
    initAudio();
    
    m_pRootNode->setRenderingEngines(m_pDisplayEngine, m_pAudioEngine);

    m_pEventDispatcher->addSource(m_pEventSource);
    m_pEventDispatcher->addSource(m_pTestHelper);
    m_pEventDispatcher->addSink(this);

    m_pDisplayEngine->initRender();
    m_bStopping = false;
    if (m_pTracker) {
        m_pTracker->start();
    }

    m_PlayStartTime = TimeSource::get()->getCurrentMillisecs();
    m_FrameTime = 0;
    m_NumFrames = 0;
    m_MaxGPUMemUsed = 0;
}

bool Player::isPlaying()
{
    return m_bIsPlaying;
}

void Player::setFramerate(double rate) {
    if (m_bIsPlaying) {
        m_pDisplayEngine->setFramerate(rate);
    } else {
        m_DP.m_Framerate = rate;
        m_DP.m_VBRate = 0;
    }
}

bool Player::setVBlankFramerate(int rate) {
    // TODO: Why does this function return anything?
    if (m_bIsPlaying) {
        return m_pDisplayEngine->setVBlankRate(rate);
    } else {
        m_DP.m_Framerate = 0;
        m_DP.m_VBRate = rate;
        return true;
    }
}
        
double Player::getEffectiveFramerate() {
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

TrackerEventSource * Player::addTracker(const string& sConfigFilename)
{
    TrackerConfig Config;
    Config.load(sConfigFilename);
    CameraPtr pCamera;

    string sSource = Config.getParam("/camera/source/@value");
    string sDevice = Config.getParam("/camera/device/@value");
    IntPoint Size(Config.getPointParam("/camera/size/"));
    string sPixFmt = Config.getParam("/camera/format/@value");
    double FPS = Config.getDoubleParam("/camera/fps/@value");

    if (!m_pEventDispatcher) {
        throw Exception(AVG_ERR_UNSUPPORTED, "You must use loadFile() before addTracker().");
    }

    if (sSource == "v4l") {
#ifdef AVG_ENABLE_V4L2
        int Channel = Config.getIntParam("/camera/channel/@value");
        AVG_TRACE(Logger::CONFIG, "Adding a Tracker for V4L camera " << sDevice
                << " size=" << Size << " channel=" << Channel << " format=" 
                << sPixFmt);
        
        pCamera = CameraPtr(new V4LCamera(sDevice, Channel, Size, sPixFmt, false));
#else
        AVG_TRACE(Logger::ERROR, "Video4Linux camera tracker requested, but " 
                "Video4Linux support not compiled in.");
        exit(1);
#endif
    } else if (sSource == "fw") {
        AVG_TRACE(Logger::CONFIG, "Adding a Tracker for FW camera " << 
                sDevice << " size=" << Size << " format=" << sPixFmt);
        pCamera = CameraPtr(new FWCamera(sDevice, Size, sPixFmt, FPS, false));
    } else if (sSource == "ds") {
#ifdef AVG_ENABLE_DSHOW        
        AVG_TRACE(Logger::CONFIG, "Adding a Tracker for DS camera " << 
                sDevice << " size=" << Size << " format=" << sPixFmt);
        pCamera = CameraPtr(new DSCamera(sDevice, Size, sPixFmt, FPS, false));
#else
        AVG_TRACE(Logger::ERROR, "DS camera tracker requested, but " 
                "DS support not compiled in.");
        exit(1);
#endif        
    } else {
        throw Exception(AVG_ERR_INVALID_CAPTURE,
                string("Invalid camera source value '")+sSource+
                        "'. Can't initialize tracker.");
    }
    
    m_pTracker = new TrackerEventSource(pCamera, Config, 
            IntPoint(m_DP.m_Width, m_DP.m_Height), true);
    m_pEventDispatcher->addSource(m_pTracker);
    if (m_bIsPlaying) {
        m_pTracker->start();
    }

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
            if (it == m_PendingTimeouts.begin()) {
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
    return m_MouseState.getLastEvent();
}

Bitmap * Player::screenshot()
{
    BitmapPtr pBmp = m_pDisplayEngine->screenshot();
    return new Bitmap(*pBmp);
}

void Player::showCursor(bool bShow)
{
    if (m_pDisplayEngine) {
        m_pDisplayEngine->showCursor(bShow);
    } else {
        m_DP.m_bShowCursor = bShow;
    }
}

void Player::setEventCapture(AreaNodePtr pNode, int cursorID) {
    std::map<int, AreaNodeWeakPtr>::iterator it = m_pEventCaptureNode.find(cursorID);
    if (it!=m_pEventCaptureNode.end()&&!it->second.expired()) {
        throw Exception(AVG_ERR_INVALID_CAPTURE, "setEventCapture called for '"
                + pNode->getID() + "', but cursor already captured by '"
                + it->second.lock()->getID() + "'.");
    } else {
        m_pEventCaptureNode[cursorID] = pNode;
    }
}

void Player::releaseEventCapture(int cursorID) {
    std::map<int, AreaNodeWeakPtr>::iterator it = m_pEventCaptureNode.find(cursorID);
    if(it==m_pEventCaptureNode.end()||(it->second.expired()) ) {
        throw Exception(AVG_ERR_INVALID_CAPTURE,
                "releaseEventCapture called, but cursor not captured.");
    } else {
        m_pEventCaptureNode.erase(cursorID);
    }

}

NodePtr Player::getElementByID (const std::string& id)
{
    if (m_IDMap.find(id) != m_IDMap.end()) {
        return m_IDMap.find(id)->second;
    } else {
        AVG_TRACE(Logger::WARNING, "getElementByID(\"" << id << "\") failed.");
        return NodePtr();
    }
}
        
void Player::addNodeID(NodePtr pNode)
{
    const string& id = pNode->getID();
    if (id != "") {
        if (m_IDMap.find(id) != m_IDMap.end() &&
            m_IDMap.find(id)->second != pNode)
        {
            throw (Exception (AVG_ERR_XML_DUPLICATE_ID,
                string("Error: duplicate id ")+id));
        }
        m_IDMap.insert(NodeIDMap::value_type(id, pNode));
    }
}

void Player::removeNodeID(const std::string& id)
{
    if (id != "") {
        std::map<std::string, NodePtr>::iterator it;
        it = m_IDMap.find(id);
        if (it != m_IDMap.end()) {
            m_IDMap.erase(it);
        } else {
            cerr << "removeNodeID(\"" << id << "\") failed." << endl;
            assert(false);
        }
    }
}

AVGNodePtr Player::getRootNode ()
{
    return m_pRootNode;
}

void Player::registerFrameListener(IFrameListener* pListener)
{
    m_Listeners.push_back(pListener);
}

void Player::unregisterFrameListener(IFrameListener* pListener)
{
    std::vector<IFrameListener*>::iterator it;
    for (it=m_Listeners.begin(); it != m_Listeners.end(); ++it) {
        if (*it == pListener) {
            m_Listeners.erase(it);
            break;
        }
    }
}

string Player::getCurDirName()
{
    return m_CurDirName;
}
        
std::string Player::getRootMediaDir()
{
    string sMediaDir;
    if (m_pRootNode) {
        sMediaDir = m_pRootNode->getEffectiveMediaDir();
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

static ProfilingZone MainProfilingZone("Player - Total frame time");
static ProfilingZone TimersProfilingZone("Player - handleTimers");
static ProfilingZone EventsProfilingZone("Player - dispatch events");
static ProfilingZone RenderProfilingZone("Player - render");
static ProfilingZone ListenerProfilingZone("Player - listeners");

void Player::doFrame ()
{
    {
        ScopeTimer Timer(MainProfilingZone);
        if (m_bFakeFPS) {
            m_NumFrames++;
            m_FrameTime = (long long)((m_NumFrames*1000.0)/m_FakeFPS);
        } else {
            m_FrameTime = m_pDisplayEngine->getDisplayTime();
//            m_FrameTime = TimeSource::get()->getCurrentMillisecs()-m_PlayStartTime;
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
        if (!m_bStopping) {
            ScopeTimer Timer(RenderProfilingZone);
            if (m_bPythonAvailable) {
                Py_BEGIN_ALLOW_THREADS;
                m_pDisplayEngine->render(m_pRootNode);
                Py_END_ALLOW_THREADS;
            } else {
                m_pDisplayEngine->render(m_pRootNode);
            }
            if (getGPUMemoryUsage() > m_MaxGPUMemUsed) {
                m_MaxGPUMemUsed = getGPUMemoryUsage();
            }
        }
        {
            ScopeTimer Timer(ListenerProfilingZone);
            for (unsigned int i=0; i<m_Listeners.size(); ++i) {
                m_Listeners[i]->onFrameEnd();
            }
        }
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

double Player::getFramerate ()
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

void Player::initConfig() {
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

    m_DP.m_WindowWidth = atoi(pMgr->getOption("scr", "windowwidth")->c_str());
    m_DP.m_WindowHeight = atoi(pMgr->getOption("scr", "windowheight")->c_str());

    if (m_DP.m_bFullscreen && (m_DP.m_WindowWidth != 0 || m_DP.m_WindowHeight != 0)) {
        AVG_TRACE(Logger::ERROR, 
                "Can't set fullscreen and window size at once. Aborting.");
        exit(-1);
    }
    if (m_DP.m_WindowWidth != 0 && m_DP.m_WindowHeight != 0) {
        AVG_TRACE(Logger::ERROR, "Can't set window width and height at once");
        AVG_TRACE(Logger::ERROR, 
                "(aspect ratio is determined by avg file). Aborting.");
        exit(-1);
    }

    m_AP.m_Channels = atoi(pMgr->getOption("aud", "channels")->c_str());
    m_AP.m_SampleRate = atoi(pMgr->getOption("aud", "samplerate")->c_str());
    m_AP.m_OutputBufferSamples = atoi(pMgr->getOption("aud", "outputbuffersamples")->c_str());

    m_bUsePOW2Textures = pMgr->getBoolOption("scr", "usepow2textures", false);

    const string * psYCbCrMode =pMgr->getOption("scr", "ycbcrmode");
    if (psYCbCrMode == 0 || *psYCbCrMode == "shader") {
        m_YCbCrMode = OGL_SHADER;
    } else if (*psYCbCrMode == "mesa") {
        m_YCbCrMode = OGL_MESA;
    } else if (*psYCbCrMode == "apple") {
        m_YCbCrMode = OGL_APPLE;
    } else if (*psYCbCrMode == "none") {
        m_YCbCrMode = OGL_NONE;
    } else {
        AVG_TRACE(Logger::ERROR, 
                "avgrc: ycbcrmode must be shader, mesa, apple or none. Current value is " 
                << *psYCbCrMode << ". Aborting." );
        exit(-1);
    }

    const string * psVSyncMode =pMgr->getOption("scr", "vsyncmode");
    if (psVSyncMode == 0 || *psVSyncMode == "auto") {
        m_VSyncMode = VSYNC_AUTO;
    } else if (*psVSyncMode == "ogl") {
        m_VSyncMode = VSYNC_OGL;
    } else if (*psVSyncMode == "dri") {
        m_VSyncMode = VSYNC_DRI;
    } else if (*psVSyncMode == "none") {
        m_VSyncMode = VSYNC_NONE;
    } else {
        AVG_TRACE(Logger::ERROR, 
                "avgrc: vsyncmode must be auto, ogl, dri or none. Current value is " 
                << *psVSyncMode << ". Aborting." );
        exit(-1);
    }

    m_bUsePixelBuffers = pMgr->getBoolOption("scr", "usepixelbuffers", true);
    m_MultiSampleSamples = pMgr->getIntOption("scr", "multisamplesamples", 1);
    pMgr->getGammaOption("scr", "gamma", m_DP.m_Gamma);
}

void Player::initGraphics()
{
    // Init display configuration.
    AVG_TRACE(Logger::CONFIG, "Display bpp: " << m_DP.m_BPP);

    if (!m_pDisplayEngine) {
        AVG_TRACE(Logger::CONFIG, "Requested OpenGL configuration: ");
        AVG_TRACE(Logger::CONFIG, "  POW2 textures: " 
                << (m_bUsePOW2Textures?"true":"false"));
        string sMode;
        switch (m_YCbCrMode) {
            case OGL_NONE:
                AVG_TRACE(Logger::CONFIG, "  No YCbCr texture support.");
                break;
            case OGL_MESA:
                AVG_TRACE(Logger::CONFIG, "  Mesa YCbCr texture support.");
                break;
            case OGL_APPLE:
                AVG_TRACE(Logger::CONFIG, "  Apple YCbCr texture support.");
                break;
            case OGL_SHADER:
                AVG_TRACE(Logger::CONFIG, "  Fragment shader YCbCr texture support.");
                break;
        }
        AVG_TRACE(Logger::CONFIG, "  Use pixel buffers: " 
                << (m_bUsePixelBuffers?"true":"false"));
        AVG_TRACE(Logger::CONFIG, "  Multisample samples: " 
                << m_MultiSampleSamples);
        switch (m_VSyncMode) {
            case VSYNC_AUTO:
                AVG_TRACE(Logger::CONFIG, "  Auto vsync");
                break;
            case VSYNC_OGL:
                AVG_TRACE(Logger::CONFIG, "  OpenGL vsync");
                break;
            case VSYNC_DRI:
                AVG_TRACE(Logger::CONFIG, "  DRI vsync");
                break;
            case VSYNC_NONE:
                AVG_TRACE(Logger::CONFIG, "  No vsync");
                break;
        }

        m_pDisplayEngine = new SDLDisplayEngine ();
        m_pEventSource = 
            dynamic_cast<SDLDisplayEngine *>(m_pDisplayEngine);
    }
    SDLDisplayEngine * pSDLDisplayEngine = 
            dynamic_cast<SDLDisplayEngine*>(m_pDisplayEngine);
    if (pSDLDisplayEngine) {
        pSDLDisplayEngine->setOGLOptions(m_bUsePOW2Textures, m_YCbCrMode, 
                m_bUsePixelBuffers, m_MultiSampleSamples, m_VSyncMode);
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

void Player::internalLoad(const string& sAVG)
{
    try {
        if (m_pRootNode) {
            cleanup();
        }
        assert (!m_pRootNode);
        m_pEventDispatcher = EventDispatcherPtr(new EventDispatcher);

        char szBuf[1024];
        getcwd(szBuf, 1024);
        m_CurDirName = string(szBuf)+"/";
        
        xmlPedanticParserDefault(1);
        xmlDoValidityCheckingDefaultValue =0;

        xmlDocPtr doc;
        doc = xmlParseMemory(sAVG.c_str(), sAVG.length());
        if (!doc) {
            throw (Exception(AVG_ERR_XML_PARSE, ""));
        }

        if (m_bDirtyDTD)
            updateDTD();

        xmlValidCtxtPtr cvp = xmlNewValidCtxt();
        cvp->error = xmlParserValidityError;
        cvp->warning = xmlParserValidityWarning;
        int valid=xmlValidateDtd(cvp, doc, m_dtd);  
        xmlFreeValidCtxt(cvp);
        if (!valid) {
            throw (Exception(AVG_ERR_XML_VALID, ""));
        }
        xmlNodePtr xmlNode = xmlDocGetRootElement(doc);
        createNodeFromXml(doc, xmlNode, DivNodePtr());
        registerNode(m_pRootNode);
        m_DP.m_Height = int(m_pRootNode->getHeight());
        m_DP.m_Width = int(m_pRootNode->getWidth());
        
        xmlFreeDoc(doc);
    } catch (Exception& ex) {
        AVG_TRACE(Logger::ERROR, ex.GetStr());
        throw;
    } catch (Magick::Exception & ex) {
        AVG_TRACE(Logger::ERROR, ex.what());
        throw;
    }
}


void Player::registerNode(NodePtr pNode)
{
    addNodeID(pNode);    
    DivNodePtr pDivNode = boost::dynamic_pointer_cast<DivNode>(pNode);
    if (pDivNode) {
        for (int i=0; i<pDivNode->getNumChildren(); i++) {
            registerNode(pDivNode->getChild(i));
        }
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

NodePtr Player::createNode(const string& sType, const boost::python::dict& PyDict)
{
    NodePtr pNode = m_NodeRegistry.createNode(sType, PyDict);
    return pNode;
}

NodePtr Player::createNodeFromXmlString (const string& sXML)
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

NodePtr Player::createNodeFromXml (const xmlDocPtr xmlDoc, 
        const xmlNodePtr xmlNode, GroupNodeWeakPtr pParent)
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
        boost::dynamic_pointer_cast<Words>(curNode)->setTextFromNodeValue(s);
    }

    // If this is the root node, remember it.
    AVGNodePtr pRootNode = boost::dynamic_pointer_cast<AVGNode>(curNode);
    if (pRootNode) {
        m_pRootNode = pRootNode;
        m_pRootNode->setParent(DivNodeWeakPtr(), Node::NS_CONNECTED);
    }

    // If this is a container, recurse into children
    GroupNodePtr curGroup = boost::dynamic_pointer_cast<GroupNode>(curNode);
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

void Player::handleTimers()
{
    vector<Timeout *>::iterator it;
    m_bInHandleTimers = true;
    vector<Timeout *> IntervalsFired;
   
    it = m_PendingTimeouts.begin();
    while (it != m_PendingTimeouts.end() && (*it)->IsReady(getFrameTime()) && !m_bStopping)
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


bool Player::handleEvent(EventPtr pEvent)
{
    assert(pEvent);
   
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
        m_pRootNode->handleEvent(pKeyEvent);
        if (m_bStopOnEscape
                && pEvent->getType() == Event::KEYDOWN
                && pKeyEvent->getKeyCode() == avg::key::KEY_ESCAPE)
        {
            m_bStopping = true;
        }
    } else {
        switch(pEvent->getType()){
            case Event::QUIT:
                m_bStopping = true;
                break;
            default:
                AVG_TRACE(Logger::ERROR, "Unknown event type in Player::handleEvent.");
                break;
        }
    // Don't pass on any events.
    }
    return true; 
}

void Player::sendFakeEvents()
{
    std::map<int, CursorStatePtr>::iterator it;
    for (it=m_pLastCursorStates.begin(); it != m_pLastCursorStates.end(); ++it) {
        CursorStatePtr state = it->second;
        handleCursorEvent(state->getLastEvent(), true);
    }
}

void Player::handleCursorEvent(CursorEventPtr pEvent, bool bOnlyCheckCursorOver)
{
    DPoint pos(pEvent->getXPosition(), pEvent->getYPosition());
    int cursorID = pEvent->getCursorID();
    // Find all nodes under the cursor.
    vector<AreaNodeWeakPtr> pCursorNodes = getElementsByPos(pos);

    // Determine the nodes the event should be sent to.
    vector<AreaNodeWeakPtr> pDestNodes = pCursorNodes;
    bool bIsCapturing = false;
    if (m_pEventCaptureNode.find(cursorID) != m_pEventCaptureNode.end()) {
        AreaNodeWeakPtr pEventCaptureNode = m_pEventCaptureNode[cursorID];
        if (pEventCaptureNode.expired()) {
            m_pEventCaptureNode.erase(cursorID);
        } else {
            pDestNodes = vector<AreaNodeWeakPtr>();
            pDestNodes.push_back(pEventCaptureNode);
            bIsCapturing = true;
        }
    } 

    vector<AreaNodeWeakPtr> pLastCursorNodes;
    {
        map<int, CursorStatePtr>::iterator it;
        it = m_pLastCursorStates.find(cursorID);
        if (it != m_pLastCursorStates.end()) {
            pLastCursorNodes = it->second->getNodes();
        }
    }

    // Send out events.
    vector<AreaNodeWeakPtr>::const_iterator itLast;
    vector<AreaNodeWeakPtr>::iterator itCur;
    for (itLast = pLastCursorNodes.begin(); itLast != pLastCursorNodes.end(); ++itLast) {
        AreaNodePtr pLastNode = itLast->lock();
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
        AreaNodePtr pCurNode = itCur->lock();
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
        vector<AreaNodeWeakPtr>::iterator it;
        for (it = pDestNodes.begin(); it != pDestNodes.end(); ++it) {
            AreaNodePtr pNode = (*it).lock();
            if (pNode) {
                CursorEventPtr pNodeEvent =
                        boost::dynamic_pointer_cast<CursorEvent>(pEvent->cloneAs(pEvent->getType()));
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
            AreaNodePtr pNode = pDestNodes.begin()->lock();
            sendOver(pEvent, Event::CURSOROUT, pNode);
        } else {
            vector<AreaNodeWeakPtr>::iterator it;
            for (it = pCursorNodes.begin(); it != pCursorNodes.end(); ++it) {
                AreaNodePtr pNode = it->lock();
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

vector<AreaNodeWeakPtr> Player::getElementsByPos(const DPoint& pos) const
{
    vector<AreaNodeWeakPtr> Elements;
    AreaNodePtr pNode = m_pRootNode->getElementByPos(pos);
    while (pNode) {
        Elements.push_back(pNode);
        pNode = pNode->getParent();
    }
    return Elements;
}

DisplayEngine * Player::getDisplayEngine() const 
{
    return m_pDisplayEngine;
}

void Player::useFakeCamera(bool bFake)
{
    m_bUseFakeCamera = bFake;
}

void Player::stopOnEscape(bool bStop)
{
    m_bStopOnEscape = bStop;
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

void Player::sendOver(const CursorEventPtr pOtherEvent, Event::Type Type, 
                AreaNodePtr pNode)
{
    if (pNode) {
        EventPtr pNewEvent = pOtherEvent->cloneAs(Type);
        pNewEvent->setElement(pNode);
        m_pEventDispatcher->sendEvent(pNewEvent);
    }
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
    if (m_pRootNode) {
        m_pRootNode->disconnect();
    }
    m_pRootNode = AVGNodePtr();

    if (m_pTracker) {
        delete m_pTracker;
        m_pTracker = 0;
    }

    if (m_pDisplayEngine) {
        m_pDisplayEngine->deinitRender();
        m_pDisplayEngine->teardown();
    }
    if (m_pAudioEngine) {
        m_pAudioEngine->teardown();
    }
    AVG_TRACE(Logger::PROFILE, "Max. GPU memory used: " << m_MaxGPUMemUsed/1024 << "k");
    
    m_IDMap.clear();
    m_pEventDispatcher = EventDispatcherPtr();
    m_MouseState = MouseState();
    initConfig();
    m_FrameTime = 0;
    m_bIsPlaying = false;
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

long long Player::getGPUMemoryUsage()
{
    if (m_pDisplayEngine) {
        return m_pDisplayEngine->getGPUMemoryUsage();
    } else {
        return 0;
    }
}

void Player::setPluginPath(const string& newPath) {
    PluginManager::get().setSearchPath(newPath);
}

string Player::getPluginPath() const {
    return  PluginManager::get().getSearchPath();
}

void Player::loadPlugin(const std::string& name)
{
    AVG_TRACE(Logger::PLUGIN, "player loading plugin: '" << name << "'");   
    PluginManager::get().loadPlugin(name);
}

}
