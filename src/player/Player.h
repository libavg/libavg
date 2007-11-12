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
#include "Timeout.h"
#include "KeyEvent.h"
#include "MouseEvent.h"
#include "DisplayEngine.h"
#include "TestHelper.h"
#include "Node.h"
#include "DisplayParams.h"

#include "../base/IFrameListener.h"

#include <libxml/parser.h>

#include <map>
#include <string>
#include <vector>

namespace avg {

class Event;
class TrackerEventSource;

class Player : IEventSink
{
    public:
        Player();
        virtual ~Player();

        void setResolution(bool bFullscreen, 
                int width=0, int height=0, int bpp=0);
        void setOGLOptions(bool bUsePOW2Textures, YCbCrMode DesiredYCbCrMode, 
                bool bUsePixelBuffers, int MultiSampleSamples);
        void loadFile (const std::string& fileName);
        void play();
        void stop();
        bool isPlaying();
        void setFramerate(double rate);
        bool setVBlankFramerate(int rate);
        double getEffectiveFramerate();
        TestHelper * getTestHelper();
        void setFakeFPS(double fps);
        long long getFrameTime();

        NodePtr createNodeFromXmlString (const std::string& sXML);
        TrackerEventSource * addTracker();
        int setInterval(int time, PyObject * pyfunc);
        int setTimeout(int time, PyObject * pyfunc);
        int setOnFrameHandler(PyObject * pyfunc);
        bool clearInterval(int id);

        const Event& getCurEvent() const;
        const MouseEvent& getMouseState() const;
        Bitmap * screenshot();
        void showCursor(bool bShow);
        void setEventCapture(NodeWeakPtr pNode, int cursorID=MOUSECURSORID);
        void releaseEventCapture(int cursorID=MOUSECURSORID);

        NodePtr getElementByID(const std::string& id);
        void registerNode(NodePtr pNode);
        void addNodeID(NodePtr pNode);
        void removeNodeID(const std::string& id);
        AVGNodePtr getRootNode();
        void doFrame();
        double getFramerate();
        double getVideoRefreshRate();
        void setGamma(double Red, double Green, double Blue);
        virtual bool handleEvent(Event * pEvent);
        DisplayEngine * getDisplayEngine() const;
        void useFakeCamera(bool bFake);

        void registerFrameListener(IFrameListener* pListener);
        void unregisterFrameListener(IFrameListener* pListener);
        std::string getCurDirName();

    private:
        void initConfig();
        void initGraphics();

        NodePtr createNodeFromXml(const xmlDocPtr xmlDoc, 
                const xmlNodePtr xmlNode, DivNodeWeakPtr pParent);

        void render (bool bRenderEverything);
        void sendOver(CursorEvent * pOtherEvent, Event::Type Type, 
                NodePtr pNode);
        void cleanup();

        AVGNodePtr m_pRootNode;
        DisplayEngine * m_pDisplayEngine;
        IEventSource * m_pEventSource;
        TestHelper m_TestHelper;
        
        std::string m_CurDirName;
        bool m_bStopping;
        typedef std::map<std::string, NodePtr> NodeIDMap;
        NodeIDMap m_IDMap;

        TrackerEventSource * m_pTracker;

        int addTimeout(Timeout* pTimeout);
        void removeTimeout(Timeout* pTimeout);
        void handleTimers();
        bool m_bInHandleTimers;
        bool m_bCurrentTimeoutDeleted;

        std::vector<Timeout *> m_PendingTimeouts;
        std::vector<Timeout *> m_NewTimeouts; // Timeouts to be added this frame.

        EventDispatcherPtr m_pEventDispatcher;
        std::map<int, NodePtr> m_pLastMouseNode;
        std::map<int, NodeWeakPtr> m_pEventCaptureNode;

        // Configuration variables.
        DisplayParams m_DP;
        bool m_bUsePOW2Textures;
        YCbCrMode m_YCbCrMode;
        bool m_bUsePixelBuffers;
        int m_MultiSampleSamples;
        bool m_bUseFakeCamera;
        VSyncMode m_VSyncMode;

        bool m_bIsPlaying;

        // Time calculation
        bool m_bFakeFPS;
        double m_FakeFPS;
        long long m_FrameTime;
        long long m_PlayStartTime;
        long long m_NumFrames;

        std::vector<IFrameListener*> m_Listeners;
        xmlDtdPtr m_dtd;
};

}
#endif //_Player_H_
