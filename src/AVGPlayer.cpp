//
// $Id$
//

#include "AVGPlayer.h"
#include "AVGAVGNode.h"
#include "AVGImage.h"
#include "AVGVideo.h"
#include "AVGWords.h"
#include "AVGExcl.h"
#include "AVGEvent.h"
#include "AVGMouseEvent.h"
#include "AVGKeyEvent.h"
#include "AVGWindowEvent.h"
#include "AVGException.h"
#include "AVGRegion.h"
#include "AVGDFBDisplayEngine.h"
#include "AVGSDLDisplayEngine.h"
#include "AVGLogger.h"
#include "AVGConradRelais.h"
#include "XMLHelper.h"
#include "JSHelper.h"
#include "FileHelper.h"

#include "acIJSContextPublisher.h"

#include <paintlib/plstdpch.h>
#include <paintlib/plexcept.h>
#include <paintlib/pldebug.h>
#include <paintlib/plpoint.h>
#include <paintlib/plpixel32.h>

#include <libxml/xmlmemory.h>

#include <algorithm>
#include <iostream>
#include <sstream>

#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <pthread.h>

#include "nsMemory.h"

#ifdef XPCOM_GLUE
#include "nsXPCOMGlue.h"
#endif

#include <xpcom/nsComponentManagerUtils.h>

using namespace std;


AVGPlayer::AVGPlayer()
    : m_pRootNode (0),
      m_pDisplayEngine(0),
      m_pFramerateManager(0),
      m_pDebugDest(0),
      m_bInHandleTimers(false),
      m_pLastMouseNode(0)
{
#ifdef XPCOM_GLUE
    XPCOMGlueStartup("XPCOMComponentGlue");
#endif
    NS_INIT_ISUPPORTS();
    nsresult myErr;
    nsCOMPtr<acIJSContextPublisher> myJSContextPublisher =
        do_CreateInstance("@artcom.com/jscontextpublisher;1", &myErr);
    if (NS_FAILED(myErr)) {
        AVG_TRACE(DEBUG_ERROR, 
              "Could not obtain reference to js context. Was xpshell used to start AVGPlayer?");
        exit(-1);
    }
    myJSContextPublisher->GetContext((PRInt32*) &m_pJSContext);
    AVGLogger::get()->setDestination(&cerr);
}

AVGPlayer::~AVGPlayer()
{
#ifdef XPCOM_GLUE
    XPCOMGlueShutdown();
#endif
    if (m_pDisplayEngine) {
        delete m_pDisplayEngine;
    }
    if (m_pDebugDest) {
        delete m_pDebugDest;
    }

}

NS_IMPL_ISUPPORTS1_CI(AVGPlayer, IAVGPlayer);

NS_IMETHODIMP
AVGPlayer::LoadFile(const char * fileName, 
        PRBool * pResult)
{
    AVG_TRACE(DEBUG_MEMORY, 
          std::string("AVGPlayer::LoadFile(") + fileName + ")");
    try {
        loadFile (fileName);
        *pResult = true;
    } catch  (AVGException& ex) {
        AVG_TRACE(DEBUG_ERROR, ex.GetStr());
        *pResult = false;
    } catch (PLTextException& ex) {
        AVG_TRACE(DEBUG_ERROR, (const char*)ex);
        *pResult = false;
    }
    return NS_OK;
}

NS_IMETHODIMP
AVGPlayer::Play(float framerate)
{
	play(framerate);
	return NS_OK;
}

NS_IMETHODIMP
AVGPlayer::Stop()
{
	stop();
	return NS_OK;
}

NS_IMETHODIMP 
AVGPlayer::GetElementByID(const char *id, IAVGNode **_retval)
{
    AVGNode * pNode = getElementByID(id);
    if (!pNode) {
        AVG_TRACE(DEBUG_ERROR, "getElementByID(" << id << ") failed");
    }
    *_retval = getElementByID(id);
    NS_IF_ADDREF(*_retval);
    return NS_OK;
}

NS_IMETHODIMP 
AVGPlayer::GetRootNode(IAVGNode **_retval)
{
    NS_IF_ADDREF(m_pRootNode);
    *_retval = m_pRootNode;
    return NS_OK;
}

NS_IMETHODIMP
AVGPlayer::SetInterval(PRInt32 time, const char * code, PRInt32 * pResult)
{
    AVGTimeout *t = new AVGTimeout(time, code, true, m_pJSContext);
    if (m_bInHandleTimers) {
        m_NewTimeouts.push_back(t);
    } else {
        addTimeout(t);
    }
    *pResult = t->GetID();
    return NS_OK;
}

NS_IMETHODIMP
AVGPlayer::SetTimeout(PRInt32 time, const char * code, PRInt32 * pResult)
{
    AVGTimeout *t = new AVGTimeout(time, code, false, m_pJSContext);
    if (m_bInHandleTimers) {
        m_NewTimeouts.push_back(t);
    } else {
        addTimeout(t);
    }
    *pResult = t->GetID();
    return NS_OK;
}

NS_IMETHODIMP
AVGPlayer::ClearInterval(PRInt32 id, PRBool * pResult)
{
    vector<AVGTimeout*>::iterator it;
    for (it=m_PendingTimeouts.begin(); it!=m_PendingTimeouts.end(); it++) {
        if (id == (*it)->GetID()) {
            if (m_bInHandleTimers) {
                // Can't kill timeouts during timeout handling...
                m_KilledTimeouts.push_back(*it);
            } else {
                delete *it;
                m_PendingTimeouts.erase(it);
            }
            *pResult = true;
            return NS_OK;
        }
    }
    *pResult = false;
    return NS_OK;
}

NS_IMETHODIMP 
AVGPlayer::GetCurEvent(IAVGEvent **_retval)
{
    NS_IF_ADDREF(m_pCurEvent);
    *_retval = m_pCurEvent;
    return NS_OK;
}

NS_IMETHODIMP 
AVGPlayer::SetDebugOutput(PRInt32 flags)
{
    AVGLogger::get()->setCategories(flags);
    return NS_OK;
}

NS_IMETHODIMP
AVGPlayer::SetDebugOutputFile(const char *name)
{
    if (m_pDebugDest) {
        delete m_pDebugDest;
    }
    m_pDebugDest = new ofstream(name, ios::out | ios::app);
    if (!*m_pDebugDest) {
        AVG_TRACE(DEBUG_ERROR, "Could not open " << name << " as log destination.");
    } else {
        AVGLogger::get()->setDestination(m_pDebugDest);
        AVG_TRACE(DEBUG_ERROR, "Logging started ");
    }
}
    
NS_IMETHODIMP
AVGPlayer::GetErrStr(char ** ppszResult)
{
	strcpy (*ppszResult, "kaputt");
	return NS_OK;
}

NS_IMETHODIMP
AVGPlayer::GetErrCode(PRInt32 * pResult)
{
	*pResult = 0;
	return NS_OK;
}

NS_IMETHODIMP 
AVGPlayer::CreateRelais(PRInt16 port, IAVGConradRelais **_retval)
{
    nsresult rv;
    AVGConradRelais* pRelais;
    rv = nsComponentManager::CreateInstance ("@c-base.org/avgconradrelais;1", 0,
            NS_GET_IID(IAVGConradRelais), (void**)&pRelais);
    if (NS_FAILED(rv)) {
        AVG_TRACE(DEBUG_ERROR, "CreateRelais failed: " << rv);
    }
    pRelais->init(port);
    m_pRelais.push_back(pRelais);
    *_retval = pRelais;
    return NS_OK;
}

void AVGPlayer::loadFile (const std::string& filename)
{
    jsgc();
    PLASSERT (!m_pRootNode);
    if (!m_pDisplayEngine) {
        char * pszDisplay = getenv("AVG_DISPLAY");
        if (!pszDisplay || strcmp(pszDisplay, "DFB") == 0) {
            m_pDisplayEngine = new AVGDFBDisplayEngine ();
            m_pEventSource =
                    dynamic_cast<AVGDFBDisplayEngine *>(m_pDisplayEngine);
        } else if (strcmp(pszDisplay, "OGL") == 0) {
            m_pDisplayEngine = new AVGSDLDisplayEngine ();
            m_pEventSource = 
                    dynamic_cast<AVGSDLDisplayEngine *>(m_pDisplayEngine);
        } else {
            AVG_TRACE(DEBUG_ERROR, 
                    "AVG_DISPLAY set to unknown value " << pszDisplay);
            exit(-1);
        }
    }

    // Construct path.
    const char * pCurFilename;
    int lineno;
    getJSFileLine(m_pJSContext, pCurFilename, lineno);
    string Path = getPath(pCurFilename);
    string RealFilename = Path+filename;

    xmlPedanticParserDefault(1);
    xmlDoValidityCheckingDefaultValue =1;
    
    xmlDocPtr doc;
    doc = xmlParseFile(RealFilename.c_str());
    if (!doc) {
        throw (AVGException(AVG_ERR_XML_PARSE, 
                string("Error parsing xml document ")+RealFilename));
    }

    m_CurDirName = RealFilename.substr(0,RealFilename.rfind('/')+1);
    m_pRootNode = dynamic_cast<AVGAVGNode*>
            (createNodeFromXml(xmlDocGetRootElement(doc), 0));
    PLRect rc = m_pRootNode->getRelViewport();
    xmlFreeDoc(doc);
}

void AVGPlayer::play (double framerate)
{
    try {
        DFBResult err;
        if (!m_pRootNode) {
            AVG_TRACE(DEBUG_ERROR, "play called, but no xml file loaded.");
        }
        PLASSERT (m_pRootNode);
        //    setRealtimePriority();

        m_EventDispatcher.addSource(m_pEventSource);
        m_EventDispatcher.addSink(&m_EventDumper);
        m_EventDispatcher.addSink(this);

        m_pFramerateManager = new AVGFramerateManager;
        m_pFramerateManager->SetRate(framerate);
        m_bStopping = false;

        m_pDisplayEngine->render(m_pRootNode, m_pFramerateManager, true);
        while (!m_bStopping) {
            doFrame();
        }
        // Kill all timeouts.
        vector<AVGTimeout*>::iterator it;
        for (it=m_PendingTimeouts.begin(); it!=m_PendingTimeouts.end(); it++) {
            delete *it;
        }
        m_PendingTimeouts.clear();

        NS_IF_RELEASE(m_pRootNode);
        m_pRootNode = 0;
        delete m_pFramerateManager;
        m_IDMap.clear();
    } catch  (AVGException& ex) {
        AVG_TRACE(DEBUG_ERROR, ex.GetStr());
    }
}

void AVGPlayer::stop ()
{
    m_bStopping = true;
}

void AVGPlayer::doFrame ()
{
    handleTimers();
    
    m_EventDispatcher.dispatch();
    if (!m_bStopping) {
        m_pDisplayEngine->render(m_pRootNode, m_pFramerateManager, false);
    }
    for (int i=0; i<m_pRelais.size(); i++) {
        m_pRelais[i]->send();
    }
    
    jsgc();
}

void AVGPlayer::jsgc()
{
    JS_GC(m_pJSContext);
}

AVGNode * AVGPlayer::getElementByID (const std::string id)
{
    if (m_IDMap.find(id) != m_IDMap.end()) {
        return m_IDMap.find(id)->second;
    } else {
        return 0;
    }
}

AVGAVGNode * AVGPlayer::getRootNode ()
{
    return m_pRootNode;
}

double AVGPlayer::getFramerate ()
{
    return m_pFramerateManager->GetRate();
}

void AVGPlayer::addEvent (int time, AVGEvent * event)
{
}

void AVGPlayer::setRealtimePriority()
{
    sched_param myParam;

    myParam.sched_priority = sched_get_priority_max (SCHED_FIFO);
    int myRetVal = pthread_setschedparam (pthread_self(), 
		    SCHED_FIFO, &myParam);
    if (myRetVal == 0) {
        AVG_TRACE(DEBUG_PROFILE, "AVGPlayer running with realtime priority.");
    } else {
        AVG_TRACE(DEBUG_PROFILE, "Setting realtime priority failed.");
    }
}

AVGNode * AVGPlayer::createNodeFromXml (const xmlNodePtr xmlNode, 
        AVGContainer * pParent)
{
    const xmlChar * nodeType = xmlNode->name;
    AVGNode * curNode = 0;
    if (!xmlStrcmp (nodeType, (const xmlChar *)"avg")) {
        // create node itself.
        string id;
        int x,y,z;
        int width, height;
        double opacity;
        getVisibleNodeAttrs(xmlNode, &id, &x, &y, &z, &width, &height, &opacity);
        curNode = AVGAVGNode::create();
        curNode->init(id, m_pDisplayEngine, pParent, this);
        curNode->initVisible(x, y, z, width, height, opacity);
        initEventHandlers(curNode, xmlNode);
        if (!pParent) {
            initDisplay(dynamic_cast<AVGAVGNode*>(curNode));
        }
    } else if (!xmlStrcmp (nodeType, (const xmlChar *)"image")) {
        string id;
        int x,y,z;
        int width, height;
        double opacity;
        getVisibleNodeAttrs(xmlNode, &id, &x, &y, &z, &width, &height, &opacity);
        string filename = m_CurDirName + 
                getRequiredStringAttr(xmlNode, (const xmlChar *)"href");

        AVGImage * pImage = AVGImage::create();
        curNode = pImage;
        pImage->init(id, x, y, z, width, height, opacity, 
                filename, m_pDisplayEngine, pParent, this);
        initEventHandlers(curNode, xmlNode);
    } else if (!xmlStrcmp (nodeType, (const xmlChar *)"video")) {
        string id;
        int x,y,z;
        int width, height;
        double opacity;
        getVisibleNodeAttrs(xmlNode, &id, &x, &y, &z, &width, &height, &opacity);
        string filename = m_CurDirName + 
                getRequiredStringAttr(xmlNode, (const xmlChar *)"href");
        bool bLoop = getDefaultedBoolAttr(xmlNode, (const xmlChar *)"loop", false); 
        bool bOverlay = getDefaultedBoolAttr(xmlNode, (const xmlChar *)"overlay", false); 
        AVGVideo * pVideo = AVGVideo::create();
        curNode = pVideo;
        pVideo->init(id, x, y, z, width, height, opacity, 
                filename, bLoop, bOverlay, m_pDisplayEngine, pParent, this);
        initEventHandlers(curNode, xmlNode);
    } else if (!xmlStrcmp (nodeType, (const xmlChar *)"words")) {
        string id;
        int x,y,z;
        int width, height;  // Bogus
        double opacity;
        getVisibleNodeAttrs(xmlNode, &id, &x, &y, &z, &width, &height, &opacity);
        string font = getDefaultedStringAttr(xmlNode, 
                (const xmlChar *)"font", "arial");
        string str = getRequiredStringAttr(xmlNode, (const xmlChar *)"text");
        string color = getDefaultedStringAttr(xmlNode, 
                (const xmlChar *)"color", "FFFFFF");
        int size = getDefaultedIntAttr(xmlNode, (const xmlChar *)"size", 15);
        AVGWords * pWords = AVGWords::create();
        curNode = pWords;
        pWords->init(id, x, y, z, opacity, size,
                font, str, color, m_pDisplayEngine, pParent, this);
        initEventHandlers(curNode, xmlNode);
    } else if (!xmlStrcmp (nodeType, (const xmlChar *)"excl")) {
        string id  = getDefaultedStringAttr (xmlNode, (const xmlChar *)"id", "");
        curNode = AVGExcl::create();
        curNode->init(id, m_pDisplayEngine, pParent, this);
        curNode->initVisible(0,0,1,10000,10000,1);
        initEventHandlers(curNode, xmlNode);
    } else if (!xmlStrcmp (nodeType, (const xmlChar *)"text") || 
               !xmlStrcmp (nodeType, (const xmlChar *)"comment")) {
        // Ignore whitespace & comments
        return 0;
    } else {
        throw (AVGException (AVG_ERR_XML_NODE_UNKNOWN, 
            string("Unknown node type ")+(const char *)nodeType+" encountered."));
    }
    // If this is a container, recurse into children
    AVGContainer * curContainer = dynamic_cast<AVGContainer*>(curNode);
    if (curContainer) {
        xmlNodePtr curXmlChild = xmlNode->xmlChildrenNode;
        while (curXmlChild) {
            AVGNode *curChild = createNodeFromXml (curXmlChild, curContainer);
            if (curChild) {
                curContainer->addChild(curChild);
            }
            curXmlChild = curXmlChild->next;
        }
    }
    const string& ID = curNode->getID();
    if (ID != "") {
        if (m_IDMap.find(ID) != m_IDMap.end()) {
            throw (AVGException (AVG_ERR_XML_DUPLICATE_ID,
                string("Error: duplicate id ")+ID));
        }
        m_IDMap.insert(NodeIDMap::value_type(ID, curNode));
    }
    return curNode;
}
void AVGPlayer::initDisplay(AVGAVGNode * pNode) {
    bool bFullscreen = false;
    char * pszFullscreen = getenv("AVG_FULLSCREEN");
    if (pszFullscreen && strcmp(pszFullscreen, "true") == 0) {
        bFullscreen = true;
    }
    int bpp = 16;
    char * pszBPP = getenv("AVG_BPP");
    if (pszBPP) {
        if (strcmp(pszBPP, "16") == 0) {
            bpp = 16;
        } else if (strcmp(pszBPP, "24") == 0) {
            bpp = 24;
        } else {
            bpp = 24;
            AVG_TRACE(DEBUG_ERROR, "Unrecognized value for AVG_BPP:" << pszBPP
                << ". Setting to 24." );
        }
    }

    m_pDisplayEngine->init(pNode->getRelViewport().Width(), 
            pNode->getRelViewport().Height(), bFullscreen, bpp);
}

void AVGPlayer::getVisibleNodeAttrs (const xmlNodePtr xmlNode, 
        string * pid, int * px, int * py, int * pz,
        int * pWidth, int * pHeight, double * pOpacity)
{
    *pid = getDefaultedStringAttr (xmlNode, (const xmlChar *)"id", "");
    *px = getDefaultedIntAttr (xmlNode, (const xmlChar *)"x", 0);
    *py = getDefaultedIntAttr (xmlNode, (const xmlChar *)"y", 0);
    *pz = getDefaultedIntAttr (xmlNode, (const xmlChar *)"z", 0);
    *pWidth = getDefaultedIntAttr (xmlNode, (const xmlChar *)"width", 0);
    *pHeight = getDefaultedIntAttr (xmlNode, (const xmlChar *)"height", 0);
    *pOpacity = getDefaultedDoubleAttr (xmlNode, (const xmlChar *)"opacity", 1.0);
}

void AVGPlayer::initEventHandlers (AVGNode * pAVGNode, const xmlNodePtr xmlNode)
{
    string MouseMoveHandler = getDefaultedStringAttr 
            (xmlNode, (const xmlChar *)"onmousemove", "");
    string MouseButtonUpHandler = getDefaultedStringAttr 
            (xmlNode, (const xmlChar *)"onmouseup", "");
    string MouseButtonDownHandler = getDefaultedStringAttr 
            (xmlNode, (const xmlChar *)"onmousedown", "");
    string MouseOverHandler = getDefaultedStringAttr 
            (xmlNode, (const xmlChar *)"onmouseover", "");
    string MouseOutHandler = getDefaultedStringAttr 
            (xmlNode, (const xmlChar *)"onmouseout", "");
    pAVGNode->InitEventHandlers
            (MouseMoveHandler, MouseButtonUpHandler, MouseButtonDownHandler, 
             MouseOverHandler, MouseOutHandler);
}


void AVGPlayer::handleTimers()
{
    vector<AVGTimeout *>::iterator it;
    m_bInHandleTimers = true;
    it = m_PendingTimeouts.begin();
    while (it != m_PendingTimeouts.end() && (*it)->IsReady() && !m_bStopping)
    {
        (*it)->Fire(m_pJSContext);
        if (!(*it)->IsInterval()) {
            delete *it;
            it = m_PendingTimeouts.erase(it);
        } else {
            AVGTimeout* pTempTimeout = *it;
            it = m_PendingTimeouts.erase(it);
            addTimeout(pTempTimeout);
        }
    }
    for (it = m_NewTimeouts.begin(); it != m_NewTimeouts.end(); ++it) {
        addTimeout(*it);
    }
    m_NewTimeouts.clear();
    for (it = m_KilledTimeouts.begin(); it != m_KilledTimeouts.end(); ++it) {
        removeTimeout(*it);
    }
    m_KilledTimeouts.clear();
    m_bInHandleTimers = false;
    
}


bool AVGPlayer::handleEvent(AVGEvent * pEvent)
{
    m_pCurEvent = pEvent;
    switch (pEvent->getType()) {
        case AVGEvent::MOUSEMOTION:
        case AVGEvent::MOUSEBUTTONUP:
        case AVGEvent::MOUSEBUTTONDOWN:
            {
                AVGMouseEvent * pMouseEvent = dynamic_cast<AVGMouseEvent*>(pEvent);
                PLPoint pos(pMouseEvent->getXPosition(), pMouseEvent->getYPosition());
                AVGNode * pNode = m_pRootNode->getElementByPos(pos);            
                if (pNode != m_pLastMouseNode) {
                    if (pNode) {
                        createMouseOver(pMouseEvent, IAVGEvent::MOUSEOVER);
                    }
                    if (m_pLastMouseNode) {
                        createMouseOver(pMouseEvent, IAVGEvent::MOUSEOUT);
                    }

                    m_pLastMouseNode = pNode;

                }

                if (pNode) {
                    pNode->handleMouseEvent(pMouseEvent, m_pJSContext);
                }
            }
            break;
        case AVGEvent::KEYDOWN:
            {
                AVGKeyEvent * pKeyEvent = dynamic_cast<AVGKeyEvent*>(pEvent);
                if (pKeyEvent->getKeyCode() == 27) {
                    m_bStopping = true;
                }
            }
            break;
        case AVGEvent::QUIT:
            m_bStopping = true;
            break;
    }
    // Don't pass on any events.
    return true; 
}

void AVGPlayer::createMouseOver(AVGMouseEvent * pOtherEvent, int Type)
{
    AVGMouseEvent * pNewEvent = dynamic_cast<AVGMouseEvent *>
            (AVGEventDispatcher::createEvent("avgmouseevent"));
    pNewEvent->init(Type, 
            pOtherEvent->getLeftButtonState(),
            pOtherEvent->getMiddleButtonState(),
            pOtherEvent->getRightButtonState(),
            pOtherEvent->getXPosition(),
            pOtherEvent->getYPosition(),
            pOtherEvent->getButton());
    m_EventDispatcher.addEvent(pNewEvent);
}


int AVGPlayer::addTimeout(AVGTimeout* pTimeout)
{
    vector<AVGTimeout*>::iterator it=m_PendingTimeouts.begin();
    while (it != m_PendingTimeouts.end() && (**it)<*pTimeout) {
        it++;
    }
    m_PendingTimeouts.insert(it, pTimeout);
    return pTimeout->GetID();
}

void AVGPlayer::removeTimeout(AVGTimeout* pTimeout)
{
    delete pTimeout;
    vector<AVGTimeout*>::iterator it=m_PendingTimeouts.begin();
    while (*it != pTimeout) {
        it++;
    }
    m_PendingTimeouts.erase(it);
}


