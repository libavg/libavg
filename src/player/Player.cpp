//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2006 Ulrich von Zadow
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

#include "../avgconfig.h"
#undef PACKAGE
#undef PACKAGE_BUGREPORT
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION

#include "Player.h"

#include "avgdtd.h"
#include "AVGNode.h"
#include "DivNode.h"
#include "Words.h"
#include "Video.h"
#include "CameraNode.h"
#include "Image.h"
#include "PanoImage.h"

#include "TrackerEventSource.h"
#include "Event.h"
#include "MouseEvent.h"
#include "KeyEvent.h"

#ifdef AVG_ENABLE_DFB
#include "DFBDisplayEngine.h"
#endif
#ifdef AVG_ENABLE_GL
#include "SDLDisplayEngine.h"
#endif

#include "../base/FileHelper.h"
#include "../base/Exception.h"
#include "../base/Logger.h"
#include "../base/ConfigMgr.h"
#include "../base/XMLHelper.h"
#include "../base/Profiler.h"
#include "../base/ScopeTimer.h"
#include "../base/TimeSource.h"
#include "../base/MemHelper.h"

#include "../graphics/Rect.h"

#include "../imaging/Camera.h"
#include "../imaging/FakeCamera.h"

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

Player::Player()
    : m_pRootNode(),
      m_pDisplayEngine(0),
      m_TestHelper(this),
      m_pTracker(0),
      m_bInHandleTimers(false),
      m_bCurrentTimeoutDeleted(false), 
      m_pLastMouseNode(),
      m_pEventCaptureNode(),
      m_bUseFakeCamera(false),
      m_bIsPlaying(false),
      m_bFakeFPS(false),
      m_FakeFPS(0),
      m_FrameTime(0),
      m_PlayStartTime(0)
{
    ThreadProfilerPtr pThreadProfiler = ThreadProfilerPtr(new ThreadProfiler("Main"));
    Profiler::get().registerThreadProfiler(pThreadProfiler);
    initConfig();
    // Find and parse dtd.
    registerDTDEntityLoader("avg.dtd", g_pAVGDTD);
    string sDTDFName = "avg.dtd";
    m_dtd = xmlParseDTD(NULL, (const xmlChar*) sDTDFName.c_str());
    if (!m_dtd) {
        AVG_TRACE(Logger::WARNING, 
                "DTD not found at " << sDTDFName << ". Not validating xml files.");
    }
    m_pLastMouseNode[MOUSECURSORID]=NodePtr();
}

Player::~Player()
{
    if (m_pDisplayEngine) {
        delete m_pDisplayEngine;
    }
    if (m_pTracker) {
        delete m_pTracker;
    }
    if (m_dtd) {
        xmlFreeDtd(m_dtd);
    }
}

void Player::setDisplayEngine(DisplayEngineType engine)
{
    if (m_pRootNode) {
        throw Exception(AVG_ERR_VIDEO_INIT_FAILED,
                "Player::setDisplayEngine called after loadFile.");
    }
    switch (engine) {
        case DFB:
            m_sDisplaySubsystem="DFB";
            break;
        case OGL:
            m_sDisplaySubsystem="OGL";
            break;
        default:
            AVG_TRACE(Logger::ERROR,
                    "Unknown display engine type in setDisplayEngine. Aborting.");
            exit(-1);
    }
}

void Player::setResolution(bool bFullscreen, 
                int width, int height, int bpp)
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
        
void Player::setOGLOptions(bool bUsePOW2Textures, YCbCrMode DesiredYCbCrMode, 
                bool bUseRGBOrder, bool bUsePixelBuffers, int MultiSampleSamples)
{
    m_bUsePOW2Textures = bUsePOW2Textures;
    m_YCbCrMode = DesiredYCbCrMode;
    m_bUseRGBOrder = bUseRGBOrder;
    m_bUsePixelBuffers = bUsePixelBuffers;
    m_MultiSampleSamples = MultiSampleSamples;
}

void Player::loadFile (const std::string& filename)
{
    try {
        m_pEventDispatcher = EventDispatcherPtr(new EventDispatcher);
        AVG_TRACE(Logger::MEMORY, 
                std::string("Player::LoadFile(") + filename + ")");
        if (m_pRootNode) {
            cleanup();
        }
        assert (!m_pRootNode);

        // When loading an avg file, assets are loaded from a directory relative
        // to the file.
        char szBuf[1024];
        string RealFilename;
        if (filename[0] == '/') {
            RealFilename = filename; 
        } else {
            getcwd(szBuf, 1024);
            m_CurDirName = string(szBuf)+"/";
            RealFilename = m_CurDirName+filename;
        }
        m_CurDirName = RealFilename.substr(0, RealFilename.rfind('/')+1);
        
        xmlPedanticParserDefault(1);
        xmlDoValidityCheckingDefaultValue =0;

        xmlDocPtr doc;
        if (!fileExists(RealFilename)) {
            throw (Exception(AVG_ERR_FILEIO, 
                        string("File ")+RealFilename+" not found."));
        }
        doc = xmlParseFile(RealFilename.c_str());
        if (!doc) {
            throw (Exception(AVG_ERR_XML_PARSE, 
                        string("Error parsing xml document ")+RealFilename));
        }

        xmlValidCtxtPtr cvp = xmlNewValidCtxt();
        cvp->error = xmlParserValidityError;
        cvp->warning = xmlParserValidityWarning;
        int valid=xmlValidateDtd(cvp, doc, m_dtd);  
        xmlFreeValidCtxt(cvp);
        if (!valid) {
            throw (Exception(AVG_ERR_XML_PARSE, 
                    filename + " does not validate."));
        }
        xmlNodePtr xmlNode = xmlDocGetRootElement(doc);
        m_pRootNode = boost::dynamic_pointer_cast<AVGNode>
            (createNodeFromXml(doc, xmlNode, DivNodePtr()));
        m_pRootNode->setParent(DivNodeWeakPtr());
        m_DP.m_Height = getDefaultedIntAttr (xmlNode, "height", 0);
        m_DP.m_Width = getDefaultedIntAttr (xmlNode, "width", 0);
        registerNode(m_pRootNode);

        // Reset the directory to load assets from to the current dir.
        getcwd(szBuf, 1024);
        m_CurDirName = string(szBuf)+"/";
        
        xmlFreeDoc(doc);
    } catch (Exception& ex) {
        AVG_TRACE(Logger::ERROR, ex.GetStr());
        throw;
    } catch (Magick::Exception & ex) {
        AVG_TRACE(Logger::ERROR, ex.what());
        throw;
    }
}

void Player::play()
{
    m_bIsPlaying = true;
    try {
        if (!m_pRootNode) {
            AVG_TRACE(Logger::ERROR, "play called, but no xml file loaded.");
        }
        assert(m_pRootNode);
        initGraphics();
        m_pRootNode->setDisplayEngine(m_pDisplayEngine);
       
        m_pEventDispatcher->addSource(m_pEventSource);
        m_pEventDispatcher->addSource(&m_TestHelper);
        m_pEventDispatcher->addSink(&m_EventDumper);
        m_pEventDispatcher->addSink(this);
        
        m_pDisplayEngine->initRender();
        m_bStopping = false;

        m_PlayStartTime = TimeSource::get()->getCurrentMillisecs();
        m_FrameTime = 0;
        m_NumFrames = 0;
        ThreadProfiler::get()->start();
        m_pDisplayEngine->render(m_pRootNode, true);
        if (m_pDisplayEngine->wasFrameLate()) {
            ThreadProfiler::get()->dumpFrame();
        }
        
        try {
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
    m_bIsPlaying = false;
}

void Player::stop ()
{
    if (m_bIsPlaying) {
        m_bStopping = true;
    } else {
        cleanup();
    }
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

unsigned Player::getMemUsed()
{
    return avg::getMemUsed();
}

TestHelper * Player::getTestHelper()
{
    return &m_TestHelper;
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
}

long long Player::getFrameTime()
{
    return m_FrameTime;
}

TrackerEventSource * Player::addTracker(std::string sDevice, 
        std::string sMode)
{
    AVG_TRACE(Logger::CONFIG, "Adding a Tracker for camera " << sDevice << " using "
            << sMode << ".");
    TrackerConfig Config;
    Config.load();
    CameraPtr pCamera;
    pCamera = CameraPtr(new Camera(sDevice, Config.m_FPS, sMode, false));
    m_pTracker = new TrackerEventSource(pCamera, Config, IntPoint(m_DP.m_Width, m_DP.m_Height), true);
    m_pEventDispatcher->addSource(m_pTracker);
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

const Event& Player::getCurEvent() const
{
    return *m_pCurEvent;
}

const MouseEvent& Player::getMouseState() const
{
    return m_pEventDispatcher->getLastMouseEvent();
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

void Player::setEventCapture(NodeWeakPtr pNode, int cursorID) {
    std::map<int, NodeWeakPtr>::iterator it = m_pEventCaptureNode.find(cursorID);
    if (it!=m_pEventCaptureNode.end()&&!it->second.expired()) {
        throw Exception(AVG_ERR_INVALID_CAPTURE,
                "setEventCapture called, but cursor already captured.");
    } else {
        m_pEventCaptureNode[cursorID] = pNode;
    }
}

void Player::releaseEventCapture(NodeWeakPtr pNode, int cursorID) {
    std::map<int, NodeWeakPtr>::iterator it = m_pEventCaptureNode.find(cursorID);
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
            AVG_TRACE(Logger::ERROR, "removeNodeID("+id+") failed.");
            exit(1);
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
        }
        if (!m_bStopping) {
            ScopeTimer Timer(RenderProfilingZone);
            m_pDisplayEngine->render(m_pRootNode, false);
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
    
    m_sDisplaySubsystem = *pMgr->getOption("scr", "subsys");
   
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

    if (m_sDisplaySubsystem == "DFB") {
        if  (pMgr->getOption("scr", "usepow2textures") != 0 || 
             pMgr->getOption("scr", "ycbcrmode") != 0 ||
             pMgr->getOption("scr", "usergborder") != 0 ||
             pMgr->getOption("scr", "usepixelbuffers") != 0) 
        {
            AVG_TRACE(Logger::WARNING, 
                    "avgrc: usepow2textures, ycbcrmode, usergborder and usepixelbuffers are unsupported in the DirectFB backend. Ignoring.");
        }
    } else {
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

        m_bUseRGBOrder = pMgr->getBoolOption("scr", "usergborder", false);
        m_bUsePixelBuffers = pMgr->getBoolOption("scr", "usepixelbuffers", true);
        m_MultiSampleSamples = pMgr->getIntOption("scr", "multisamplesamples", 1);
        pMgr->getGammaOption("scr", "gamma", m_DP.m_Gamma);
    }
}

void Player::initGraphics()
{
    // Init display configuration.
    AVG_TRACE(Logger::CONFIG, "Display subsystem: " << 
            m_sDisplaySubsystem);
    AVG_TRACE(Logger::CONFIG, "Display bpp: " << m_DP.m_BPP);
    AVG_TRACE(Logger::CONFIG, "Display fullscreen: "
            << (m_DP.m_bFullscreen?"true":"false"));

    if (!m_pDisplayEngine) {
        if (m_sDisplaySubsystem == "DFB") {
#ifdef AVG_ENABLE_DFB
            m_pDisplayEngine = new DFBDisplayEngine ();
            m_pEventSource =
                dynamic_cast<DFBDisplayEngine *>(m_pDisplayEngine);
#else
            AVG_TRACE(Logger::ERROR,
                    "Display subsystem set to DFB but no DFB support compiled."
                    << " Aborting.");
            exit(-1);
#endif
        } else if (m_sDisplaySubsystem == "OGL") {
#ifdef AVG_ENABLE_GL
            AVG_TRACE(Logger::CONFIG, "Requested OGL configuration: ");
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
            AVG_TRACE(Logger::CONFIG, "  RGB order: " 
                    << (m_bUseRGBOrder?"true":"false"));
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
#else
            AVG_TRACE(Logger::ERROR,
                    "Display subsystem set to GL but no GL support compiled."
                    << " Aborting.");
            exit(-1);
#endif
        } else {
            AVG_TRACE(Logger::ERROR, 
                    "Display subsystem set to unknown value " << 
                    m_sDisplaySubsystem << ". Aborting.");
            exit(-1);
        }
    }
    SDLDisplayEngine * pSDLDisplayEngine = 
            dynamic_cast<SDLDisplayEngine*>(m_pDisplayEngine);
    if (pSDLDisplayEngine) {
        pSDLDisplayEngine->setOGLOptions(m_bUsePOW2Textures, m_YCbCrMode, 
                m_bUseRGBOrder, m_bUsePixelBuffers, m_MultiSampleSamples,
                m_VSyncMode);
    }
    m_pDisplayEngine->init(m_DP);
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

NodePtr Player::createNodeFromXmlString (const string& sXML)
{
    try {
        xmlPedanticParserDefault(1);
        xmlDoValidityCheckingDefaultValue =0;
        
        xmlDocPtr doc;
        doc = xmlParseMemory(sXML.c_str(), sXML.length());
        if (!doc) {
            throw (Exception(AVG_ERR_XML_PARSE, 
                        string("Error parsing xml:\n  ")+sXML));
        }
        NodePtr pNode = createNodeFromXml(doc, xmlDocGetRootElement(doc), DivNodePtr());

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
    } catch (Exception& ex) {
        AVG_TRACE(Logger::ERROR, ex.GetStr());
        return NodePtr();
    } catch (Magick::Exception& ex) {
        AVG_TRACE(Logger::ERROR, ex.what());
        return NodePtr();
    }
}

NodePtr Player::createNodeFromXml (const xmlDocPtr xmlDoc, 
        const xmlNodePtr xmlNode, DivNodeWeakPtr pParent)
{
    const char * nodeType = (const char *)xmlNode->name;
    NodePtr curNode;
    
    string id = getDefaultedStringAttr (xmlNode, "id", "");
    if (!strcmp (nodeType, "avg")) {
        curNode = NodePtr(new AVGNode(xmlNode, this));
    } else if (!strcmp (nodeType, "div")) {
        curNode = NodePtr(new DivNode(xmlNode, this));
    } else if (!strcmp (nodeType, "image")) {
        curNode = NodePtr(new Image(xmlNode, this));
    } else if (!strcmp (nodeType, "words")) {
        curNode = NodePtr(new Words(xmlNode, this));
        string s = getXmlChildrenAsString(xmlDoc, xmlNode);
        boost::dynamic_pointer_cast<Words>(curNode)->initText(s);
    } else if (!strcmp (nodeType, "video")) {
        curNode = NodePtr(new Video(xmlNode, this));
    } else if (!strcmp (nodeType, "camera")) {
        curNode = NodePtr(new CameraNode(xmlNode, this));
    }
    else if (!strcmp (nodeType, "panoimage")) {
        curNode = NodePtr(new PanoImage(xmlNode, this));
    } 
    else if (!strcmp (nodeType, "text") || 
               !strcmp (nodeType, "comment")) {
        // Ignore whitespace & comments
        return NodePtr();
    } else {
        throw (Exception (AVG_ERR_XML_NODE_UNKNOWN, 
            string("Unknown node type ")+(const char *)nodeType+" encountered."));
    }
    curNode->setThis(curNode);
    // If this is a container, recurse into children
    DivNodePtr curDivNode = boost::dynamic_pointer_cast<DivNode>(curNode);
    if (curDivNode) {
        xmlNodePtr curXmlChild = xmlNode->xmlChildrenNode;
        while (curXmlChild) {
            NodePtr curChild = createNodeFromXml (xmlDoc, curXmlChild, 
                    curDivNode);
            if (curChild) {
                curDivNode->addChild(curChild);
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
        if (!m_bCurrentTimeoutDeleted) {
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


bool Player::handleEvent(Event * pEvent)
{
    m_pCurEvent = pEvent;
    assert(pEvent); 
    if(CursorEvent * pCursorEvent = dynamic_cast<CursorEvent*>(pEvent)) {
        DPoint pos(pCursorEvent->getXPosition(), 
                pCursorEvent->getYPosition());
        int cursorID = pCursorEvent->getCursorID();
        NodePtr pNode;
        if (m_pEventCaptureNode.find(cursorID) != m_pEventCaptureNode.end()) {
            NodeWeakPtr pEventCaptureNode = m_pEventCaptureNode[cursorID];
            if (pEventCaptureNode.expired()) {
                m_pEventCaptureNode.erase(cursorID);
            } else {
                pNode = m_pEventCaptureNode[cursorID].lock();
            }
        } 
        if (!pNode) {
            if (pEvent->getType() == Event::CURSOROVER ||
                    pEvent->getType() == Event::CURSOROUT)
            {
                pNode = pCursorEvent->getElement();
            } else {
                pNode = m_pRootNode->getElementByPos(pos);
            }
        }
        if (pNode != m_pLastMouseNode[cursorID] && 
                pEvent->getType() != Event::CURSOROVER &&
                pEvent->getType() != Event::CURSOROUT)
        {
            if (m_pLastMouseNode[cursorID] && m_pLastMouseNode[cursorID]->getSensitive()) {
                sendOver(pCursorEvent, Event::CURSOROUT, 
                        m_pLastMouseNode[cursorID]);
            }
            if (pNode && pNode->getSensitive()) {
                sendOver(pCursorEvent, Event::CURSOROVER, pNode);
            }
            m_pLastMouseNode[cursorID] = pNode;
        }
        if (pCursorEvent->getType() == Event::CURSORUP && 
            pCursorEvent->getSource() != CursorEvent::MOUSE) 
        {
            m_pLastMouseNode.erase(cursorID);
        }
        if (pNode && pNode->getSensitive()) {
            m_pCurEvent = pEvent;
            pNode->handleEvent(pCursorEvent);
        }
    } else if ( KeyEvent * pKeyEvent = dynamic_cast<KeyEvent*>(pEvent)){
        m_pCurEvent = pEvent;
        m_pRootNode->handleEvent(pKeyEvent);
        if (pEvent->getType() == Event::KEYDOWN &&
            pKeyEvent->getKeyCode() == 27) 
        {
            m_bStopping = true;
        }
    }else {
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

DisplayEngine * Player::getDisplayEngine() const 
{
    return m_pDisplayEngine;
}

void Player::useFakeCamera(bool bFake)
{
    m_bUseFakeCamera = bFake;
}

void Player::sendOver(CursorEvent * pOtherEvent, Event::Type Type, 
                NodePtr pNode)
{
    Event * pNewEvent = pOtherEvent->cloneAs(Type);
    pNewEvent->setElement(pNode);
    m_pEventDispatcher->sendEvent(pNewEvent);
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
    if (m_pDisplayEngine) {
        m_pDisplayEngine->deinitRender();
        m_pDisplayEngine->teardown();
    }
    m_pRootNode = AVGNodePtr();
    m_pLastMouseNode.clear();
    m_IDMap.clear();
    m_pEventDispatcher = EventDispatcherPtr();
    initConfig();
    m_FrameTime = 0;
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

}
