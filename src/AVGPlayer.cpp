//
// $Id$
//

#include "AVGPlayer.h"
#include "AVGAVGNode.h"
#include "AVGImage.h"
#include "AVGExcl.h"
#include "AVGEvent.h"
#include "AVGException.h"
#include "AVGSDLDisplayEngine.h"
#include "IJSEvalKruecke.h"
#include "XMLHelper.h"

#include <paintlib/plstdpch.h>
#include <paintlib/plexcept.h>
#include <paintlib/pldebug.h>
#include <paintlib/plpoint.h>

#include <libxml/xmlmemory.h>

#include <algorithm>
#include <iostream>

#include <stdlib.h>
#include <stdio.h>
#include "nsMemory.h"

#ifdef XPCOM_GLUE
#include "nsXPCOMGlue.h"
#endif

#include <xpcom/nsComponentManagerUtils.h>

using namespace std;

AVGPlayer::AVGPlayer()
    : m_pRootNode (0),
      m_pDisplayEngine(0),
      m_FramerateManager(),
      m_pLastMouseNode(0),
      m_EventDebugLevel(0)
{
#ifdef XPCOM_GLUE
    XPCOMGlueStartup("XPCOMComponentGlue");
#endif
    NS_INIT_ISUPPORTS();
}

AVGPlayer::~AVGPlayer()
{
#ifdef XPCOM_GLUE
    XPCOMGlueShutdown();
#endif

}

NS_IMPL_ISUPPORTS1_CI(AVGPlayer, IAVGPlayer);

NS_IMETHODIMP
AVGPlayer::LoadFile(const char * fileName, IJSEvalKruecke* pKruecke, 
        PRBool * pResult)
{
    m_pKruecke = pKruecke;
    m_pKruecke->AddRef();
	try {
		loadFile (fileName);
		*pResult = true;
    } catch  (AVGException& ex) {
        cout <<  endl << "    ----------- error: " << ex.GetStr() 
             << "-----------" << endl;
		*pResult = false;
    } catch (PLTextException& ex) {
        cout << endl << "    ----------- error: " << (const char *)ex
             << "-----------" << endl;
		*pResult = false;
	}
	return NS_OK;
}

NS_IMETHODIMP
AVGPlayer::Play()
{
	play();
	return NS_OK;
}

NS_IMETHODIMP
AVGPlayer::Stop()
{
	stop();
    m_pKruecke->Release();
	return NS_OK;
}

NS_IMETHODIMP 
AVGPlayer::GetElementByID(const char *id, IAVGNode **_retval)
{
    AVGNode * pNode = getElementByID(id);
    if (!pNode) {
        cerr << "getElementByID(" << id << ") failed" << endl;
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
    *pResult = addTimeout(AVGTimeout(time, code, true));
	return NS_OK;
}

NS_IMETHODIMP
AVGPlayer::SetTimeout(PRInt32 time, const char * code, PRInt32 * pResult)
{
    *pResult = addTimeout(AVGTimeout(time, code, false));
	return NS_OK;
}

NS_IMETHODIMP
AVGPlayer::ClearInterval(PRInt32 id, PRBool * pResult)
{
    vector<AVGTimeout>::iterator it;
    for (it=m_PendingTimeouts.begin(); it!=m_PendingTimeouts.end(); it++) {
        if (id == (*it).GetID()) {
            m_PendingTimeouts.erase(it);
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
    NS_IF_ADDREF(m_CurEvent);
    *_retval = m_CurEvent;
    return NS_OK;
}

NS_IMETHODIMP 
AVGPlayer::SetEventDebugLevel(PRInt32 level)
{
    m_EventDebugLevel = level;
    return NS_OK;
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

void AVGPlayer::loadFile (const std::string& filename)
{
    PLASSERT (!m_pRootNode);
    m_pDisplayEngine = new AVGSDLDisplayEngine ();

    xmlPedanticParserDefault(1);
    xmlDoValidityCheckingDefaultValue =1;
    
    xmlDocPtr doc;
    doc = xmlParseFile(filename.c_str());
    if (!doc) {
        throw (AVGException(AVG_ERR_XML_PARSE, 
                string("Error parsing xml document ")+filename));
    }
    m_CurDirName = filename.substr(0,filename.rfind('/')+1);

    m_pRootNode = dynamic_cast<AVGAVGNode*>
            (createNodeFromXml(xmlDocGetRootElement(doc), 0));
    PLRect rc = m_pRootNode->getRelViewport();
    m_pDisplayEngine->init(rc.Width(), rc.Height(), m_IsFullscreen);
}

void AVGPlayer::play ()
{
    PLASSERT (m_pRootNode);
    m_FramerateManager.SetRate(30);
    m_bStopping = false;
    while (!m_bStopping) {
        doFrame();
    }
    m_PendingTimeouts.clear();

    m_pDisplayEngine->teardown();
    delete m_pDisplayEngine;
    m_pDisplayEngine = 0;
    NS_IF_RELEASE(m_pRootNode);
    m_pRootNode = 0;

    m_pLastMouseNode = 0;
    m_IDMap.clear();
}

void AVGPlayer::stop ()
{
    m_bStopping = true;
}

void AVGPlayer::doFrame ()
{
    handleTimers();
    handleEvents();
    if (!m_bStopping) {
        m_pRootNode->update(0, PLPoint(0,0));
        m_pRootNode->render();
        m_FramerateManager.FrameWait();
        m_pDisplayEngine->swapBuffers();
    }
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

void AVGPlayer::addEvent (int time, AVGEvent * event)
{
    
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
        dynamic_cast<AVGAVGNode*>(curNode)->init(id, x, y, z, width, height, opacity, 
                    m_pDisplayEngine, pParent);
        initEventHandlers(curNode, xmlNode);
        // Fullscreen handling only for topmost node.
        if (!pParent) {
            m_IsFullscreen = getDefaultedBoolAttr 
                        (xmlNode, (const xmlChar *)"fullscreen", false);
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
                filename, m_pDisplayEngine, pParent);
        initEventHandlers(curNode, xmlNode);
    } else if (!xmlStrcmp (nodeType, (const xmlChar *)"excl")) {
        string id  = getDefaultedStringAttr (xmlNode, (const xmlChar *)"id", "");
        curNode = AVGExcl::create();
        dynamic_cast<AVGExcl*>(curNode)->init(id, pParent);
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
    vector<AVGTimeout>::iterator it;
    it = m_PendingTimeouts.begin();
    while (it != m_PendingTimeouts.end() && it->IsReady() && !m_bStopping)
    {
        it->Fire(m_pKruecke);
        if (!it->IsInterval()) {
            it = m_PendingTimeouts.erase(it);
        } else {
            AVGTimeout tempTimeout = *it;
            it = m_PendingTimeouts.erase(it);
            addTimeout(tempTimeout);
        }
    }
}

void AVGPlayer::handleEvents()
{
    SDL_Event SDLEvent; 
    while(SDL_PollEvent(&SDLEvent) && !m_bStopping) {
        if (SDLEvent.type == SDL_KEYDOWN || SDLEvent.type == SDL_MOUSEMOTION ||
            SDLEvent.type == SDL_MOUSEBUTTONUP || SDLEvent.type == SDL_MOUSEBUTTONDOWN ||
            SDLEvent.type == SDL_QUIT)
        {
            AVGEvent * pCPPEvent = createCurEvent();
            pCPPEvent->init(SDLEvent);
            pCPPEvent->dump(m_EventDebugLevel);
            int EventType;
            m_CurEvent->GetType(&EventType);
            if (EventType == AVGEvent::QUIT) {
                m_bStopping = true;
            }

            int b;
            m_CurEvent->IsMouseEvent(&b);
            if (b) {
                handleMouseEvent(pCPPEvent);
            }
            NS_IF_RELEASE(pCPPEvent);
        }
    }
}

AVGEvent* AVGPlayer::createCurEvent()
{
    nsresult rv;
    nsCOMPtr<IAVGEvent> pXPEvent = do_CreateInstance("@c-base.org/avgevent;1", &rv);
    PLASSERT(!NS_FAILED(rv));
    m_CurEvent = pXPEvent;
    NS_IF_ADDREF((IAVGEvent*)m_CurEvent);
    return dynamic_cast<AVGEvent*>(m_CurEvent);
}

void AVGPlayer::handleMouseEvent (AVGEvent* pEvent)
{
    PLPoint pos;
    pEvent->GetXPos(&pos.x);
    pEvent->GetYPos(&pos.y);
    int ButtonState;
    pEvent->GetMouseButtonState(&ButtonState);
    AVGNode * pNode = m_pRootNode->getElementByPos(pos);
    if (pNode) {
        pNode->handleEvent(pEvent, m_pKruecke);
    }
    if (pNode != m_pLastMouseNode) {
        if (pNode) {
            AVGEvent * pEvent = createCurEvent();
            pEvent->init(AVGEvent::MOUSEOVER, pos, ButtonState);
            pEvent->dump(m_EventDebugLevel);
            pNode->handleEvent(pEvent, m_pKruecke);
        }
        if (m_pLastMouseNode) {
            AVGEvent * pEvent = createCurEvent();
            pEvent->init(AVGEvent::MOUSEOUT, pos, ButtonState);
            pEvent->dump(m_EventDebugLevel);
            m_pLastMouseNode->handleEvent(pEvent, m_pKruecke);
        }

        m_pLastMouseNode = pNode;
    }
}

int AVGPlayer::addTimeout(const AVGTimeout& timeout)
{
    vector<AVGTimeout>::iterator it;
    it = lower_bound(m_PendingTimeouts.begin(), m_PendingTimeouts.end(),
            timeout);
    m_PendingTimeouts.insert(it, timeout);
    return timeout.GetID();
}

