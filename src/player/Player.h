//
// $Id$
//

#ifndef _Player_H_
#define _Player_H_

#include "IEventSink.h"
#include "EventDispatcher.h"
#include "DebugEventSink.h"
#include "Timeout.h"


#include "../base/IFrameListener.h"

#include <libxml/parser.h>

#include <map>
#include <string>
#include <vector>

class PLPoint;

namespace avg {

class AVGNode;
class Node;
class Container;
class Event;
class MouseEvent;
class ConradRelais;
class IDisplayEngine;
class Camera;
class FramerateManager;

class Player : IEventSink
{
    public:
        Player ();
        virtual ~Player ();

        /**
         * Loads the avg file specified in fileName. Returns false if the file 
         * could not be opened.
         */
        void loadFile (const std::string& fileName);
        
        /**
         * Opens a playback window or screen and starts playback. framerate is 
         * the number of frames per second that should be displayed. 
         */
        void play (double framerate);
        
        /**
         * Stops playback and resets the video mode if nessesary.
         */
        void stop ();

        Node * createNodeFromXml (const std::string& sXML);

        /**
         * Sets javascript code that should be executed every time milliseconds.
         * The smallest timeframe that can be set is once per frame. If less
         * than that is specified, the code will be executed exactly once per 
         * frame. The function returns an id that can be used to call 
         * clearInterval() to stop the code from being called.
         */
        int setInterval(int time, PyObject * pyfunc);
        /**
         * Sets javascript code that should be executed after time milliseconds.
         * The function returns an id that can be used to call clearInterval() 
         * to stop the code from being called.
         */
        int setTimeout(int time, PyObject * pyfunc);
        /**
         * Stops a timeout or an interval from being called. Returns true if 
         * there was an interval with the given id, false if not.
         */
        bool clearInterval(int id);

        /**
         * Saves the contents of the current screen in a png file. Returns 
         * true on success, false if the screen couldn't be saved.
         */
        bool screenshot(const std::string& sFilename);

        /**
         * Shows or hides the mouse cursor. (Currently, this only works for 
         * OpenGL. Showing the DirectFB mouse cursor seems to expose some 
         * issue with DirectFB.)
         */
        void showCursor(bool bShow);

        /**
         * Returns an element in the avg tree. The id corresponds to the id 
         * attribute of the node. 
         */
        Node * getElementByID (const std::string& id);
        /**
         * Returns the outermost element in the avg tree. 
         */
        AVGNode * getRootNode ();
        void doFrame ();
        void setPriority();
        double getFramerate ();
        virtual bool handleEvent(Event * pEvent);

        void registerFrameListener(IFrameListener* pListener);
        std::string getCurDirName();
        void initNode(Node * pNode, Container * pParent);

    private:
        void initConfig();

        Node * createNodeFromXml(const xmlDocPtr xmlDoc, 
                const xmlNodePtr xmlNode, Container * pParent);

        void initDisplay(const xmlNodePtr xmlNode);
        void render (bool bRenderEverything);
        void teardownDFB();
        void createMouseOver(MouseEvent * pOtherEvent, int Type);
	
        AVGNode * m_pRootNode;
        IDisplayEngine * m_pDisplayEngine;
        IEventSource * m_pEventSource;

        std::string m_CurDirName;
        FramerateManager * m_pFramerateManager;
        bool m_bStopping;
        typedef std::map<std::string, Node*> NodeIDMap;
        NodeIDMap m_IDMap;

        int addTimeout(Timeout* pTimeout);
        void removeTimeout(Timeout* pTimeout);
        void handleTimers();
        bool m_bInHandleTimers;

        std::vector<Timeout *> m_PendingTimeouts;
        std::vector<Timeout *> m_NewTimeouts; // Timeouts to be added this frame.
        std::vector<int> m_KilledTimeouts; // Timeouts to be deleted this frame.

        EventDispatcher m_EventDispatcher;
        DebugEventSink  m_EventDumper;
        Event * m_pCurEvent;
        Node * m_pLastMouseNode;

        // Configuration variables.
        std::string m_sDisplaySubsystem;
        bool m_bFullscreen;
        int m_BPP;
        int m_WindowWidth;
        int m_WindowHeight;

        std::vector<IFrameListener*> m_Listeners;
};

}
#endif //_Player_H_
