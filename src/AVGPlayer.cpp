//
// $Id$
//

#include "AVGPlayer.h"
#include "AVGAVGNode.h"
#include "AVGImage.h"
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

#include <stdlib.h>
#include <stdio.h>
#include "nsMemory.h"

#ifdef XPCOM_GLUE
#include "nsXPCOMGlue.h"
#endif

//#include "xpconnect/nsIXPConnect.h"
//#include "xpcom/nsIServiceManager.h"

using namespace std;

AVGPlayer::AVGPlayer()
    : m_pRootNode (0),
      m_pDisplayEngine(0),
      m_FramerateManager(),
      m_pCurEvent(0),
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
    
    delete m_pRootNode;
    m_pRootNode = 0;

    m_pLastMouseNode = 0;
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
    return 0;
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
//    cerr << "Node: " << (const char *) nodeType << endl;
    if (!xmlStrcmp (nodeType, (const xmlChar *)"avg")) {
        // create node itself.
        string id;
        int x,y,z;
        int width, height;
        double opacity;
        getVisibleNodeAttrs(xmlNode, &id, &x, &y, &z, &width, &height, &opacity);
        curNode = new AVGAVGNode (id, x, y, z, width, height, opacity, 
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
        curNode = new AVGImage (id, x, y, z, width, height, opacity, filename,
                m_pDisplayEngine, pParent);
        initEventHandlers(curNode, xmlNode);
    } else if (!xmlStrcmp (nodeType, (const xmlChar *)"text")) {
        // Ignore whitespace
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
    while(SDL_PollEvent(&SDLEvent) && !m_bStopping){
        AVGEvent* pEvent = 0;

        switch(SDLEvent.type){
            case SDL_KEYDOWN:
                pEvent = new AVGEvent();
                pEvent->init(SDLEvent);
                break;
            case SDL_MOUSEMOTION:
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                pEvent = new AVGEvent();
                pEvent->init(SDLEvent);
                handleMouseEvent(pEvent);
                break;
            case SDL_QUIT:
                pEvent = new AVGEvent();
                pEvent->init(SDLEvent);
                m_bStopping = true;
                break;
            default:
                break;
        }
        if (pEvent) {
            dumpEvent(pEvent);
            delete pEvent;
        }
    }
}

/* bullshit
nsCOMPtr<IAVGEvent> AVGPlayer::wrapJSEvent(AVGEvent* pEvent)
{
    nsresult rv;
    nsCOMPtr<nsIXPConnect> xpc(do_GetService(nsIXPConnect::GetCID(), &rv));
    PLASSERT(!NS_FAILED(rv));

    nsCOMPtr<nsIXPCNativeCallContext> callContext;
    xpc->GetCurrentNativeCallContext(getter_AddRefs(callContext));

    JSContext* cx;
    rv = callContext->GetJSContext(&cx);
    PLASSERT(!NS_FAILED(rv));
    PLASSERT(cx);

    nsCOMPtr<nsIXPConnectWrappedNative> calleeWrapper;
    callContext->GetCalleeWrapper(getter_AddRefs(calleeWrapper));
    PLASSERT(calleeWrapper);

    JSObject* calleeJSObject;
    rv = calleeWrapper->GetJSObject(&calleeJSObject);
    PLASSERT(!NS_FAILED(rv));
    PLASSERT(calleeJSObject);

    nsCOMPtr<IAVGEvent> JSEvent;
    rv = xpc->WrapNative(cx, calleeJSObject, pEvent, NS_GET_IID(AVGEvent),
            getter_AddRefs(JSEvent));
    PLASSERT(!NS_FAILED(rv));
    PLASSERT(JSEvent);

    return JSEvent;
}
*/

void AVGPlayer::dumpEvent(AVGEvent* pEvent)
{
    string EventName;
    int EventType;
    pEvent->GetType(&EventType);
    switch(EventType) {
        case AVGEvent::KEYDOWN:
            EventName = "KEYDOWN";
            break;
        case AVGEvent::MOUSEDOWN:
            EventName =  "MOUSEBUTTONDOWN";
            break;
        case AVGEvent::MOUSEUP:
            EventName =  "MOUSEBUTTONUP";
            break;
        case AVGEvent::QUIT:
            EventName = "QUIT";
            break;
        case AVGEvent::MOUSEMOVE: // Mousemotion events aren't dumped.
        default: 
            return;
    }
    dumpEventStr(EventName, 
        EventType == AVGEvent::MOUSEUP || EventType == AVGEvent::MOUSEDOWN);
}

void AVGPlayer::dumpEventStr(const string& EventName, bool IsMouse)
{
    switch (m_EventDebugLevel) {
        case 0:
            return;
        case 1:
            cerr << "Event: " << EventName << endl;
            return;
        case 2:
            if (IsMouse) {
                if (m_pLastMouseNode) {
                    cerr << "Event: " << EventName << "(Node id: " << 
                            m_pLastMouseNode->getID() << ")" << endl;
                }
            } else {
                cerr << "Event: " << EventName << endl;
            }
    }
}

void AVGPlayer::handleMouseEvent (AVGEvent* pEvent)
{
    PLPoint pos;
    pEvent->GetXPos(&pos.x);
    pEvent->GetYPos(&pos.y);
    AVGNode * pNode = m_pRootNode->getElementByPos(pos);
    if (pNode != m_pLastMouseNode) {
        if (pNode) {
            dumpEventStr("MOUSEOVER", true);
            pNode->onMouseOver(m_pKruecke);
        }
        if (m_pLastMouseNode) {
            dumpEventStr("MOUSEOUT", true);
            m_pLastMouseNode->onMouseOut(m_pKruecke);
        }
        m_pLastMouseNode = pNode;
    }
    if (pNode) {
        pNode->handleEvent(pEvent, m_pKruecke);
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

