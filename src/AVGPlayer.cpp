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


using namespace std;

AVGPlayer::AVGPlayer()
    : m_pRootNode (0),
      m_pDisplayEngine(0),
      m_FramerateManager()
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
    delete m_pDisplayEngine;
    m_pDisplayEngine = 0;
    delete m_pRootNode;
    m_pRootNode = 0;
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
    m_pDisplayEngine->teardown();
    m_PendingTimeouts.clear();
}

void AVGPlayer::stop ()
{
    m_bStopping = true;
}

void AVGPlayer::doFrame ()
{
    handleTimers();
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

int AVGPlayer::addTimeout(const AVGTimeout& timeout)
{
    vector<AVGTimeout>::iterator it;
    it = lower_bound(m_PendingTimeouts.begin(), m_PendingTimeouts.end(),
            timeout);
    m_PendingTimeouts.insert(it, timeout);
    return timeout.GetID();
}

