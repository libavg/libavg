//
// $Id$
//

#ifndef _AVGPlayer_H_
#define _AVGPlayer_H_

#include "IAVGPlayer.h"
#include "IAVGEvent.h"
#include "AVGFramerateManager.h"
#include "AVGTimeout.h"
#include "IAVGEventSink.h"
#include "AVGEventDispatcher.h"
#include "AVGDebugEventSink.h"

#ifdef AVG_ENABLE_DFB
#include <directfb/directfb.h>
#endif
#include <libxml/parser.h>

#include <xpcom/nsCOMPtr.h>
#include <jsapi.h>

#include <map>
#include <string>
#include <vector>
#include <fstream>

class AVGAVGNode;
class AVGNode;
class AVGContainer;
class AVGEvent;
class AVGMouseEvent;
class AVGConradRelais;
class IAVGDisplayEngine;
class AVGCamera;

class PLPoint;

//fe906737-5231-4a55-a9da-d348a7da581f
#define AVGPLAYER_CID \
{ 0xfe906737, 0x5231, 0x4a55, { 0xa9, 0xda, 0xd3, 0x48, 0xa7, 0xda, 0x58, 0x1f } }

#define AVGPLAYER_CONTRACTID "@c-base.org/avgplayer;1"

class AVGPlayer: public IAVGPlayer, IAVGEventSink
{
    public:
        AVGPlayer ();
        virtual ~AVGPlayer ();

        NS_DECL_ISUPPORTS

        NS_DECL_IAVGPLAYER

        void loadFile (const std::string& fileName);
        void play (double framerate);
        void stop ();
        void doFrame ();

        AVGNode * getElementByID (const std::string id);
        AVGAVGNode * getRootNode ();
        double getFramerate ();
        void addEvent (int time, AVGEvent * event);
        virtual bool handleEvent(AVGEvent * pEvent);

    private:
        void readConfigFile(const std::string& sFName);
        void initConfig();
        AVGNode * createNodeFromXml(const xmlNodePtr xmlNode, 
                AVGContainer * pParent);
        AVGNode * createCameraNode(const xmlNodePtr xmlNode, 
                AVGContainer * pParent, const std::string& id);
        void setCamFeatureFromXml(AVGCamera * pCamera, 
                const xmlNodePtr xmlNode, const std::string& sName, 
                int Default);
        std::string initFileName(const xmlNodePtr xmlNode);
        void initVisible (const xmlNodePtr xmlNode, 
                AVGNode * pNode);
        void initEventHandlers (AVGNode * pAVGNode, const xmlNodePtr xmlNode);
        void initDisplay(AVGAVGNode * pNode);
        void render (bool bRenderEverything);
        void jsgc();
        void teardownDFB();
        void setRealtimePriority();
        void createMouseOver(AVGMouseEvent * pOtherEvent, int Type);
	
        AVGAVGNode * m_pRootNode;
        IAVGDisplayEngine * m_pDisplayEngine;
        IAVGEventSource * m_pEventSource;

        std::string m_CurDirName;
        AVGFramerateManager * m_pFramerateManager;
        bool m_bStopping;
        typedef std::map<std::string, AVGNode*> NodeIDMap;
        NodeIDMap m_IDMap;
        std::ofstream * m_pDebugDest;

        int addTimeout(AVGTimeout* pTimeout);
        void removeTimeout(AVGTimeout* pTimeout);
        void handleTimers();
        bool m_bInHandleTimers;

        std::vector<AVGTimeout *> m_PendingTimeouts;
        std::vector<AVGTimeout *> m_NewTimeouts; // Timeouts to be added this frame.
        std::vector<AVGTimeout *> m_KilledTimeouts; // Timeouts to be deleted this frame.

        AVGEventDispatcher m_EventDispatcher;
        AVGDebugEventSink  m_EventDumper;
        AVGEvent * m_pCurEvent;
        AVGNode * m_pLastMouseNode;

        JSContext * m_pJSContext;
        std::vector<AVGConradRelais*> m_pRelais;

        // Configuration variables.
        std::string m_sFontDir;
        std::string m_sDisplaySubsystem;
        bool m_bFullscreen;
        int m_BPP;
};


#endif //_AVGPlayer_H_
