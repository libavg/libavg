//
// $Id$
//

#include <avgconfig.h>
#include "AVGPlayer.h"
#include "AVGAVGNode.h"
#include "AVGDivNode.h"
#include "AVGImage.h"
#include "AVGPanoImage.h"
#include "AVGVideo.h"
#ifdef AVG_ENABLE_1394
#include "AVGCamera.h"
#endif
#include "AVGWords.h"
#include "AVGExcl.h"
#include "AVGEvent.h"
#include "AVGMouseEvent.h"
#include "AVGKeyEvent.h"
#include "AVGWindowEvent.h"
#include "AVGException.h"
#include "AVGRegion.h"
#ifdef AVG_ENABLE_DFB
#include "AVGDFBDisplayEngine.h"
#endif
#ifdef AVG_ENABLE_GL
#include "AVGSDLDisplayEngine.h"
#endif
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
#include <paintlib/planybmp.h>
#include <paintlib/plpngenc.h>

#include <libxml/xmlmemory.h>

#include <algorithm>
#include <iostream>
#include <sstream>

#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

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
              "Could not obtain reference to js context." << endl << 
              "Was xpshell used to start AVGPlayer?");
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
    AVG_TRACE(DEBUG_PROFILE, 
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
    if (!id) {
        AVG_TRACE(DEBUG_WARNING, "getElementByID() called with empty ID");
        return NS_OK; 
    }
    AVGNode * pNode = getElementByID(id);
    if (!pNode) {
        AVG_TRACE(DEBUG_WARNING, "getElementByID(" << id << ") failed");
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
AVGPlayer::Exec(const char *command, const char *input, 
        char **output, PRInt32 * pResult)
{
    // Set up arguments array
    const char ** argv;
    string sCmd;
    vector<string> sArgs;
    string sCmdLine(command);
    string::size_type pos = sCmdLine.find(" ");
    sCmd = sCmdLine.substr(0, pos);
    sArgs.push_back(sCmd);
    while (pos != string::npos) {
        sCmdLine.erase(0, pos+1);
        pos = sCmdLine.find(" ");
        sArgs.push_back(sCmdLine.substr(0, pos));
    }
    argv = new const char*[sArgs.size()+1];
    for (int i=0; i<sArgs.size(); i++) {
        argv[i] = sArgs[i].c_str();
    }
    argv[sArgs.size()] = 0;

    // Set up pipes for stdin and stdout of the child process.
    int stdout_fds[2];
    pipe (stdout_fds);
    int stdin_fds[2];
    pipe (stdin_fds);

    pid_t child_pid = fork();
    if (child_pid == 0) {
        close(stdout_fds[0]);
        dup2(stdout_fds[1], STDOUT_FILENO);
        close(stdin_fds[1]);
        dup2(stdin_fds[0], STDIN_FILENO);
        // Do the actual call.
        int err = execvp(sCmd.c_str(), (char * const *)argv);
        if (err) {
            AVG_TRACE(DEBUG_WARNING, "Could not exec '" << command << 
                    "'. Error was '" << strerror(errno) << "'");
            exit(-1);
        }
    }
    FILE * pStdin;
    close(stdin_fds[0]);
    pStdin = fdopen(stdin_fds[1], "w");
    fputs(input, pStdin);
    fflush(pStdin);
    close(stdin_fds[1]);

    FILE * pStdout;
    close(stdout_fds[1]);
    pStdout = fdopen(stdout_fds[0], "r");
    char * pOk = (char*)(1);
    string sOutput;
    while (!feof(pStdout) && !ferror(pStdout) && pOk) {
        char buffer[1024];
        pOk = fgets(buffer, sizeof(buffer), pStdout);
        if (pOk) {
            sOutput.append(buffer);
        }
    }
    *output = (char *) nsMemory::Clone(sOutput.c_str(), 
            sOutput.length()+1);
    
    int Status;
    waitpid (child_pid, &Status, 0);
    *pResult = WEXITSTATUS(Status);
    return NS_OK;
}

NS_IMETHODIMP
AVGPlayer::Screenshot(const char *pFilename, PRBool *pResult)
{
    PLAnyBmp Bmp;
    m_pDisplayEngine->screenshot(pFilename, Bmp);
    PLPNGEncoder Encoder;
    try {
        Encoder.MakeFileFromBmp(pFilename, &Bmp);
        AVG_TRACE(DEBUG_WARNING, "Saved screen as " << pFilename << ".");
    } catch (PLTextException& ex) {
        *pResult = false;
        AVG_TRACE(DEBUG_WARNING, "Could not save screenshot. Error: " 
                << ex << ".");
    }
    *pResult = true;
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
        AVG_TRACE(DEBUG_ERROR, "Could not open " << name 
                << " as log destination.");
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
        AVG_TRACE(DEBUG_ERROR, "CreateRelais failed: " << rv << ". Aborting.");
        exit(-1);
    }
    pRelais->init(port);
    m_pRelais.push_back(pRelais);
    *_retval = pRelais;
    return NS_OK;
}

NS_IMETHODIMP 
AVGPlayer::ShowCursor(PRBool bShow)
{
    m_pDisplayEngine->showCursor(bShow);
    return NS_OK;
}


void AVGPlayer::loadFile (const std::string& filename)
{
    jsgc();
    PLASSERT (!m_pRootNode);

    initConfig();
    // Get display configuration.
    if (!m_pDisplayEngine) {
        if (m_sDisplaySubsystem == "DFB") {
#ifdef AVG_ENABLE_DFB
            m_pDisplayEngine = new AVGDFBDisplayEngine ();
            m_pEventSource =
                    dynamic_cast<AVGDFBDisplayEngine *>(m_pDisplayEngine);
#else
            AVG_TRACE(DEBUG_ERROR,
                    "Display subsystem set to DFB but no DFB support compiled."
                    << " Aborting.");
            exit(-1);
#endif
        } else if (m_sDisplaySubsystem == "OGL") {
#ifdef AVG_ENABLE_GL
            m_pDisplayEngine = new AVGSDLDisplayEngine ();
            m_pEventSource = 
                    dynamic_cast<AVGSDLDisplayEngine *>(m_pDisplayEngine);
#else
            AVG_TRACE(DEBUG_ERROR,
                    "Display subsystem set to GL but no GL support compiled."
                    << " Aborting.");
            exit(-1);
#endif
        } else {
            AVG_TRACE(DEBUG_ERROR, 
                    "Display subsystem set to unknown value " << 
                    m_sDisplaySubsystem << ". Aborting.");
            exit(-1);
        }
    }

    // Find and parse dtd.
    string sDTDFName = getenv("AVG_INCLUDE_PATH");
    sDTDFName += "/avg.dtd";
    xmlDtdPtr dtd = xmlParseDTD(NULL, (const xmlChar*) sDTDFName.c_str());
    if (!dtd) {
        AVG_TRACE(DEBUG_ERROR, 
                "Required DTD not found at " << sDTDFName << ". Aborting.");
        exit(-1);
    }

    // Construct path.
    const char * pCurFilename;
    int lineno;
    getJSFileLine(m_pJSContext, pCurFilename, lineno);
    string Path = getPath(pCurFilename);
    string RealFilename = Path+filename;

    xmlPedanticParserDefault(1);
    xmlDoValidityCheckingDefaultValue =0;
    
    xmlDocPtr doc;
    doc = xmlParseFile(RealFilename.c_str());
    if (!doc) {
        throw (AVGException(AVG_ERR_XML_PARSE, 
                string("Error parsing xml document ")+RealFilename));
    }
    xmlValidCtxtPtr cvp = xmlNewValidCtxt();
    cvp->error = xmlParserValidityError;
    cvp->warning = xmlParserValidityWarning;
    int valid=xmlValidateDtd(cvp, doc, dtd);  
    xmlFreeValidCtxt(cvp);
    if (!valid) {
        AVG_TRACE(DEBUG_ERROR, 
                filename + " does not validate. Aborting.");
        exit(-1);
    }

    m_CurDirName = RealFilename.substr(0,RealFilename.rfind('/')+1);
    m_pRootNode = dynamic_cast<AVGAVGNode*>
            (createNodeFromXml(xmlDocGetRootElement(doc), 0));
    AVGDRect rc = m_pRootNode->getRelViewport();
    xmlFreeDoc(doc);
}

void AVGPlayer::play (double framerate)
{
    try {
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

void AVGPlayer::readConfigFile(const string& sFName) {
    xmlDocPtr doc;
    doc = xmlParseFile(sFName.c_str());
    if (doc) {
        xmlNodePtr pRoot = xmlDocGetRootElement(doc);
        if (xmlStrcmp(pRoot->name, (const xmlChar *)"avgrc")) {
            AVG_TRACE(DEBUG_ERROR, 
                    "/etc/avgrc: Root node must be <avgrc>. Aborting.");
            exit(-1);
        }
        xmlNodePtr pChild = pRoot->xmlChildrenNode;
        while (pChild) {
            if (!xmlStrcmp(pChild->name, (const xmlChar *)"font")) {
                m_sFontDir=getRequiredStringAttr(pChild, "dir");
            } else if (!xmlStrcmp(pChild->name, (const xmlChar *)"display")) {
#ifdef AVG_ENABLE_GL
                m_sDisplaySubsystem=getDefaultedStringAttr(pChild, 
                        "subsystem", "OGL");
#else
                m_sDisplaySubsystem=getDefaultedStringAttr(pChild, 
                        "subsystem", "DFB");
#endif
                m_bFullscreen=getDefaultedBoolAttr(pChild, 
                        "fullscreen", false);
                m_BPP=int(getDefaultedDoubleAttr(pChild, "bpp", 24));
            } else if (xmlStrcmp(pChild->name, (const xmlChar *)"text") &&
                       xmlStrcmp(pChild->name, (const xmlChar *)"comment")) 
            {
                AVG_TRACE(DEBUG_ERROR, 
                    "/etc/avgrc: Unknown node " << pChild->name << ". Aborting.");
                exit(-1);
            }
            pChild = pChild->next;
        }
    }
    
}

void AVGPlayer::initConfig() {
    // Set defaults.
    m_sFontDir = "";
    m_sDisplaySubsystem = "OGL";
    m_bFullscreen = false;
    m_BPP = 24;
   
    // Get data from config files.
    xmlPedanticParserDefault(1);
    xmlDoValidityCheckingDefaultValue =0;
    readConfigFile("/etc/avgrc");
    char * pHomeDir = getenv("HOME");
    if (pHomeDir) {
        string sFName = pHomeDir;
        sFName += "/avgrc";
        readConfigFile(sFName.c_str());
    }

    // Get command line data through environment variables.
    char * pFontDir = getenv("AVG_FONT_PATH");
    if (pFontDir) {
        m_sFontDir = pFontDir;
    }
    
    char * pszDisplay = getenv("AVG_DISPLAY");
    if (pszDisplay) {
        m_sDisplaySubsystem = pszDisplay;
    }
    char * pszBPP = getenv("AVG_BPP");
    if (pszBPP) {
        if (strcmp(pszBPP, "15") == 0) {
            m_BPP = 15;
        } else if (strcmp(pszBPP, "16") == 0) {
            m_BPP = 16;
        } else if (strcmp(pszBPP, "24") == 0) {
            m_BPP = 24;
        } else if (strcmp(pszBPP, "32") == 0) {
            m_BPP = 32;
        } else {
            AVG_TRACE(DEBUG_ERROR, 
                    "BPP must be 15, 16, 24 or 32. Current value is " 
                    << m_BPP << ". Aborting." );
            exit(-1);
        }
    }
    char * pszFullscreen = getenv("AVG_FULLSCREEN");
    if (pszFullscreen) {
        m_bFullscreen = (!strcmp(pszFullscreen, "true"));
    }

    FILE * pFile = fopen (m_sFontDir.c_str(), "r");
    fclose (pFile);
    if (!pFile) {
        AVG_TRACE(DEBUG_WARNING, "Font directory " << m_sFontDir << 
                " could not be opened (" <<
                strerror(errno) << ")."); 
    } 

    AVG_TRACE(DEBUG_CONFIG, "Font directory: " << m_sFontDir);
    AVG_TRACE(DEBUG_CONFIG, "Display subsystem: " << 
            m_sDisplaySubsystem?"true":"false");
    AVG_TRACE(DEBUG_CONFIG, "Display bpp: " << m_BPP);
    AVG_TRACE(DEBUG_CONFIG, "Display fullscreen: " << m_bFullscreen);
}

AVGNode * AVGPlayer::createNodeFromXml (const xmlNodePtr xmlNode, 
        AVGContainer * pParent)
{
    const char * nodeType = (const char *)xmlNode->name;
    AVGNode * curNode = 0;
    string id = getDefaultedStringAttr (xmlNode, "id", "");
    if (!strcmp (nodeType, "avg")) {
        AVGAVGNode * rootNode = AVGAVGNode::create();
        curNode = rootNode;
        curNode->init(id, m_pDisplayEngine, 0, this);
        initVisible(xmlNode, curNode);
        string sKeyDownHandler = getDefaultedStringAttr(xmlNode, "onkeydown", "");
        string sKeyUpHandler = getDefaultedStringAttr(xmlNode, "onkeyup", "");
        rootNode->initKeyEventHandlers(sKeyDownHandler, sKeyUpHandler);
        initDisplay(rootNode);
    } else if (!strcmp (nodeType, "div")) {
        curNode = AVGDivNode::create();
        curNode->init(id, m_pDisplayEngine, pParent, this);
        initVisible(xmlNode, curNode);
    } else if (!strcmp (nodeType, "image")) {
        string filename = initFileName(xmlNode);
        AVGImage * pImage = AVGImage::create();
        curNode = pImage;
        int bpp = getDefaultedIntAttr(xmlNode, "bpp", 32);
        pImage->init(id, filename, bpp, m_pDisplayEngine, pParent, this);
        initVisible(xmlNode, pImage);
    } else if (!strcmp (nodeType, "panoimage")) {
        string filename = initFileName(xmlNode);
        AVGPanoImage * pPanoImage = AVGPanoImage::create();
        curNode = pPanoImage;
        double SensorHeight = getDefaultedDoubleAttr(xmlNode, 
                "sensorheight", 0);
        double SensorWidth = getDefaultedDoubleAttr(xmlNode, 
                "sensorwidth", 0);
        double FocalLength = getDefaultedDoubleAttr(xmlNode, 
                "focallength", 0);
        pPanoImage->init(id, filename, SensorWidth, SensorHeight, FocalLength, 
                m_pDisplayEngine, pParent, this);
        initVisible(xmlNode, pPanoImage);
    } else if (!strcmp (nodeType, "video")) {
        string filename = initFileName(xmlNode);
        bool bLoop = getDefaultedBoolAttr(xmlNode, "loop", false); 
        bool bOverlay = getDefaultedBoolAttr(xmlNode,"overlay", false); 
        AVGVideo * pVideo = AVGVideo::create();
        curNode = pVideo;
        pVideo->init(id, filename, bLoop, bOverlay, m_pDisplayEngine, 
                pParent, this);
        initVisible(xmlNode, pVideo);
    } else if (!strcmp (nodeType, "camera")) {
        curNode = createCameraNode(xmlNode, pParent, id);
    } else if (!strcmp (nodeType, "words")) {
        string font = getDefaultedStringAttr(xmlNode, "font", "arial");
        string str = getRequiredStringAttr(xmlNode, "text");
        string color = getDefaultedStringAttr(xmlNode, "color", "FFFFFF");
        int size = getDefaultedIntAttr(xmlNode, "size", 15);
        AVGWords * pWords = AVGWords::create();
        curNode = pWords;
        pWords->init(id, size, font, str, color, m_pDisplayEngine, pParent, this);
        initVisible(xmlNode, pWords);
    } else if (!strcmp (nodeType, "excl")) {
        string id  = getDefaultedStringAttr (xmlNode, "id", "");
        curNode = AVGExcl::create();
        curNode->init(id, m_pDisplayEngine, pParent, this);
        curNode->initVisible(0,0,1,10000,10000,1,0,0,0);
        initEventHandlers(curNode, xmlNode);
    } else if (!strcmp (nodeType, "text") || 
               !strcmp (nodeType, "comment")) {
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

AVGNode * AVGPlayer::createCameraNode(const xmlNodePtr xmlNode, 
                AVGContainer * pParent, const string& id)
{
#ifdef AVG_ENABLE_1394
    string sDevice = getDefaultedStringAttr(xmlNode, "device", "0");
    bool bOverlay = getDefaultedBoolAttr(xmlNode, "overlay", false);
    string sMode = getDefaultedStringAttr(xmlNode, "mode", "640x480_RGB");
    int Framerate = getDefaultedIntAttr(xmlNode, "framerate", 15);

    AVGCamera * pCamera = AVGCamera::create();
    pCamera->init(id, sDevice, Framerate, sMode, bOverlay, 
            m_pDisplayEngine, pParent, this);
    initVisible(xmlNode, pCamera);
    setCamFeatureFromXml(pCamera, xmlNode, "brightness", -1);
    setCamFeatureFromXml(pCamera, xmlNode, "exposure", -1);
    setCamFeatureFromXml(pCamera, xmlNode, "sharpness", 60);
    setCamFeatureFromXml(pCamera, xmlNode, "saturation", 75);
    setCamFeatureFromXml(pCamera, xmlNode, "gamma", 1);
    setCamFeatureFromXml(pCamera, xmlNode, "shutter", 2);
    setCamFeatureFromXml(pCamera, xmlNode, "gain", 75);

    return pCamera;
#else
    throw (AVGException (AVG_ERR_UNSUPPORTED, 
            string("Camera node encountered but avg not compiled for camera support.")));
#endif
}

void AVGPlayer::setCamFeatureFromXml(AVGCamera * pCamera, 
        const xmlNodePtr xmlNode, const string& sName, int Default)
{
    unsigned int Value = getDefaultedIntAttr(xmlNode, sName.c_str(), Default);
    pCamera->setFeature(sName, Value);
}

string AVGPlayer::initFileName(const xmlNodePtr xmlNode) {
    string origFName = getRequiredStringAttr(xmlNode, "href");
    if (origFName[0] != '/') {
        return m_CurDirName + origFName;
    } else {
        return origFName;
    }
}

void AVGPlayer::initDisplay(AVGAVGNode * pNode) {
    m_pDisplayEngine->init(int(pNode->getRelViewport().Width()), 
            int(pNode->getRelViewport().Height()), m_bFullscreen, m_BPP,
            m_sFontDir);
}

void AVGPlayer::initVisible (const xmlNodePtr xmlNode, 
        AVGNode * pNode)
{
    int x = getDefaultedIntAttr (xmlNode, "x", 0);
    int y = getDefaultedIntAttr (xmlNode, "y", 0);
    int z = getDefaultedIntAttr (xmlNode, "z", 0);
    int width = getDefaultedIntAttr (xmlNode, "width", 0);
    int height = getDefaultedIntAttr (xmlNode, "height", 0);
    double opacity = getDefaultedDoubleAttr (xmlNode, "opacity", 1.0);
    double angle = getDefaultedDoubleAttr (xmlNode, "angle", 0.0);
    int pivotx = getDefaultedIntAttr (xmlNode, "pivotx", -32767);
    int pivoty = getDefaultedIntAttr (xmlNode, "pivoty", -32767);

    pNode->initVisible(x, y, z, width, height, opacity, angle, 
                pivotx, pivoty);
    initEventHandlers(pNode, xmlNode);
}

void AVGPlayer::initEventHandlers (AVGNode * pAVGNode, const xmlNodePtr xmlNode)
{
    string MouseMoveHandler = getDefaultedStringAttr 
            (xmlNode, "onmousemove", "");
    string MouseButtonUpHandler = getDefaultedStringAttr 
            (xmlNode, "onmouseup", "");
    string MouseButtonDownHandler = getDefaultedStringAttr 
            (xmlNode, "onmousedown", "");
    string MouseOverHandler = getDefaultedStringAttr 
            (xmlNode, "onmouseover", "");
    string MouseOutHandler = getDefaultedStringAttr 
            (xmlNode, "onmouseout", "");
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
                AVGDPoint pos(pMouseEvent->getXPosition(), 
                        pMouseEvent->getYPosition());
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
        case AVGEvent::KEYUP:
            {
                AVGKeyEvent * pKeyEvent = dynamic_cast<AVGKeyEvent*>(pEvent);
                m_pRootNode->handleKeyEvent(pKeyEvent, m_pJSContext);
                if (pEvent->getType() == AVGEvent::KEYDOWN &&
                    pKeyEvent->getKeyCode() == 27) 
                {
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


