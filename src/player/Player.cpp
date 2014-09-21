//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2014 Ulrich von Zadow
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
#include "SoundNode.h"
#include "LineNode.h"
#include "RectNode.h"
#include "CurveNode.h"
#include "PolyLineNode.h"
#include "PolygonNode.h"
#include "CircleNode.h"
#include "MeshNode.h"
#include "FontStyle.h"
#include "PluginManager.h"
#include "TextEngine.h"
#include "TestHelper.h"
#include "MainCanvas.h"
#include "OffscreenCanvas.h"
#include "TrackerInputDevice.h"
#include "DisplayEngine.h"
#include "MultitouchInputDevice.h"
#include "TUIOInputDevice.h"
#include "OGLSurface.h"
#include "SDLWindow.h"
#if defined(_WIN32) && defined(SM_DIGITIZER)
    #include "Win7TouchInputDevice.h"
#endif
#if defined(HAVE_XI2_1) || defined(HAVE_XI2_2) 
    #include "XInputMTInputDevice.h"
#endif
#include "Contact.h"
#include "KeyEvent.h"
#include "MouseEvent.h"
#include "EventDispatcher.h"
#include "PublisherDefinition.h"
#include "BitmapManager.h"

#include "../base/FileHelper.h"
#include "../base/StringHelper.h"
#include "../base/OSHelper.h"
#include "../base/Exception.h"
#include "../base/Logger.h"
#include "../base/ConfigMgr.h"
#include "../base/XMLHelper.h"
#include "../base/ScopeTimer.h"
#include "../base/WorkerThread.h"
#include "../base/DAG.h"

#include "../graphics/BitmapLoader.h"
#include "../graphics/ShaderRegistry.h"
#include "../graphics/Display.h"
#include "../graphics/GLContextManager.h"

#include "../imaging/Camera.h"

#include "../audio/AudioEngine.h"

#include <libxml/xmlmemory.h>

#ifdef _WIN32
#include <direct.h>
#define getcwd _getcwd
#endif

#include <iostream>

#ifdef __linux
#include <fenv.h>
#endif

#include <glib-object.h>
#include <typeinfo>

using namespace std;
using namespace boost;

namespace avg {

Player * Player::s_pPlayer=0;

Player::Player()
    : Publisher("Player"),
      m_pDisplayEngine(),
      m_bDisplayEngineBroken(false),
      m_bIsTraversingTree(false),
      m_pMultitouchInputDevice(),
      m_bInHandleTimers(false),
      m_bCurrentTimeoutDeleted(false),
      m_bKeepWindowOpen(false),
      m_bStopOnEscape(true),
      m_bIsPlaying(false),
      m_bFakeFPS(false),
      m_FakeFPS(0),
      m_FrameTime(0),
      m_Volume(1),
      m_bPythonAvailable(true),
      m_pLastMouseEvent(new MouseEvent(Event::CURSOR_MOTION, false, false, false, 
            IntPoint(-1, -1), MouseEvent::NO_BUTTON, glm::vec2(-1, -1), 0)),
      m_EventHookPyFunc(Py_None),
      m_bMouseEnabled(true)
{
    string sDummy;
#ifdef _WIN32
    if (getEnv("AVG_WIN_CRASH_SILENTLY", sDummy)) {
        DWORD dwMode = SetErrorMode(SEM_NOGPFAULTERRORBOX);
        SetErrorMode(dwMode | SEM_NOGPFAULTERRORBOX);
    }
#endif
    m_pContextManager = GLContextManagerPtr(new GLContextManager());
#ifdef __linux
// Turning this on causes fp exceptions in the linux nvidia drivers.
//    feenableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);
#endif
    setAffinityMask(true);

    if (s_pPlayer) {
        throw Exception(AVG_ERR_UNKNOWN, "Player has already been instantiated.");
    }
    ThreadProfiler* pProfiler = ThreadProfiler::get();
    pProfiler->setName("main");

    DisplayEngine::initSDL();
    initConfig();

    FontStyle::registerType();
    Node::registerType();
    AreaNode::registerType();
    RasterNode::registerType();
    VectorNode::registerType();
    FilledVectorNode::registerType();

    DivNode::registerType();
    CanvasNode::registerType();
    OffscreenCanvasNode::registerType();
    AVGNode::registerType();
    ImageNode::registerType();
    WordsNode::registerType();
    VideoNode::registerType();
    CameraNode::registerType();
    SoundNode::registerType();
    LineNode::registerType();
    RectNode::registerType();
    CurveNode::registerType();
    PolyLineNode::registerType();
    PolygonNode::registerType();
    CircleNode::registerType();
    MeshNode::registerType();

    Contact::registerType();

    m_pTestHelper = TestHelperPtr(new TestHelper());

    s_pPlayer = this;

    m_CurDirName = getCWD();
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
    m_pMainCanvas = MainCanvasPtr();
    if (m_pDisplayEngine) {
        m_pDisplayEngine->teardown();
    }
    DisplayEngine::quitSDL();
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
    errorIfMultiDisplay("Player.setResolution");
    errorIfPlaying("Player.setResolution");

    m_DP.setResolution(bFullscreen, width, height, bpp);
}

bool Player::isFullscreen()
{
    return m_DP.isFullscreen();
}

void Player::setWindowFrame(bool bHasWindowFrame)
{
    errorIfMultiDisplay("Player.setWindowFrame");
    errorIfPlaying("Player.setWindowFrame");
    m_DP.getWindowParams(0).m_bHasWindowFrame = bHasWindowFrame;
}

void Player::setWindowPos(int x, int y)
{
    errorIfMultiDisplay("Player.setWindowPos");
    errorIfPlaying("Player.setWindowPos");
    WindowParams& wp = m_DP.getWindowParams(0);
    wp.m_Pos.x = x;
    wp.m_Pos.y = y;
}

void Player::setWindowTitle(const string& sTitle)
{
    m_pDisplayEngine->setWindowTitle(sTitle);
}

void Player::setWindowConfig(const string& sFileName)
{
    m_DP.setConfig(sFileName);
}

void Player::useGLES(bool bGLES)
{
    errorIfPlaying("Player.useGLES");
    m_GLConfig.m_bGLES = bGLES;
#ifdef AVG_ENABLE_EGL
    m_GLConfig.m_bGLES = true;
#endif
    BitmapLoader::init(!m_GLConfig.m_bGLES);
}

void Player::setOGLOptions(bool bUsePOTTextures, bool bUsePixelBuffers, 
        int multiSampleSamples, GLConfig::ShaderUsage shaderUsage,
        bool bUseDebugContext)
{
    errorIfPlaying("Player.setOGLOptions");
    m_GLConfig.m_bUsePOTTextures = bUsePOTTextures;
    m_GLConfig.m_bUsePixelBuffers = bUsePixelBuffers;
    setMultiSampleSamples(multiSampleSamples);
    if (shaderUsage != GLConfig::AUTO) {
        m_GLConfig.m_ShaderUsage = shaderUsage;
    }
    m_GLConfig.m_bUseDebugContext = bUseDebugContext;
}

void Player::setMultiSampleSamples(int multiSampleSamples)
{
    errorIfPlaying("Player.setMultiSampleSamples");
    if (multiSampleSamples < 1) {
        throw Exception(AVG_ERR_OUT_OF_RANGE,
                "MultiSampleSamples must be 1 or greater (was " +
                toString(multiSampleSamples) + ").");
    }
    m_GLConfig.m_MultiSampleSamples = multiSampleSamples;
}

void Player::setAudioOptions(int samplerate, int channels)
{
    errorIfPlaying("Player.setAudioOptions");
    m_AP.m_SampleRate = samplerate;
    m_AP.m_Channels = channels;
}

void Player::enableGLErrorChecks(bool bEnable)
{
    GLContext::enableErrorChecks(bEnable);
}
        
glm::vec2 Player::getScreenResolution()
{
    return glm::vec2(Display::get()->getScreenResolution());
}

float Player::getPixelsPerMM()
{
    return Display::get()->getPixelsPerMM();
}

glm::vec2 Player::getPhysicalScreenDimensions()
{
    return Display::get()->getPhysicalScreenDimensions();
}

void Player::assumePixelsPerMM(float ppmm)
{
    Display::get()->assumePixelsPerMM(ppmm);
}

CanvasPtr Player::loadFile(const string& sFilename)
{
    errorIfPlaying("Player.loadFile");
    NodePtr pNode = loadMainNodeFromFile(sFilename);
    if (m_pMainCanvas) {
        cleanup(false);
    }

    initMainCanvas(pNode);
    
    return m_pMainCanvas;
}

CanvasPtr Player::loadString(const string& sAVG)
{
    errorIfPlaying("Player.loadString");
    if (m_pMainCanvas) {
        cleanup(false);
    }

    NodePtr pNode = loadMainNodeFromString(sAVG);
    initMainCanvas(pNode);

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

CanvasPtr Player::createMainCanvas(const py::dict& params)
{
    errorIfPlaying("Player.createMainCanvas");
    if (m_pMainCanvas) {
        cleanup(false);
    }

    NodePtr pNode = createNode("avg", params);
    initMainCanvas(pNode);

    return m_pMainCanvas;
}

OffscreenCanvasPtr Player::createCanvas(const py::dict& params)
{
    NodePtr pNode = createNode("canvas", params);
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
            (*it)->stopPlayback(false);
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

void Player::newCanvasDependency()
{
    DAG dag;
    for (unsigned i = 0; i < m_pCanvases.size(); ++i) {
        set<long> dependentCanvasSet;
        OffscreenCanvasPtr pCanvas = m_pCanvases[i];
        const vector<CanvasPtr>& pDependents = pCanvas->getDependentCanvases();
        for (unsigned j = 0; j < pDependents.size(); ++j) {
            dependentCanvasSet.insert(pDependents[j]->getHash());
        }
        dag.addNode(pCanvas->getHash(), dependentCanvasSet);
    }
    dag.addNode(m_pMainCanvas->getHash(), set<long>());

    vector<long> sortedCanvasIDs;
    try {
        dag.sort(sortedCanvasIDs);
    } catch (Exception&) {
        throw Exception(AVG_ERR_INVALID_ARGS, "Circular dependency between canvases.");
    }

    vector<OffscreenCanvasPtr> pTempCanvases = m_pCanvases;
    m_pCanvases.clear();
    for (unsigned i = 0; i < sortedCanvasIDs.size(); ++i) {
        long canvasID = sortedCanvasIDs[i];
        for (unsigned j = 0; j < pTempCanvases.size(); ++j) {
            OffscreenCanvasPtr pCandidateCanvas = pTempCanvases[j];
            if (pCandidateCanvas->getHash() == canvasID) {
                m_pCanvases.push_back(pCandidateCanvas);
                break;
            }
        }
    }
}

NodePtr Player::loadMainNodeFromFile(const string& sFilename)
{
    string sRealFilename;
    AVG_TRACE(Logger::category::MEMORY, Logger::severity::INFO,
           "Player::loadFile(" << sFilename << ")");

    // When loading an avg file, assets are loaded from a directory relative
    // to the file.
    char szBuf[1024];
    char * pBuf = getcwd(szBuf, 1024);
    if (sFilename[0] == '/') {
        sRealFilename = sFilename;
    } else {
        m_CurDirName = string(pBuf)+"/";
        sRealFilename = m_CurDirName+sFilename;
    }
    m_CurDirName = sRealFilename.substr(0, sRealFilename.rfind('/')+1);

    string sAVG;
    readWholeFile(sRealFilename, sAVG);
    NodePtr pNode = internalLoad(sAVG, sRealFilename);

    // Reset the directory to load assets from to the current dir.
    m_CurDirName = string(pBuf)+"/";
    return pNode;
}

NodePtr Player::loadMainNodeFromString(const string& sAVG)
{
    AVG_TRACE(Logger::category::MEMORY, Logger::severity::INFO, "Player::loadString()");

    string sEffectiveDoc = removeStartEndSpaces(sAVG);
    NodePtr pNode = internalLoad(sEffectiveDoc, "");
    return pNode;
}

void Player::play()
{
    try {
        if (!m_pMainCanvas) {
            throw Exception(AVG_ERR_NO_NODE, "Play called, but no xml file loaded.");
        }
        initPlayback();
        notifySubscribers("PLAYBACK_START");
        try {
            ThreadProfiler::get()->start();
            doFrame(true);
            while (!m_bStopping) {
                doFrame(false);
            }
            notifySubscribers("PLAYBACK_END");
        } catch (...) {
            cleanup(true);
            m_bDisplayEngineBroken = true;
            throw;
        }
        cleanup(false);
        AVG_TRACE(Logger::category::PLAYER, Logger::severity::INFO, "Playback ended.");
    } catch (Exception& ex) {
        m_bIsPlaying = false;
        throw;
    }
}

void Player::stop()
{
    if (m_bIsPlaying) {
        m_bStopping = true;
    } else {
        cleanup(false);
    }
}

bool Player::isStopping()
{
    return m_bStopping;
}

void Player::initPlayback(const std::string& sShaderPath)
{
    m_bIsPlaying = true;
    AVG_TRACE(Logger::category::PLAYER, Logger::severity::INFO, "Playback started.");
    initGraphics(sShaderPath);
    initAudio();
    try {
        for (unsigned i = 0; i < m_pCanvases.size(); ++i) {
            m_pCanvases[i]->initPlayback();
        }
        m_pMainCanvas->initPlayback(m_pDisplayEngine);
    } catch (Exception&) {
        cleanup(true);
        m_bDisplayEngineBroken = true;
        throw;
    }
    m_pEventDispatcher->addInputDevice(
            boost::dynamic_pointer_cast<InputDevice>(m_pDisplayEngine));
    m_pEventDispatcher->addInputDevice(m_pTestHelper);

    m_pDisplayEngine->initRender();
    Display::get()->rereadScreenResolution();
    m_bStopping = false;
    if (m_pMultitouchInputDevice) {
        m_pMultitouchInputDevice->start();
    }

    m_FrameTime = 0;
    m_NumFrames = 0;
}

bool Player::isPlaying()
{
    return m_bIsPlaying;
}

void Player::setFramerate(float rate)
{
    if (m_bIsPlaying) {
        m_pDisplayEngine->setFramerate(rate);
    }
    m_DP.setFramerate(rate, 0);
}

void Player::setVBlankFramerate(int rate)
{
    if (m_bIsPlaying) {
        m_pDisplayEngine->setVBlankRate(rate);
    }
    m_DP.setFramerate(0, rate);
}

float Player::getEffectiveFramerate()
{
    if (m_bIsPlaying) {
        if (m_bFakeFPS) {
            return m_FakeFPS;
        } else {
            return m_pDisplayEngine->getEffectiveFramerate();
        }
    } else {
        return 0;
    }
}

TestHelper * Player::getTestHelper()
{
    return m_pTestHelper.get();
}

void Player::setFakeFPS(float fps)
{
    if (fabs(fps + 1.0) < 0.0001) {
        // fps = -1
        m_bFakeFPS = false;
    } else {
        m_bFakeFPS = true;
        m_FakeFPS = fps;
    }

    if (AudioEngine::get()) {
        AudioEngine::get()->setAudioEnabled(!m_bFakeFPS);
    }
}

void Player::addInputDevice(InputDevicePtr pSource)
{
    if (!m_pEventDispatcher) {
        throw Exception(AVG_ERR_UNSUPPORTED,
                "You must use loadFile() before addInputDevice().");
    }
    m_pEventDispatcher->addInputDevice(pSource);
}

long long Player::getFrameTime()
{
    return m_FrameTime;
}

float Player::getFrameDuration()
{
    if (!m_bIsPlaying) {
        throw Exception(AVG_ERR_UNSUPPORTED,
                "Must call Player.play() before getFrameDuration().");
    }
    if (m_bFakeFPS) {
        return 1000.0f/m_FakeFPS;
    } else {
        float framerate = m_pDisplayEngine->getEffectiveFramerate();
        if (framerate > 0) {
            return 1000.f/framerate;
        } else {
            return 0;
        }
    }
}

TrackerInputDevice * Player::getTracker()
{
    TrackerInputDevice* pTracker = dynamic_cast<TrackerInputDevice*>(
            m_pMultitouchInputDevice.get());
    return pTracker;
}

void Player::enableMultitouch()
{
    if (!m_bIsPlaying) {
        throw Exception(AVG_ERR_UNSUPPORTED,
                "Must call Player.play() before enableMultitouch().");
    }

    string sDriver;
    getEnv("AVG_MULTITOUCH_DRIVER", sDriver);
    if (sDriver == "") {
#if defined(_WIN32) && defined(SM_DIGITIZER)
        sDriver = "WIN7TOUCH";
#elif defined(HAVE_XI2_1) || defined(HAVE_XI2_2) 
        sDriver = "XINPUT";
#else
        AVG_LOG_WARNING("Valid values for AVG_MULTITOUCH_DRIVER are WIN7TOUCH, XINPUT, TRACKER, TUIO and APPLETRACKPAD.");
        throw Exception(AVG_ERR_MT_INIT,
                "Multitouch support: No default driver available. Set AVG_MULTITOUCH_DRIVER.");
#endif
    }
    if (sDriver == "TUIO") {
        m_pMultitouchInputDevice = InputDevicePtr(new TUIOInputDevice);
#if defined(_WIN32) && defined(SM_DIGITIZER)
    } else if (sDriver == "WIN7TOUCH") {
        m_pMultitouchInputDevice = InputDevicePtr(new Win7TouchInputDevice);
#endif
    } else if (sDriver == "XINPUT" || sDriver == "XINPUT21") {
#if defined(HAVE_XI2_1) || defined(HAVE_XI2_2) 
        m_pMultitouchInputDevice =  InputDevicePtr(new XInputMTInputDevice);
#else
        throw Exception(AVG_ERR_MT_INIT,
                "XInput multitouch event source: Support not configured.'");
#endif
    } else if (sDriver == "TRACKER") {
        m_pMultitouchInputDevice = InputDevicePtr(new TrackerInputDevice);
    } else {
        AVG_LOG_WARNING("Valid values for AVG_MULTITOUCH_DRIVER are WIN7TOUCH, XINPUT, TRACKER and TUIO.");
        throw Exception(AVG_ERR_UNSUPPORTED, string("Unsupported multitouch driver '")+
                sDriver +"'.");
    }
    if (m_bIsPlaying) {
        try {
            m_pMultitouchInputDevice->start();
        } catch (Exception&) {
            m_pMultitouchInputDevice = InputDevicePtr();
            throw;
        }
    }
    addInputDevice(m_pMultitouchInputDevice);
}

void Player::enableMouse(bool enabled)
{
    m_bMouseEnabled = enabled;
    
    if (m_pEventDispatcher) {
        m_pEventDispatcher->enableMouse(enabled);
    }
}

bool Player::isMultitouchAvailable() const
{
    if (m_bIsPlaying) {
        return m_pMultitouchInputDevice != 0;
    } else {
        throw Exception(AVG_ERR_UNSUPPORTED,
                "Must call Player.play() before isMultitouchAvailable().");
    }
}

void Player::setEventCapture(NodePtr pNode, int cursorID=MOUSECURSORID)
{
    std::map<int, EventCaptureInfoPtr>::iterator it =
            m_EventCaptureInfoMap.find(cursorID);
    if (it != m_EventCaptureInfoMap.end()) {
        EventCaptureInfoPtr pCaptureInfo = it->second;
        NodePtr pOldNode = pCaptureInfo->m_pNode;
        if (pOldNode->getState() != Node::NS_UNCONNECTED) {
            if (pOldNode == pNode) {
                pCaptureInfo->m_CaptureCount++;
            } else {
                throw Exception(AVG_ERR_INVALID_CAPTURE, "setEventCapture called for '"
                        + pNode->getID() + "', but cursor already captured by '"
                        + pOldNode->getID() + "'.");
            }
        }
    } else {
        m_EventCaptureInfoMap[cursorID] = EventCaptureInfoPtr(
                new EventCaptureInfo(pNode));
    }
}

void Player::releaseEventCapture(int cursorID)
{
    std::map<int, EventCaptureInfoPtr>::iterator it =
            m_EventCaptureInfoMap.find(cursorID);
    if (it == m_EventCaptureInfoMap.end() || 
            (it->second->m_pNode->getState() == Node::NS_UNCONNECTED))
    {
        throw Exception(AVG_ERR_INVALID_CAPTURE,
                "releaseEventCapture called, but cursor not captured.");
    } else {
        it->second->m_CaptureCount--;
        if (it->second->m_CaptureCount == 0) {
            m_EventCaptureInfoMap.erase(cursorID);
        }
    }
}

bool Player::isCaptured(int cursorID)
{
    std::map<int, EventCaptureInfoPtr>::iterator it =
            m_EventCaptureInfoMap.find(cursorID);
    return (it != m_EventCaptureInfoMap.end());
}

void Player::removeDeadEventCaptures()
{
    std::map<int, EventCaptureInfoPtr>::iterator it;
    for (it = m_EventCaptureInfoMap.begin(); it != m_EventCaptureInfoMap.end();) {
        std::map<int, EventCaptureInfoPtr>::iterator lastIt = it;
        it++;
        if (lastIt->second->m_pNode->getState() == Node::NS_UNCONNECTED) {
            m_EventCaptureInfoMap.erase(lastIt);
        }
    }
}

int Player::setInterval(int time, PyObject * pyfunc)
{
    return internalSetTimeout(time, pyfunc, true);
}

int Player::setTimeout(int time, PyObject * pyfunc)
{
    return internalSetTimeout(time, pyfunc, false);
}

int Player::setOnFrameHandler(PyObject * pyfunc)
{
    avgDeprecationWarning("1.8", "Player.setOnFrameHandler", 
            "Player.subscribe(Player.ON_FRAME)");
    return internalSetTimeout(0, pyfunc, true);
}

bool Player::clearInterval(int id)
{
    vector<Timeout*>::iterator it;
    for (it = m_PendingTimeouts.begin(); it != m_PendingTimeouts.end(); it++) {
        if (id == (*it)->getID()) {
            if (it == m_PendingTimeouts.begin() && m_bInHandleTimers) {
                m_bCurrentTimeoutDeleted = true;
            }
            delete *it;
            m_PendingTimeouts.erase(it);
            return true;
        }
    }
    for (it = m_NewTimeouts.begin(); it != m_NewTimeouts.end(); it++) {
        if (id == (*it)->getID()) {
            delete *it;
            m_NewTimeouts.erase(it);
            return true;
        }
    }
    return false;
}

void Player::callFromThread(PyObject * pyfunc)
{
    lock_guard lock(m_AsyncCallMutex);
    Timeout* pTimeout = new Timeout(0, pyfunc, false, getFrameTime());
    m_AsyncCalls.push_back(pTimeout);
}

MouseEventPtr Player::getMouseState() const
{
    return m_pLastMouseEvent;
}

EventPtr Player::getCurrentEvent() const
{
    if (!m_pCurrentEvent) {
        throw Exception(AVG_ERR_UNSUPPORTED, "No current event.");
    }
    return m_pCurrentEvent;
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
    if (!m_bIsPlaying) {
        throw Exception(AVG_ERR_UNSUPPORTED,
                "Must call Player.play() before screenshot().");
    }
    if (GLContext::getCurrent()->isGLES()) {
        // Some GLES implementations invalidate the buffer after eglSwapBuffers.
        // The only way we can get at the contents at this point is to rerender them.
        WindowPtr pWindow = m_pDisplayEngine->getSDLWindow();
        IntRect viewport = pWindow->getViewport();
        m_pMainCanvas->renderWindow(pWindow, MCFBOPtr(), viewport);
        GLContextManager::get()->reset();
    }
    return m_pDisplayEngine->screenshot();
}

void Player::showCursor(bool bShow)
{
    if (m_pDisplayEngine) {
        m_pDisplayEngine->showCursor(bShow);
    }
    m_DP.setShowCursor(bShow);
}

bool Player::isCursorShown()
{
    return m_DP.isCursorVisible();
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
    delete[] pData;
    delete[] pMask;
}

NodePtr Player::getElementByID(const std::string& sID)
{
    if (m_pMainCanvas) {
        return m_pMainCanvas->getElementByID(sID);
    } else {
        return NodePtr();
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

void Player::disablePython()
{
    m_bPythonAvailable = false;
}

void Player::startTraversingTree()
{
    AVG_ASSERT(!m_bIsTraversingTree);
    m_bIsTraversingTree = true;
}

void Player::endTraversingTree()
{
    AVG_ASSERT(m_bIsTraversingTree);
    m_bIsTraversingTree = false;
}

bool Player::isTraversingTree() const
{
    return m_bIsTraversingTree;
}

void Player::registerFrameEndListener(IFrameEndListener* pListener)
{
    AVG_ASSERT(m_pMainCanvas);
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
    AVG_ASSERT(m_pMainCanvas);
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
    AVG_ASSERT(m_pMainCanvas);
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
    EventPtr pLastEvent = m_pCurrentEvent;
    m_pCurrentEvent = pEvent;
    if (MouseEventPtr pMouseEvent = boost::dynamic_pointer_cast<MouseEvent>(pEvent)) {
        m_pLastMouseEvent = pMouseEvent;
    }

    if (CursorEventPtr pCursorEvent = boost::dynamic_pointer_cast<CursorEvent>(pEvent)) {
        if (pEvent->getType() == Event::CURSOR_OUT ||
                pEvent->getType() == Event::CURSOR_OVER)
        {
            pEvent->trace();
            pCursorEvent->getNode()->handleEvent(pEvent);
        } else {
            handleCursorEvent(pCursorEvent);
        }
    }
    else if (KeyEventPtr pKeyEvent = boost::dynamic_pointer_cast<KeyEvent>(pEvent))
    {
        pEvent->trace();
        switch (pEvent->getType()) {
            case Event::KEY_DOWN:
                notifySubscribers("KEY_DOWN", pEvent);
                break;
            case Event::KEY_UP:
                notifySubscribers("KEY_UP", pEvent);
                break;
            default:
                AVG_ASSERT(false);
        }
        getRootNode()->handleEvent(pKeyEvent);
        if (getStopOnEscape() && pEvent->getType() == Event::KEY_DOWN
                && pKeyEvent->getKeyCode() == avg::key::KEY_ESCAPE)
        {
            stop();
        }
    }
    else {
        if (pEvent->getType() != Event::QUIT) {
            pEvent->trace();
            getRootNode()->handleEvent(pEvent);
        }
        else {
            stop();
        }
    }
    m_pCurrentEvent = pLastEvent;
    return true;
}

static ProfilingZoneID MainProfilingZone("Player - Total frame time");
static ProfilingZoneID TimersProfilingZone("Player - handleTimers");
static ProfilingZoneID EventsProfilingZone("Dispatch events");
static ProfilingZoneID MainCanvasProfilingZone("Main canvas rendering");
static ProfilingZoneID OffscreenProfilingZone("Offscreen rendering");

void Player::doFrame(bool bFirstFrame)
{
    {
        ScopeTimer Timer(MainProfilingZone);
        if (!bFirstFrame) {
            m_NumFrames++;
            if (m_bFakeFPS) {
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
                removeDeadEventCaptures();
            }
        }
        for (unsigned i = 0; i < m_pCanvases.size(); ++i) {
            ScopeTimer Timer(OffscreenProfilingZone);
            dispatchOffscreenRendering(m_pCanvases[i].get());
        }
        {
            ScopeTimer Timer(MainCanvasProfilingZone);
            m_pMainCanvas->doFrame(m_bPythonAvailable);
        }
        GLContext::mandatoryCheckError("End of frame");
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
    ThreadProfiler::get()->reset();
    if (m_NumFrames == 5) {
        ThreadProfiler::get()->restart();
    }
}

void Player::endFrame()
{
    m_pDisplayEngine->frameWait();
    m_pDisplayEngine->swapBuffers();
    m_pDisplayEngine->checkJitter();
}

float Player::getFramerate()
{
    if (!m_pDisplayEngine) {
        return m_DP.getFramerate();
    } else {
        return m_pDisplayEngine->getFramerate();
    }
}

float Player::getVideoRefreshRate()
{
    return Display::get()->getRefreshRate();
}

size_t Player::getVideoMemInstalled()
{
    if (!m_pDisplayEngine) {
        throw Exception(AVG_ERR_UNSUPPORTED,
                "Player.getVideoMemInstalled must be called after Player.play().");
    }
    return GLContext::getCurrent()->getVideoMemInstalled();
}

size_t Player::getVideoMemUsed()
{
    if (!m_pDisplayEngine) {
        throw Exception(AVG_ERR_UNSUPPORTED,
                "Player.getVideoMemUsed must be called after Player.play().");
    }
    return GLContext::getCurrent()->getVideoMemUsed();
}

void Player::setGamma(float red, float green, float blue)
{
    if (m_pDisplayEngine) {
        m_pDisplayEngine->setGamma(red, green, blue);
    }
    m_DP.setGamma(red, green, blue);
}

void Player::initConfig()
{
    errorIfMultiDisplay("Player.setWindowPos");
    // Get data from config files.
    ConfigMgr* pMgr = ConfigMgr::get();

    int bpp = atoi(pMgr->getOption("scr", "bpp")->c_str());
    if (bpp != 15 && bpp != 16 && bpp != 24 && bpp != 32) {
        AVG_LOG_ERROR("BPP must be 15, 16, 24 or 32. Current value is " << bpp <<
                ". Aborting." );
        exit(-1);
    }
    m_DP.setBPP(bpp);
    m_DP.setFullscreen(pMgr->getBoolOption("scr", "fullscreen", false));

    WindowParams& wp = m_DP.getWindowParams(0);
    wp.m_Size.x = atoi(pMgr->getOption("scr", "windowwidth")->c_str());
    wp.m_Size.y = atoi(pMgr->getOption("scr", "windowheight")->c_str());

    if (m_DP.isFullscreen() && (wp.m_Size != IntPoint(0, 0))) {
        AVG_LOG_ERROR("Can't set fullscreen and window size at once. Aborting.");
        exit(-1);
    }
    if (wp.m_Size.x != 0 && wp.m_Size.y != 0) {
        AVG_LOG_ERROR("Can't set window width and height at once");
        AVG_LOG_ERROR("(aspect ratio is determined by avg file). Aborting.");
        exit(-1);
    }

    m_AP.m_Channels = atoi(pMgr->getOption("aud", "channels")->c_str());
    m_AP.m_SampleRate = atoi(pMgr->getOption("aud", "samplerate")->c_str());
    m_AP.m_OutputBufferSamples =
            atoi(pMgr->getOption("aud", "outputbuffersamples")->c_str());

    m_GLConfig.m_bGLES = pMgr->getBoolOption("scr", "gles", false);
    m_GLConfig.m_bUsePOTTextures = pMgr->getBoolOption("scr", "usepow2textures", false);

    m_GLConfig.m_bUsePixelBuffers = pMgr->getBoolOption("scr", "usepixelbuffers", true);
    int multiSampleSamples = pMgr->getIntOption("scr", "multisamplesamples", 8);
    if (multiSampleSamples < 1) {
        AVG_LOG_ERROR("multisamplesamples must be >= 1. Aborting")
        exit(-1);
    }
    m_GLConfig.m_MultiSampleSamples = multiSampleSamples;

    string sShaderUsage;
    pMgr->getStringOption("scr", "shaderusage", "auto", sShaderUsage);
    if (sShaderUsage == "full") {
        m_GLConfig.m_ShaderUsage = GLConfig::FULL;
    } else if (sShaderUsage == "minimal") {
        m_GLConfig.m_ShaderUsage = GLConfig::MINIMAL;
    } else if (sShaderUsage == "auto") {
        m_GLConfig.m_ShaderUsage = GLConfig::AUTO;
    } else {
        throw Exception(AVG_ERR_OUT_OF_RANGE,
               "avgrc parameter shaderusage must be full, minimal or auto");
    }
    string sDummy;
    m_GLConfig.m_bUseDebugContext = getEnv("AVG_USE_DEBUG_GL_CONTEXT", sDummy);
#ifdef AVG_ENABLE_EGL
    m_GLConfig.m_bGLES = true;
#endif
    BitmapLoader::init(!m_GLConfig.m_bGLES);

    float gamma[3];
    pMgr->getGammaOption("scr", "gamma", gamma);
    m_DP.setGamma(gamma[0], gamma[1], gamma[2]);
}

void Player::initGraphics(const string& sShaderPath)
{
    if (!Display::isInitialized()) {
        ConfigMgr* pMgr = ConfigMgr::get();
        float dotsPerMM = float(atof(pMgr->getOption("scr", "dotspermm")->c_str()));
        Display::get()->assumePixelsPerMM(dotsPerMM);
    }
    // Init display configuration.
    AVG_TRACE(Logger::category::CONFIG, Logger::severity::INFO,
            "Display bpp: " << m_DP.getBPP());

    if (m_bDisplayEngineBroken) {
        m_bDisplayEngineBroken = false;
        m_pDisplayEngine->teardown();
        m_pDisplayEngine = DisplayEnginePtr();
    }

    if (!m_pDisplayEngine) {
        m_pDisplayEngine = DisplayEnginePtr(new DisplayEngine());
    }
    AVG_TRACE(Logger::category::CONFIG, Logger::severity::INFO,
            "Requested OpenGL configuration: ");
    m_GLConfig.log();
    m_DP.calcWindowSizes();
    if (m_DP.getNumWindows() > 1 ||
            m_pDisplayEngine->getWindowSize() != m_DP.getWindowParams(0).m_Size ||
            m_pDisplayEngine->isFullscreen() != m_DP.isFullscreen()) 
    {
        m_pDisplayEngine->teardown();
        m_pDisplayEngine->init(m_DP, m_GLConfig);
    }
    AVG_TRACE(Logger::category::CONFIG, Logger::severity::INFO,
            "Pixels per mm: " << Display::get()->getPixelsPerMM());
    if (sShaderPath != "") {
        ShaderRegistry::get()->setShaderPath(sShaderPath);
    }
    m_pDisplayEngine->setGamma(1.0, 1.0, 1.0);
    m_GLConfig = GLContext::getCurrent()->getConfig();
}

void Player::initAudio()
{
    AudioEngine* pAudioEngine = AudioEngine::get();
    if (!pAudioEngine) {
        pAudioEngine = new AudioEngine();
    }
    pAudioEngine->init(m_AP, m_Volume);
    pAudioEngine->setAudioEnabled(!m_bFakeFPS);
    pAudioEngine->play();
}

void Player::initMainCanvas(NodePtr pRootNode)
{
    m_pEventDispatcher = EventDispatcherPtr(new EventDispatcher(this, m_bMouseEnabled));
    m_pMainCanvas = MainCanvasPtr(new MainCanvas(this));
    m_pMainCanvas->setRoot(pRootNode);
    if (m_DP.getNumWindows() == 1) {
        m_DP.getWindowParams(0).m_Viewport = 
                IntRect(IntPoint(0,0), m_pMainCanvas->getSize());
    }
    registerFrameEndListener(BitmapManager::get());
}

NodePtr Player::internalLoad(const string& sAVG, const string& sFilename)
{
    XMLParser parser;
    parser.setDTD(TypeRegistry::get()->getDTD(), "avg.dtd");
    parser.parse(sAVG, sFilename);
    xmlNodePtr xmlNode = parser.getRootNode();
    NodePtr pNode = createNodeFromXml(parser.getDoc(), xmlNode);
    if (!pNode) {
        throw (Exception(AVG_ERR_XML_PARSE,
                "Root node of an avg tree needs to be an <avg> node."));
    }
    return pNode;
}

DisplayEnginePtr Player::safeGetDisplayEngine()
{
    if (!m_pDisplayEngine) {
        m_pDisplayEngine = DisplayEnginePtr(new DisplayEngine());
    }
    return m_pDisplayEngine;

}

NodePtr Player::createNode(const string& sType,
        const py::dict& params, const boost::python::object& self)
{
    DivNodePtr pParentNode;
    py::dict attrs = params;
    py::object parent;
    if (params.has_key("parent")) {
        parent = params["parent"];
        attrs.attr("__delitem__")("parent");
        pParentNode = py::extract<DivNodePtr>(parent);
    }
    NodePtr pNode = dynamic_pointer_cast<Node>(
            TypeRegistry::get()->createObject(sType, attrs));

    // See if the class names of self and pNode match. If they don't, there is a
    // python derived class that's being constructed and we can't set parent here.
    string sSelfClassName = py::extract<string>(
            self.attr("__class__").attr("__name__"));
    py::object pythonClassName = 
            (py::object(pNode).attr("__class__").attr("__name__"));
    string sThisClassName = py::extract<string>(pythonClassName);
    bool bHasDerivedClass = sSelfClassName != sThisClassName && 
            sSelfClassName != "NoneType";
    if (bHasDerivedClass) {
        if (pParentNode) {
            throw Exception(AVG_ERR_UNSUPPORTED,
                    "Can't pass 'parent' parameter to C++ class constructor if there is a derived python class. Use Node.registerInstance() instead.");
        }
        pNode->registerInstance(self.ptr(), pParentNode);
    } else {
        pNode->registerInstance(0, pParentNode);
    } 
    if (parent) {
        attrs["parent"] = parent;
    }
    return pNode;
}

NodePtr Player::createNodeFromXmlString(const string& sXML)
{
    xmlPedanticParserDefault(1);
    xmlDoValidityCheckingDefaultValue =0;

    XMLParser parser;
    parser.setDTD(TypeRegistry::get()->getDTD(), "avg.dtd");
    parser.parse(sXML, "");

//        cvp->error = xmlParserValidityError;
//        cvp->warning = xmlParserValidityWarning;
    xmlNodePtr xmlNode = parser.getRootNode();
    NodePtr pNode = createNodeFromXml(parser.getDoc(), xmlNode);

    return pNode;
}

NodePtr Player::createNodeFromXml(const xmlDocPtr xmlDoc,
        const xmlNodePtr xmlNode)
{
    const char * nodeType = (const char *)xmlNode->name;

    if (!strcmp (nodeType, "text") || !strcmp (nodeType, "comment")) {
        // Ignore whitespace & comments
        return NodePtr();
    }
    NodePtr pCurNode = dynamic_pointer_cast<Node>(
            TypeRegistry::get()->createObject(nodeType, xmlNode));
    if (!strcmp(nodeType, "words")) {
        // TODO: This is an end-run around the generic serialization mechanism
        // that will probably break at some point.
        string s = getXmlChildrenAsString(xmlDoc, xmlNode);
        boost::dynamic_pointer_cast<WordsNode>(pCurNode)->setTextFromNodeValue(s);
    } else {
        // If this is a container, recurse into children
        if (pCurNode->getDefinition()->hasChildren()) {
            xmlNodePtr curXmlChild = xmlNode->xmlChildrenNode;
            while (curXmlChild) {
                NodePtr curChild = createNodeFromXml(xmlDoc, curXmlChild);
                if (curChild) {
                    DivNodePtr pDivNode = boost::dynamic_pointer_cast<DivNode>(pCurNode);
                    pDivNode->appendChild(curChild);
                }
                curXmlChild = curXmlChild->next;
            }
        }
    }
    return pCurNode;
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
            pCanvas->initPlayback();
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
        CursorStatePtr pState = it->second;
        handleCursorEvent(pState->getLastEvent(), true);
    }
}

void Player::sendOver(const CursorEventPtr pOtherEvent, Event::Type type,
        NodePtr pNode)
{
    if (pNode) {
        CursorEventPtr pNewEvent = pOtherEvent->cloneAs(type);
        pNewEvent->setNode(pNode);
        m_pEventDispatcher->sendEvent(pNewEvent);
    }
}

void Player::handleCursorEvent(CursorEventPtr pEvent, bool bOnlyCheckCursorOver)
{
    // Find all nodes under the cursor.
    vector<NodePtr> pCursorNodes;
    DivNodePtr pEventReceiverNode = pEvent->getInputDevice()->getEventReceiverNode();
    if (!pEventReceiverNode) {
        pEventReceiverNode = getRootNode();
    }
    pEventReceiverNode->getElementsByPos(pEvent->getPos(), pCursorNodes);
    ContactPtr pContact = pEvent->getContact();
    if (pContact && !bOnlyCheckCursorOver) {
        if (!pCursorNodes.empty()) {
            NodePtr pNode = *(pCursorNodes.begin());
            pEvent->setNode(pNode);
        }
        pContact->sendEventToListeners(pEvent);
    }
        
    int cursorID = pEvent->getCursorID();

    // Determine the nodes the event should be sent to.
    vector<NodePtr> pDestNodes = pCursorNodes;
    if (m_EventCaptureInfoMap.find(cursorID) != m_EventCaptureInfoMap.end()) {
        NodeWeakPtr pEventCaptureNode = 
                m_EventCaptureInfoMap[cursorID]->m_pNode;
        if (pEventCaptureNode.expired()) {
            m_EventCaptureInfoMap.erase(cursorID);
        } else {
            pDestNodes = pEventCaptureNode.lock()->getParentChain();
        }
    }

    vector<NodePtr> pLastCursorNodes;
    {
        map<int, CursorStatePtr>::iterator it;
        it = m_pLastCursorStates.find(cursorID);
        if (it != m_pLastCursorStates.end()) {
            pLastCursorNodes = it->second->getNodes();
        }
    }

    // Send out events.
    vector<NodePtr>::const_iterator itLast;
    vector<NodePtr>::iterator itCur;
    for (itLast = pLastCursorNodes.begin(); itLast != pLastCursorNodes.end(); 
            ++itLast)
    {
        NodePtr pLastNode = *itLast;
        for (itCur = pCursorNodes.begin(); itCur != pCursorNodes.end(); ++itCur) {
            if (*itCur == pLastNode) {
                break;
            }
        }
        if (itCur == pCursorNodes.end()) {
            sendOver(pEvent, Event::CURSOR_OUT, pLastNode);
        }
    }

    // Send over events.
    for (itCur = pCursorNodes.begin(); itCur != pCursorNodes.end(); ++itCur) {
        NodePtr pCurNode = *itCur;
        for (itLast = pLastCursorNodes.begin(); itLast != pLastCursorNodes.end();
                ++itLast)
        {
            if (*itLast == pCurNode) {
                break;
            }
        }
        if (itLast == pLastCursorNodes.end()) {
            sendOver(pEvent, Event::CURSOR_OVER, pCurNode);
        }
    }

    if (!bOnlyCheckCursorOver) {
        // Iterate through the nodes and send the event to all of them.
        vector<NodePtr>::iterator it;
        for (it = pDestNodes.begin(); it != pDestNodes.end(); ++it) {
            NodePtr pNode = *it;
            if (pNode->getState() != Node::NS_UNCONNECTED) {
                pEvent->setNode(pNode);
                if (pEvent->getType() != Event::CURSOR_MOTION) {
                    pEvent->trace();
                }
                if (pNode->handleEvent(pEvent) == true) {
                    // stop bubbling
                    break;
                }
            }
        }
    }

    if (pEvent->getType() == Event::CURSOR_UP && pEvent->getSource() != Event::MOUSE) {
        // Cursor has disappeared: send out events.
        vector<NodePtr>::iterator it;
        for (it = pCursorNodes.begin(); it != pCursorNodes.end(); ++it) {
            NodePtr pNode = *it;
            sendOver(pEvent, Event::CURSOR_OUT, pNode);
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

void Player::errorIfPlaying(const string& sFunc) const
{
    if (m_bIsPlaying) {
        throw Exception(AVG_ERR_UNSUPPORTED,
                sFunc + " must be called before Player.play().");
    }
}

void Player::errorIfMultiDisplay(const string& sFunc) const
{
    if (m_DP.getNumWindows() != 1) {
        throw Exception(AVG_ERR_INVALID_ARGS,
                sFunc + " only supported in single-window mode.");
    }
}



void Player::handleTimers()
{
    vector<Timeout *>::iterator it;
    m_bInHandleTimers = true;

    it = m_PendingTimeouts.begin();
    while (it != m_PendingTimeouts.end() && (*it)->isReady(getFrameTime())
            && !m_bStopping)
    {
        m_bCurrentTimeoutDeleted = false;
        (*it)->fire(getFrameTime());
        if (m_bCurrentTimeoutDeleted) {
            it = m_PendingTimeouts.begin();
        } else {
            if ((*it)->isInterval()) {
                Timeout* pTempTimeout = *it;
                it = m_PendingTimeouts.erase(it);
                m_NewTimeouts.insert(m_NewTimeouts.begin(), pTempTimeout);
            } else {
                delete *it;
                it = m_PendingTimeouts.erase(it);
            }
        }
    }
    for (it = m_NewTimeouts.begin(); it != m_NewTimeouts.end(); ++it) {
        addTimeout(*it);
    }
    m_NewTimeouts.clear();
    
    notifySubscribers("ON_FRAME");
    
    m_bInHandleTimers = false;

    if (m_bPythonAvailable) {
        std::vector<Timeout *> tempAsyncCalls;
        Py_BEGIN_ALLOW_THREADS;
        {
            lock_guard lock(m_AsyncCallMutex);
            tempAsyncCalls = m_AsyncCalls;
            m_AsyncCalls.clear();
        }
        Py_END_ALLOW_THREADS;
        for (it = tempAsyncCalls.begin(); it != tempAsyncCalls.end(); ++it) {
            (*it)->fire(getFrameTime());
            delete *it;
        }
    }
}

DisplayEngine * Player::getDisplayEngine() const
{
    return m_pDisplayEngine.get();
}

void Player::keepWindowOpen()
{
    m_bKeepWindowOpen = true;
}

void Player::setStopOnEscape(bool bStop)
{
    m_bStopOnEscape = bStop;
}

bool Player::getStopOnEscape() const
{
    return m_bStopOnEscape;
}

void Player::setVolume(float volume)
{
    m_Volume = volume;
    if (AudioEngine::get()) {
        AudioEngine::get()->setVolume(m_Volume);
    }
}

float Player::getVolume() const
{
    return m_Volume;
}

string Player::getConfigOption(const string& sSubsys, const string& sName) const
{
    const string* psValue = ConfigMgr::get()->getOption(sSubsys, sName);
    if (!psValue) {
        throw Exception(AVG_ERR_INVALID_ARGS, string("Unknown config option ") + sSubsys
                + ":" + sName);
    } else {
        return *psValue;
    }
}

bool Player::isUsingGLES() const
{
    return m_GLConfig.m_bGLES;
}

bool Player::areFullShadersSupported() const
{
    if (!m_bIsPlaying) {
        throw Exception(AVG_ERR_UNSUPPORTED,
                "Must call Player.play() before areFullShadersSupported().");
    }
    return (m_GLConfig.m_ShaderUsage == GLConfig::FULL);
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

void Player::cleanup(bool bIsAbort)
{
    // Kill all timeouts.
    vector<Timeout*>::iterator it;
    for (it = m_PendingTimeouts.begin(); it != m_PendingTimeouts.end(); it++) {
        delete *it;
    }
    m_PendingTimeouts.clear();
    m_EventCaptureInfoMap.clear();
    m_pLastCursorStates.clear();
    m_pTestHelper->reset();
    ThreadProfiler::get()->dumpStatistics();
    for (unsigned i = 0; i < m_pCanvases.size(); ++i) {
        m_pCanvases[i]->stopPlayback(bIsAbort);
    }
    m_pCanvases.clear();
    if (m_pMainCanvas) {
        unregisterFrameEndListener(BitmapManager::get());
        delete BitmapManager::get();
        m_pMainCanvas->stopPlayback(bIsAbort);
        m_pMainCanvas = MainCanvasPtr();
    }

    if (m_pMultitouchInputDevice) {
        m_pMultitouchInputDevice = InputDevicePtr();
    }

    if (m_pDisplayEngine) {
        m_DP.getWindowParams(0).m_Size = IntPoint(0, 0);
        if (!m_bKeepWindowOpen) {
            m_pDisplayEngine->deinitRender();
            m_pDisplayEngine->teardown();
            m_pDisplayEngine = DisplayEnginePtr();
        }
    }
    if (AudioEngine::get()) {
        AudioEngine::get()->teardown();
    }
    m_pEventDispatcher = EventDispatcherPtr();
    m_pLastMouseEvent = MouseEventPtr(new MouseEvent(Event::CURSOR_MOTION, false, false, 
            false, IntPoint(-1, -1), MouseEvent::NO_BUTTON, glm::vec2(-1, -1), 0));

    m_FrameTime = 0;
    m_bIsPlaying = false;

    m_CurDirName = getCWD();

    removeSubscribers();
}

int Player::internalSetTimeout(int time, PyObject * pyfunc, bool bIsInterval)
{
    Timeout* pTimeout = new Timeout(time, pyfunc, bIsInterval, getFrameTime());
    if (m_bInHandleTimers) {
        m_NewTimeouts.push_back(pTimeout);
    } else {
        addTimeout(pTimeout);
    }
    return pTimeout->getID();
}


int Player::addTimeout(Timeout* pTimeout)
{
    vector<Timeout*>::iterator it = m_PendingTimeouts.begin();
    while (it != m_PendingTimeouts.end() && (**it)<*pTimeout) {
        it++;
    }
    m_PendingTimeouts.insert(it, pTimeout);
    return pTimeout->getID();
}

void Player::setPluginPath(const string& newPath)
{
    PluginManager::get().setSearchPath(newPath);
}

string Player::getPluginPath() const
{
    return  PluginManager::get().getSearchPath();
}

py::object Player::loadPlugin(const std::string& name)
{
    return PluginManager::get().loadPlugin(name);
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

Player::EventCaptureInfo::EventCaptureInfo(const NodeWeakPtr& pNode)
    : m_pNode(pNode),
      m_CaptureCount(1)
{
}

}
