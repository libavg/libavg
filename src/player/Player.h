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

#ifndef _Player_H_
#define _Player_H_

#include "IEventSink.h"
#include "EventDispatcher.h"
#include "DebugEventSink.h"
#include "Timeout.h"
#include "KeyEvent.h"
#include "MouseEvent.h"


#include "../base/IFrameListener.h"

#include <libxml/parser.h>

#include <map>
#include <string>
#include <vector>

namespace avg {

class AVGNode;
class Node;
class DivNode;
class Event;
class MouseEvent;
class ConradRelais;
class DisplayEngine;
class Camera;

class Player : IEventSink
{
    public:
        Player ();
        virtual ~Player ();

        enum DisplayEngineType{DFB, OGL};
        void setDisplayEngine(DisplayEngineType engine);
        void setResolution(bool bFullscreen, 
                int width=0, int height=0, int bpp=0);
        void loadFile (const std::string& fileName);
        void play();
        void stop();
        bool isPlaying();
        void setFramerate(double rate);
        bool setVBlankFramerate(int rate);

        Node * createNodeFromXmlString (const std::string& sXML);
        int setInterval(int time, PyObject * pyfunc);
        int setTimeout(int time, PyObject * pyfunc);
        bool clearInterval(int id);
        const Event& getCurEvent() const;
        const MouseEvent& getMouseState() const;
        bool screenshot(const std::string& sFilename);
        void showCursor(bool bShow);

        Node * getElementByID(const std::string& id);
        AVGNode * getRootNode();
        void doFrame();
        double getFramerate();
        double getVideoRefreshRate();
        virtual bool handleEvent(Event * pEvent);
        DisplayEngine * getDisplayEngine() const;

        void registerFrameListener(IFrameListener* pListener);
        void unregisterFrameListener(IFrameListener* pListener);
        std::string getCurDirName();
        void initNode(Node * pNode, DivNode * pParent);

    private:
        void initConfig();

        Node * createNodeFromXml(const xmlDocPtr xmlDoc, 
                const xmlNodePtr xmlNode, DivNode * pParent);

        void initDisplay(const xmlNodePtr xmlNode);
        void render (bool bRenderEverything);
        void createMouseOver(MouseEvent * pOtherEvent, Event::Type Type, 
                Node * pNode);
        void cleanup();
	
        AVGNode * m_pRootNode;
        DisplayEngine * m_pDisplayEngine;
        IEventSource * m_pEventSource;

        std::string m_CurDirName;
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
        bool m_bShowCursor;

        bool m_bIsPlaying;

        std::vector<IFrameListener*> m_Listeners;
};

}
#endif //_Player_H_
