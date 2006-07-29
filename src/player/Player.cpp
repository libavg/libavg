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
#include "Camera.h"
#include "Image.h"
#include "PanoImage.h"

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

#include <Magick++.h>

#include <libxml/xmlmemory.h>

#include <algorithm>
#include <iostream>
#include <sstream>
#include <assert.h>

using namespace std;

namespace avg {

Player::Player()
    : m_pRootNode (0),
      m_pDisplayEngine(0),
      m_TestHelper(this),
      m_bInHandleTimers(false),
      m_pLastMouseNode(0),
      m_bShowCursor(true),
      m_bIsPlaying(false)
{
    initConfig();
    registerDTDEntityLoader(g_pAVGDTD);
}

Player::~Player()
{
    if (m_pDisplayEngine) {
        delete m_pDisplayEngine;
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
    if (m_pRootNode) {
        AVG_TRACE(Logger::ERROR,
                "Player::setResolution called after loadFile."
                << " Aborting.");
        exit(-1);
    }
    m_bFullscreen = bFullscreen;
    if (bpp) {
        m_BPP = bpp;
    }
    if (width) {
        m_WindowWidth = width;
    }
    if (height) {
        m_WindowHeight = height;
    }
}
        
void Player::setOGLOptions(bool bUsePOW2Textures, DisplayEngine::YCbCrMode DesiredYCbCrMode, 
                bool bUseRGBOrder, bool bUsePixelBuffers, int MultiSampleSamples)
{
    if (m_pRootNode) {
        AVG_TRACE(Logger::ERROR,
                "Player::setOGLOptions called before loadFile."
                << " Aborting.");
        exit(-1);
    }
    m_bUsePOW2Textures = bUsePOW2Textures;
    m_YCbCrMode = DesiredYCbCrMode;
    m_bUseRGBOrder = bUseRGBOrder;
    m_bUsePixelBuffers = bUsePixelBuffers;
    m_MultiSampleSamples = MultiSampleSamples;
}

void Player::loadFile (const std::string& filename)
{
    try {
        AVG_TRACE(Logger::PROFILE, 
                std::string("Player::LoadFile(") + filename + ")");
        assert (!m_pRootNode);

        // Init display configuration.
        AVG_TRACE(Logger::CONFIG, "Display subsystem: " << 
                m_sDisplaySubsystem);
        AVG_TRACE(Logger::CONFIG, "Display bpp: " << m_BPP);
        AVG_TRACE(Logger::CONFIG, "Display fullscreen: "
                << (m_bFullscreen?"true":"false"));

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
                    case DisplayEngine::NONE:
                        AVG_TRACE(Logger::CONFIG, "  No YCbCr texture support.");
                        break;
                    case DisplayEngine::OGL_MESA:
                        AVG_TRACE(Logger::CONFIG, "  Mesa YCbCr texture support.");
                        break;
                    case DisplayEngine::OGL_APPLE:
                        AVG_TRACE(Logger::CONFIG, "  Apple YCbCr texture support.");
                        break;
                    case DisplayEngine::OGL_SHADER:
                        AVG_TRACE(Logger::CONFIG, "  Fragment shader YCbCr texture support.");
                        break;
                }
                AVG_TRACE(Logger::CONFIG, "  RGB order: " 
                        << (m_bUseRGBOrder?"true":"false"));
                AVG_TRACE(Logger::CONFIG, "  Use pixel buffers: " 
                        << (m_bUsePixelBuffers?"true":"false"));
                AVG_TRACE(Logger::CONFIG, "  Multisample samples: " 
                        << m_MultiSampleSamples);
                
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

        // Find and parse dtd.
        // PREFIXDIR is the install prefix set by configure.

        string sDTDFName = "avg.dtd";
        xmlDtdPtr dtd = xmlParseDTD(NULL, (const xmlChar*) sDTDFName.c_str());
        if (!dtd) {
            AVG_TRACE(Logger::WARNING, 
                    "DTD not found at " << sDTDFName << ". Not validating xml files.");
        }

        // Construct path.
        string RealFilename;
        if (filename[0] == '/') {
            RealFilename = filename; 
        } else {
            char szBuf[1024];
            getcwd(szBuf, 1024);
            m_CurDirName = string(szBuf)+"/";
            RealFilename = m_CurDirName+filename;
        }
        m_CurDirName = RealFilename.substr(0, RealFilename.rfind('/')+1);
        
        xmlPedanticParserDefault(1);
        xmlDoValidityCheckingDefaultValue =0;

        xmlDocPtr doc;
        doc = xmlParseFile(RealFilename.c_str());
        if (!doc) {
            throw (Exception(AVG_ERR_XML_PARSE, 
                        string("Error parsing xml document ")+RealFilename));
        }

        xmlValidCtxtPtr cvp = xmlNewValidCtxt();
        cvp->error = xmlParserValidityError;
        cvp->warning = xmlParserValidityWarning;
        int valid=xmlValidateDtd(cvp, doc, dtd);  
        xmlFreeValidCtxt(cvp);
        if (!valid) {
            throw (Exception(AVG_ERR_XML_PARSE, 
                    filename + " does not validate."));
        }

        m_pRootNode = dynamic_cast<AVGNode*>
            (createNodeFromXml(doc, xmlDocGetRootElement(doc), 0));
        initDisplay(xmlDocGetRootElement(doc));
        initNode(m_pRootNode, 0);
        DRect rc = m_pRootNode->getRelViewport();

        xmlFreeDoc(doc);
    } catch (Exception& ex) {
        AVG_TRACE(Logger::ERROR, ex.GetStr());
    } catch (Magick::Exception & ex) {
        AVG_TRACE(Logger::ERROR, ex.what());
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
        
        m_EventDispatcher.addSource(m_pEventSource);
        m_EventDispatcher.addSink(&m_EventDumper);
        m_EventDispatcher.addSink(this);
        
        m_pDisplayEngine->initRender();
        m_bStopping = false;

        m_pDisplayEngine->render(m_pRootNode, true);
        
        Profiler::get().start();
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

    } catch  (Exception& ex) {
        AVG_TRACE(Logger::ERROR, ex.GetStr());
    }
    m_bIsPlaying = false;
}

void Player::stop ()
{
    m_bStopping = true;
}

bool Player::isPlaying()
{
    return m_bIsPlaying;
}

void Player::setFramerate(double rate) {
    if (!m_pDisplayEngine) {
        AVG_TRACE(Logger::ERROR, 
                "Player::setFramerate called without a loaded avg file. Aborting.");
        exit(-1);
    }
    m_pDisplayEngine->setFramerate(rate);
}

bool Player::setVBlankFramerate(int rate) {
    if (!m_pDisplayEngine) {
        AVG_TRACE(Logger::ERROR, 
                "Player::setVBlankFramerate called without a loaded avg file. Aborting.");
        exit(-1);
    }
    return m_pDisplayEngine->setVBlankRate(rate);
}

TestHelper * Player::getTestHelper()
{
    return &m_TestHelper;
}

int Player::setInterval(int time, PyObject * pyfunc)
{
    Timeout *t = new Timeout(time, pyfunc, true);
    if (m_bInHandleTimers) {
        m_NewTimeouts.push_back(t);
    } else {
        addTimeout(t);
    }
    return t->GetID();
}

int Player::setTimeout(int time, PyObject * pyfunc)
{
    Timeout *t = new Timeout(time, pyfunc, false);
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
            if (m_bInHandleTimers) {
                // Can't kill timeouts during timeout handling...
                m_KilledTimeouts.push_back(id);
            } else {
                delete *it;
                m_PendingTimeouts.erase(it);
            }
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
    return m_EventDispatcher.getLastMouseEvent();
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
        m_bShowCursor = bShow;
    }
}


Node * Player::getElementByID (const std::string& id)
{
    if (m_IDMap.find(id) != m_IDMap.end()) {
        return m_IDMap.find(id)->second;
    } else {
        AVG_TRACE(Logger::WARNING, "getElementByID(" << id << ") failed.");
        return 0;
    }
}

AVGNode * Player::getRootNode ()
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
        {
            ScopeTimer Timer(TimersProfilingZone);
            handleTimers();
        }
        {
            ScopeTimer Timer(EventsProfilingZone);
            m_EventDispatcher.dispatch();
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
        Profiler::get().dumpFrame();
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
    Profiler::get().reset();
}

double Player::getFramerate ()
{
    if (!m_pDisplayEngine) {
        return 0;
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

void Player::initConfig() {
    // Get data from config files.
    ConfigMgr* pMgr = ConfigMgr::get();
    
    m_sDisplaySubsystem = *pMgr->getOption("scr", "subsys");
    
    m_BPP = atoi(pMgr->getOption("scr", "bpp")->c_str());
    if (m_BPP != 15 && m_BPP != 16 && m_BPP != 24 && m_BPP != 32) {
        AVG_TRACE(Logger::ERROR, 
                "BPP must be 15, 16, 24 or 32. Current value is " 
                << m_BPP << ". Aborting." );
        exit(-1);
    }
    m_bFullscreen = pMgr->getBoolOption("scr", "fullscreen", false);

    m_WindowWidth = atoi(pMgr->getOption("scr", "windowwidth")->c_str());
    m_WindowHeight = atoi(pMgr->getOption("scr", "windowheight")->c_str());

    if (m_bFullscreen && (m_WindowWidth != 0 || m_WindowHeight != 0)) {
        AVG_TRACE(Logger::ERROR, 
                "Can't set fullscreen and window size at once. Aborting.");
        exit(-1);
    }
    if (m_WindowWidth != 0 && m_WindowHeight != 0) {
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
            m_YCbCrMode = DisplayEngine::OGL_SHADER;
        } else if (*psYCbCrMode == "mesa") {
            m_YCbCrMode = DisplayEngine::OGL_MESA;
        } else if (*psYCbCrMode == "apple") {
            m_YCbCrMode = DisplayEngine::OGL_APPLE;
        } else if (*psYCbCrMode == "none") {
            m_YCbCrMode = DisplayEngine::NONE;
        } else {
            AVG_TRACE(Logger::ERROR, 
                    "avgrc: ycbcrmode must be shader, mesa, apple or none. Current value is " 
                    << *psYCbCrMode << ". Aborting." );
            exit(-1);
        }

        m_bUseRGBOrder = pMgr->getBoolOption("scr", "usergborder", false);
        m_bUsePixelBuffers = pMgr->getBoolOption("scr", "usepixelbuffers", true);
        m_MultiSampleSamples = pMgr->getIntOption("scr", "multisamplesamples", 1);
    }
}

Node * Player::createNodeFromXmlString (const string& sXML)
{
    try {
        xmlDocPtr doc;
        doc = xmlParseMemory(sXML.c_str(), sXML.length());
        if (!doc) {
            throw (Exception(AVG_ERR_XML_PARSE, 
                        string("Error parsing xml:\n  ")+sXML));
        }
        Node * pNode = createNodeFromXml(doc, xmlDocGetRootElement(doc), 0);

        xmlFreeDoc(doc);
        return pNode;
    } catch (Exception& ex) {
        AVG_TRACE(Logger::ERROR, ex.GetStr());
        return 0;
    } catch (Magick::Exception& ex) {
        AVG_TRACE(Logger::ERROR, ex.what());
        return 0;
    }
}

Node * Player::createNodeFromXml (const xmlDocPtr xmlDoc, 
        const xmlNodePtr xmlNode, DivNode * pParent)
{
    const char * nodeType = (const char *)xmlNode->name;
    Node * curNode = 0;
    
    string id = getDefaultedStringAttr (xmlNode, "id", "");
    if (!strcmp (nodeType, "avg")) {
        curNode = new AVGNode(xmlNode);
    } else if (!strcmp (nodeType, "div")) {
        curNode = new DivNode(xmlNode, pParent);
    } else if (!strcmp (nodeType, "image")) {
        curNode = new Image(xmlNode, pParent);
    } else if (!strcmp (nodeType, "words")) {
        curNode = new Words(xmlNode, pParent);
        string s = getXmlChildrenAsString(xmlDoc, xmlNode);
        dynamic_cast<Words*>(curNode)->initText(s);
    } else if (!strcmp (nodeType, "video")) {
        curNode = new Video(xmlNode, pParent);
    } else if (!strcmp (nodeType, "camera")) {
        curNode = new Camera(xmlNode, pParent);
    }
    else if (!strcmp (nodeType, "panoimage")) {
        curNode = new PanoImage(xmlNode, pParent);
    } 
    else if (!strcmp (nodeType, "text") || 
               !strcmp (nodeType, "comment")) {
        // Ignore whitespace & comments
        return 0;
    } else {
        throw (Exception (AVG_ERR_XML_NODE_UNKNOWN, 
            string("Unknown node type ")+(const char *)nodeType+" encountered."));
    }
    // If this is a container, recurse into children
    DivNode * curDivNode = dynamic_cast<DivNode*>(curNode);
    if (curDivNode) {
        xmlNodePtr curXmlChild = xmlNode->xmlChildrenNode;
        while (curXmlChild) {
            Node *curChild = createNodeFromXml (xmlDoc, curXmlChild, 
                    curDivNode);
            if (curChild) {
                curDivNode->addChild(curChild);
            }
            curXmlChild = curXmlChild->next;
        }
    }
    return curNode;
}

void Player::initNode(Node * pNode, DivNode * pParent)
{
    const string& ID = pNode->getID();
    pNode->init(m_pDisplayEngine, pParent, this);
    pNode->initVisible();
    // If this is a container, recurse into children
    DivNode * curDivNode = dynamic_cast<DivNode*>(pNode);
    if (curDivNode) {
        for (int i=0; i<curDivNode->getNumChildren(); ++i) {
            initNode(curDivNode->getChild(i), curDivNode);
        }
    }
    if (ID != "") {
        if (m_IDMap.find(ID) != m_IDMap.end()) {
            throw (Exception (AVG_ERR_XML_DUPLICATE_ID,
                string("Error: duplicate id ")+ID));
        }
        m_IDMap.insert(NodeIDMap::value_type(ID, pNode));
    }
}

void Player::initDisplay(const xmlNodePtr xmlNode) {
    int Height = getDefaultedIntAttr (xmlNode, "height", 0);
    int Width = getDefaultedIntAttr (xmlNode, "width", 0);
    SDLDisplayEngine * pSDLDisplayEngine = dynamic_cast<SDLDisplayEngine*>(m_pDisplayEngine);
    if (pSDLDisplayEngine) {
        pSDLDisplayEngine->setOGLOptions(m_bUsePOW2Textures, m_YCbCrMode, m_bUseRGBOrder,
                m_bUsePixelBuffers, m_MultiSampleSamples);
    }
    m_pDisplayEngine->init(Width, Height, m_bFullscreen, m_BPP, 
            m_WindowWidth, m_WindowHeight);
    m_pDisplayEngine->showCursor(m_bShowCursor);
}

void Player::handleTimers()
{
    vector<Timeout *>::iterator it;
    m_bInHandleTimers = true;
    it = m_PendingTimeouts.begin();
    while (it != m_PendingTimeouts.end() && (*it)->IsReady() && !m_bStopping)
    {
        (*it)->Fire();
        if (!(*it)->IsInterval()) {
            delete *it;
            it = m_PendingTimeouts.erase(it);
        } else {
            Timeout* pTempTimeout = *it;
            it = m_PendingTimeouts.erase(it);
            addTimeout(pTempTimeout);
        }
    }
    for (it = m_NewTimeouts.begin(); it != m_NewTimeouts.end(); ++it) {
        addTimeout(*it);
    }
    m_NewTimeouts.clear();
    vector<int>::iterator i;
    for (i = m_KilledTimeouts.begin(); i != m_KilledTimeouts.end(); ++i) {
        int id = *i;
        
        vector<Timeout *>::iterator it2;
        for (it2=m_PendingTimeouts.begin(); it2 != m_PendingTimeouts.end(); ++it2) {
            if (id == (*it2)->GetID()) {
                delete *it2;
                m_PendingTimeouts.erase(it2);
                break;
            }
        }
    }
    m_KilledTimeouts.clear();
    m_bInHandleTimers = false;
    
}


bool Player::handleEvent(Event * pEvent)
{
    m_pCurEvent = pEvent;
    switch (pEvent->getType()) {
        case Event::MOUSEMOTION:
        case Event::MOUSEBUTTONUP:
        case Event::MOUSEBUTTONDOWN:
        case Event::MOUSEOVER:
        case Event::MOUSEOUT:
            {
                MouseEvent * pMouseEvent = dynamic_cast<MouseEvent*>(pEvent);
                DPoint pos(pMouseEvent->getXPosition(), 
                        pMouseEvent->getYPosition());
                Node * pNode;
                if (pEvent->getType() != Event::MOUSEOVER &&
                        pEvent->getType() != Event::MOUSEOUT)
                {
                    pNode = m_pRootNode->getElementByPos(pos);
                } else {
                    pNode = pMouseEvent->getElement();
                }
                if (pNode != m_pLastMouseNode && 
                        pEvent->getType() != Event::MOUSEOVER &&
                        pEvent->getType() != Event::MOUSEOUT)
                {
                    if (pNode) {
                        createMouseOver(pMouseEvent, Event::MOUSEOVER, pNode);
                    }
                    if (m_pLastMouseNode) {
                        createMouseOver(pMouseEvent, Event::MOUSEOUT, 
                                m_pLastMouseNode);
                    }
                    m_pLastMouseNode = pNode;
                }
                if (pNode) {
                    pNode->handleMouseEvent(pMouseEvent);
                }
            }
            break;
        case Event::KEYDOWN:
        case Event::KEYUP:
            {
                KeyEvent * pKeyEvent = dynamic_cast<KeyEvent*>(pEvent);
                m_pRootNode->handleKeyEvent(pKeyEvent);
                if (pEvent->getType() == Event::KEYDOWN &&
                    pKeyEvent->getKeyCode() == 27) 
                {
                    m_bStopping = true;
                }
            }
            break;
        case Event::QUIT:
            m_bStopping = true;
            break;
        default:
            AVG_TRACE(Logger::ERROR, "Unknown event type in Player::handleEvent.");
            break;
    }
    // Don't pass on any events.
    return true; 
}

DisplayEngine * Player::getDisplayEngine() const 
{
    return m_pDisplayEngine;
}

void Player::createMouseOver(MouseEvent * pOtherEvent, Event::Type Type, 
                Node * pNode)
{
    MouseEvent * pNewEvent = new MouseEvent(Type,
            pOtherEvent->getLeftButtonState(),
            pOtherEvent->getMiddleButtonState(),
            pOtherEvent->getRightButtonState(),
            pOtherEvent->getXPosition(),
            pOtherEvent->getYPosition(),
            pOtherEvent->getButton());
    pNewEvent->setElement(pNode);
    m_EventDispatcher.addEvent(pNewEvent);
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
    m_pDisplayEngine->deinitRender();
    m_pDisplayEngine->teardown();
    delete m_pRootNode;
    m_pRootNode = 0;
    m_pLastMouseNode = 0;
    m_IDMap.clear();
    initConfig();
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
