//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2008 Ulrich von Zadow
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

#include "../api.h"
#include "IEventSink.h"
#include "EventDispatcher.h"
#include "Timeout.h"
#include "KeyEvent.h"
#include "MouseEvent.h"
#include "DisplayEngine.h"
#include "TestHelper.h"
#include "NodeRegistry.h"
#include "DisplayParams.h"
#include "CursorState.h"
#include "MouseState.h"

#include "../base/IFrameListener.h"

#include "../audio/AudioParams.h"

#include <libxml/parser.h>

#include <map>
#include <string>
#include <vector>

namespace avg {

class TrackerEventSource;
class AudioEngine;
class Node;

typedef boost::shared_ptr<Node> NodePtr;
typedef boost::weak_ptr<Node> NodeWeakPtr;

class AVG_API Player : IEventSink
{
    public:
        Player();
        virtual ~Player();
        static Player* get();

        void updateDTD();

        void setResolution(bool bFullscreen,
                int width=0, int height=0, int bpp=0);
        void setWindowPos(int x=0, int y=0);
        void setOGLOptions(bool bUsePOW2Textures, YCbCrMode DesiredYCbCrMode, 
                bool bUsePixelBuffers, int MultiSampleSamples);
        void setMultiSampleSamples(int MultiSampleSamples);
        void setAudioOptions(int samplerate, int channels);
        void loadFile(const std::string& sFilename);
        void loadString(const std::string& sAVG);
        void play();
        void stop();
        void initPlayback();
        void cleanup();
        bool isPlaying();
        void setFramerate(double rate);
        bool setVBlankFramerate(int rate);
        double getEffectiveFramerate();
        TestHelper * getTestHelper();
        void setFakeFPS(double fps);
        long long getFrameTime();

        void registerNodeType(NodeDefinition Def, const char* pParentNames[] = 0);
        
        NodePtr createNode (const std::string& sType, const boost::python::dict& PyDict);
        NodePtr createNodeFromXmlString (const std::string& sXML);
        TrackerEventSource * addTracker(const std::string& sConfigFilename);
        TrackerEventSource * getTracker();
        int setInterval(int time, PyObject * pyfunc);
        int setTimeout(int time, PyObject * pyfunc);
        int setOnFrameHandler(PyObject * pyfunc);
        bool clearInterval(int id);

        EventPtr getCurEvent() const;
        MouseEventPtr getMouseState() const;
        void setMousePos(const IntPoint& pos);
        Bitmap * screenshot();
        void setCursor(const Bitmap* pBmp, IntPoint hotSpot);
        void showCursor(bool bShow);
        void setEventCapture(NodePtr pNode, int cursorID=MOUSECURSORID);
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
        virtual bool handleEvent(EventPtr pEvent);
        DisplayEngine * getDisplayEngine() const;
        void useFakeCamera(bool bFake);
        void stopOnEscape(bool bStop);
        void setVolume(double volume);
        double getVolume() const;
        long long getGPUMemoryUsage();

        void registerFrameListener(IFrameListener* pListener);
        void unregisterFrameListener(IFrameListener* pListener);
        std::string getCurDirName();
        std::string getRootMediaDir();
        const NodeDefinition& getNodeDef(const std::string& sType);

        void disablePython();

        void loadPlugin(const std::string& name);
        void setPluginPath(const std::string& newPath);
        std::string getPluginPath() const;
    private:
        void initConfig();
        void initGraphics();
        void initAudio();

        void internalLoad(const std::string& sAVG);

        NodePtr createNodeFromXml(const xmlDocPtr xmlDoc, 
                const xmlNodePtr xmlNode, DivNodeWeakPtr pParent);

        void render (bool bRenderEverything);

        void sendFakeEvents();
        void sendOver(CursorEventPtr pOtherEvent, Event::Type Type, NodePtr pNode);
        void handleCursorEvent(CursorEventPtr pEvent, bool bOnlyCheckCursorOver=false);
        std::vector<NodeWeakPtr> getElementsByPos(const DPoint& Pos) const;

        AVGNodePtr m_pRootNode;
        DisplayEngine * m_pDisplayEngine;
        AudioEngine * m_pAudioEngine;
        IEventSource * m_pEventSource;
        TestHelper * m_pTestHelper;
        
        std::string m_CurDirName;
        bool m_bStopping;
        typedef std::map<std::string, NodePtr> NodeIDMap;
        NodeIDMap m_IDMap;
        NodeRegistry m_NodeRegistry;

        TrackerEventSource * m_pTracker;

        int addTimeout(Timeout* pTimeout);
        void removeTimeout(Timeout* pTimeout);
        void handleTimers();
        bool m_bInHandleTimers;
        bool m_bCurrentTimeoutDeleted;

        std::vector<Timeout *> m_PendingTimeouts;
        std::vector<Timeout *> m_NewTimeouts; // Timeouts to be added this frame.

        EventDispatcherPtr m_pEventDispatcher;
        MouseState m_MouseState;

        // These are maps for each cursor id.
        std::map<int, CursorStatePtr> m_pLastCursorStates;
        std::map<int, NodeWeakPtr> m_pEventCaptureNode;

        // Configuration variables.
        DisplayParams m_DP;
        AudioParams m_AP;
        bool m_bUsePOW2Textures;
        YCbCrMode m_YCbCrMode;
        bool m_bUsePixelBuffers;
        int m_MultiSampleSamples;
        bool m_bUseFakeCamera;
        VSyncMode m_VSyncMode;
        long long m_MaxGPUMemUsed;

        bool m_bStopOnEscape;

        bool m_bIsPlaying;

        // Time calculation
        bool m_bFakeFPS;
        double m_FakeFPS;
        long long m_FrameTime;
        long long m_PlayStartTime;
        long long m_NumFrames;

        double m_Volume;

        std::vector<IFrameListener*> m_Listeners;
        bool m_bDirtyDTD;
        xmlDtdPtr m_dtd;

        bool m_bPythonAvailable;

        static Player * s_pPlayer;
        friend void deletePlayer();
};

}
#endif //_Player_H_
