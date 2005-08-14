//
// $Id$
//

#include "../avgconfig.h"
#include "Player.h"

#include "AVGNode.h"
#include "DivNode.h"
#include "Words.h"
#include "Video.h"
#include "PanoImage.h"
#include "Camera.h"
#include "Excl.h"
#include "Image.h"

#include "Event.h"
#include "MouseEvent.h"
#include "KeyEvent.h"

#ifdef AVG_ENABLE_DFB
#include "DFBDisplayEngine.h"
#endif
#ifdef AVG_ENABLE_GL
#include "SDLDisplayEngine.h"
#endif

#include "FramerateManager.h"

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
      m_pFramerateManager(0),
      m_bInHandleTimers(false),
      m_pLastMouseNode(0),
      m_bShowCursor(true)
{
    TimeSource::get()->getCyclesPerSecond();
    initConfig();
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
        AVG_TRACE(Logger::ERROR,
                "Player::setDisplayEngine called after loadFile."
                << " Aborting.");
        exit(-1);
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

void Player::loadFile (const std::string& filename)
{
    try {
        AVG_TRACE(Logger::PROFILE, 
                std::string("Player::LoadFile(") + filename + ")");
        assert (!m_pRootNode);

        // Get display configuration.
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
            m_pDisplayEngine->showCursor(m_bShowCursor);
        }

        // Find and parse dtd.
        string sDTDFName = findFile("avg.dtd", "", "includepath", "");
        if (sDTDFName == "") {
            AVG_TRACE(Logger::ERROR,
                    "Required file avg.dtd not found. Search path was " 
                    << *ConfigMgr::get()->getGlobalOption("includepath")
                    << ". Aborting.");
            exit(-1);
        }
        xmlDtdPtr dtd = xmlParseDTD(NULL, (const xmlChar*) sDTDFName.c_str());
        if (!dtd) {
            AVG_TRACE(Logger::ERROR, 
                    "Required DTD not found at " << sDTDFName << ". Aborting.");
            exit(-1);
        }

        // Construct path.
        char szBuf[1024];
        getcwd(szBuf, 1024);
        m_CurDirName = string(szBuf)+"/";
        string RealFilename = m_CurDirName+filename;
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

void Player::play (double framerate)
{
    try {
        if (!m_pRootNode) {
            AVG_TRACE(Logger::ERROR, "play called, but no xml file loaded.");
        }
        assert(m_pRootNode);
        
        m_EventDispatcher.addSource(m_pEventSource);
        m_EventDispatcher.addSink(&m_EventDumper);
        m_EventDispatcher.addSink(this);
        
        m_pFramerateManager = new FramerateManager;
        m_pFramerateManager->SetRate(framerate);
        m_bStopping = false;

        m_pDisplayEngine->render(m_pRootNode, m_pFramerateManager, true);
//        setPriority();
        
        Profiler::get().start();
        try {
            while (!m_bStopping) {
                doFrame();
            }
        } catch (...) {
            cleanup();
            throw;
        }
        cleanup();

    } catch  (Exception& ex) {
        AVG_TRACE(Logger::ERROR, ex.GetStr());
    }
}

void Player::stop ()
{
    m_bStopping = true;
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

bool Player::screenshot(const std::string& sFilename)
{
    BitmapPtr pBmp = m_pDisplayEngine->screenshot();
    try {
        pBmp->save(sFilename);
        AVG_TRACE(Logger::WARNING, "Saved screen as " << sFilename << ".");
    } catch (Magick::Exception& ex) {
        AVG_TRACE(Logger::WARNING, "Could not save screenshot. Error: " 
                << ex.what() << ".");
        return false;
    }
    return true;
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
            m_pDisplayEngine->render(m_pRootNode, m_pFramerateManager, false);
        }
        {
            ScopeTimer Timer(ListenerProfilingZone);
            for (unsigned int i=0; i<m_Listeners.size(); ++i) {
                m_Listeners[i]->onFrameEnd();
            }
        }
    }
    long FrameTime = long(MainProfilingZone.getUSecs()/1000);
    long TargetTime = long(1000/m_pFramerateManager->GetRate());
    if (FrameTime > TargetTime+2) {
        AVG_TRACE(Logger::PROFILE_LATEFRAMES, "frame too late by " <<
                FrameTime-TargetTime << " ms.");
        Profiler::get().dumpFrame();
    }
    Profiler::get().reset();
}

void Player::setPriority() 
{
    pthread_t ThisThreadID = pthread_self();
    int Policy;
    sched_param Params;
    int err = pthread_getschedparam(ThisThreadID, &Policy, &Params);
    if (err) {
        AVG_TRACE(Logger::ERROR, "Player::play: getschedparam failed.");
    }
    Params.sched_priority++; //  = PTHREAD_MAX_PRIORITY;
    err = pthread_setschedparam(ThisThreadID, Policy, &Params);
    if (err) {
        AVG_TRACE(Logger::ERROR, "Player::play: setschedparam failed.");
    }
}
        

double Player::getFramerate ()
{
    return m_pFramerateManager->GetRate();
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

    string sFullscreen = *pMgr->getOption("scr", "fullscreen");
    if (sFullscreen == "true") {
        m_bFullscreen = true;
    } else if (sFullscreen == "false") {
        m_bFullscreen = false;
    } else {
        AVG_TRACE(Logger::ERROR, 
                "Unrecognized value for option fullscreen: " 
                << sFullscreen);
        exit(-1);
    }

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

    AVG_TRACE(Logger::CONFIG, "Display subsystem: " << 
            m_sDisplaySubsystem);
    AVG_TRACE(Logger::CONFIG, "Display bpp: " << m_BPP);
    AVG_TRACE(Logger::CONFIG, "Display fullscreen: "
            << m_bFullscreen?"true":"false");
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
        const xmlNodePtr xmlNode, Container * pParent)
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
    } else if (!strcmp (nodeType, "excl")) {
        curNode = new Excl(xmlNode, pParent);
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
    Container * curContainer = dynamic_cast<Container*>(curNode);
    if (curContainer) {
        xmlNodePtr curXmlChild = xmlNode->xmlChildrenNode;
        while (curXmlChild) {
            Node *curChild = createNodeFromXml (xmlDoc, curXmlChild, 
                    curContainer);
            if (curChild) {
                curContainer->addChild(curChild);
            }
            curXmlChild = curXmlChild->next;
        }
    }
    return curNode;
}

void Player::initNode(Node * pNode, Container * pParent)
{
    const string& ID = pNode->getID();
    pNode->init(m_pDisplayEngine, pParent, this);
    pNode->initVisible();
    // If this is a container, recurse into children
    Container * curContainer = dynamic_cast<Container*>(pNode);
    if (curContainer) {
        for (int i=0; i<curContainer->getNumChildren(); ++i) {
            initNode(curContainer->getChild(i), curContainer);
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
    m_pDisplayEngine->init(Width, Height, m_bFullscreen, m_BPP, 
            m_WindowWidth, m_WindowHeight);
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
            {
                MouseEvent * pMouseEvent = dynamic_cast<MouseEvent*>(pEvent);
                DPoint pos(pMouseEvent->getXPosition(), 
                        pMouseEvent->getYPosition());
                Node * pNode = m_pRootNode->getElementByPos(pos);            
                if (pNode != m_pLastMouseNode) {
                    if (pNode) {
                        createMouseOver(pMouseEvent, Event::MOUSEOVER);
                    }
                    if (m_pLastMouseNode) {
                        createMouseOver(pMouseEvent, Event::MOUSEOUT);
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
        case Event::MOUSEOVER:
        case Event::MOUSEOUT:
            break;
        default:
            AVG_TRACE(Logger::ERROR, "Unknown event type in Player::handleEvent.");
            break;
    }
    // Don't pass on any events.
    return true; 
}

void Player::createMouseOver(MouseEvent * pOtherEvent, int Type)
{
    MouseEvent * pNewEvent = new MouseEvent(Type,
            pOtherEvent->getLeftButtonState(),
            pOtherEvent->getMiddleButtonState(),
            pOtherEvent->getRightButtonState(),
            pOtherEvent->getXPosition(),
            pOtherEvent->getYPosition(),
            pOtherEvent->getButton());
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

    m_pRootNode = 0;

    delete m_pFramerateManager;
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
