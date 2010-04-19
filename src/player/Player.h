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
#include "Timeout.h"
#include "DisplayEngine.h"
#include "NodeRegistry.h"
#include "MouseEvent.h"
#include "DisplayParams.h"
#include "GLConfig.h"
#include "IEventSink.h"
#include "EventDispatcher.h"
#include "KeyEvent.h"
#include "MouseEvent.h"
#include "CursorState.h"
#include "MouseState.h"

#include "../audio/AudioParams.h"

#include <libxml/parser.h>

#include <string>
#include <vector>

namespace avg {

class AudioEngine;
class Node;
class VisibleNode;
class TestHelper;
class Scene;
class MainScene;
class OffscreenScene;
class TrackerEventSource;
class IFrameEndListener;
class IPlaybackEndListener;
class IPreRenderListener;

typedef boost::shared_ptr<Node> NodePtr;
typedef boost::weak_ptr<Node> NodeWeakPtr;
typedef boost::shared_ptr<VisibleNode> VisibleNodePtr;
typedef boost::weak_ptr<VisibleNode> VisibleNodeWeakPtr;
typedef boost::shared_ptr<Scene> ScenePtr;
typedef boost::shared_ptr<MainScene> MainScenePtr;
typedef boost::shared_ptr<OffscreenScene> OffscreenScenePtr;

class AVG_API Player: IEventSink
{
    public:
        Player();
        virtual ~Player();
        static Player* get();
        static bool exists();

        void setResolution(bool bFullscreen,
                int width=0, int height=0, int bpp=0);
        void setWindowPos(int x=0, int y=0);
        void setOGLOptions(bool bUsePOTTextures, bool bUseShaders, 
                bool bUsePixelBuffers, int MultiSampleSamples);
        void setMultiSampleSamples(int MultiSampleSamples);
        void enableAudio(bool bEnable);
        void setAudioOptions(int samplerate, int channels);
        ScenePtr loadFile(const std::string& sFilename);
        ScenePtr loadString(const std::string& sAVG);

        OffscreenScenePtr loadSceneFile(const std::string& sFilename);
        OffscreenScenePtr loadSceneString(const std::string& sAVG);
        void deleteScene(const std::string& sID);
        ScenePtr getMainScene() const;
        OffscreenScenePtr getScene(const std::string& sID) const;
        void newSceneDependency(const OffscreenScenePtr pScene);

        void play();
        void stop();
        bool isStopping();
        void initPlayback();
        void cleanup();
        bool isPlaying();
        void setFramerate(double rate);
        void setVBlankFramerate(int rate);
        double getEffectiveFramerate();
        TestHelper * getTestHelper();
        void setFakeFPS(double fps);
        long long getFrameTime();

        void registerNodeType(NodeDefinition Def, const char* pParentNames[] = 0);
        
        NodePtr createNode(const std::string& sType, const boost::python::dict& PyDict);
        NodePtr createNodeFromXmlString(const std::string& sXML);
        
        int setInterval(int time, PyObject * pyfunc);
        int setTimeout(int time, PyObject * pyfunc);
        int setOnFrameHandler(PyObject * pyfunc);
        bool clearInterval(int id);

        void addEventSource(IEventSource* pSource);
        MouseEventPtr getMouseState() const;
        TrackerEventSource * addTracker();
        TrackerEventSource * getTracker();
        void setEventCapture(VisibleNodePtr pNode, int cursorID);
        void releaseEventCapture(int cursorID);

        EventPtr getCurEvent() const;
        void setMousePos(const IntPoint& pos);
        int getKeyModifierState() const;
        BitmapPtr screenshot();
        void setCursor(const Bitmap* pBmp, IntPoint hotSpot);
        void showCursor(bool bShow);

        VisibleNodePtr getElementByID(const std::string& id);
        AVGNodePtr getRootNode();
        void doFrame(bool bFirstFrame);
        double getFramerate();
        double getVideoRefreshRate();
        void setGamma(double Red, double Green, double Blue);
        DisplayEngine * getDisplayEngine() const;
        void setStopOnEscape(bool bStop);
        bool getStopOnEscape() const;
        void setVolume(double volume);
        double getVolume() const;

        OffscreenScenePtr getSceneFromURL(const std::string& sURL);

        std::string getCurDirName();
        std::string getRootMediaDir();
        const NodeDefinition& getNodeDef(const std::string& sType);

        void disablePython();
        bool isAudioEnabled() const;

        void loadPlugin(const std::string& name);
        void setPluginPath(const std::string& newPath);
        std::string getPluginPath() const;
        
        void setEventHook(PyObject * pyfunc);
        PyObject * getEventHook() const;

        void registerFrameEndListener(IFrameEndListener* pListener);
        void unregisterFrameEndListener(IFrameEndListener* pListener);
        void registerPlaybackEndListener(IPlaybackEndListener* pListener);
        void unregisterPlaybackEndListener(IPlaybackEndListener* pListener);
        void registerPreRenderListener(IPreRenderListener* pListener);
        void unregisterPreRenderListener(IPreRenderListener* pListener);

        virtual bool handleEvent(EventPtr pEvent);
        
    private:
        void initConfig();
        void initGraphics();
        void initAudio();

        void updateDTD();

        NodePtr loadMainNodeFromFile(const std::string& sFilename);
        NodePtr loadMainNodeFromString(const std::string& sAVG);
        NodePtr internalLoad(const std::string& sAVG);

        NodePtr createNodeFromXml(const xmlDocPtr xmlDoc,
                const xmlNodePtr xmlNode);
        OffscreenScenePtr registerOffscreenScene(NodePtr pNode);
        OffscreenScenePtr findScene(const std::string& sID) const;
        void endFrame();

        void sendFakeEvents();
        void sendOver(CursorEventPtr pOtherEvent, Event::Type Type, VisibleNodePtr pNode);
        void handleCursorEvent(CursorEventPtr pEvent, bool bOnlyCheckCursorOver=false);

        MainScenePtr m_pMainScene;

        DisplayEngine * m_pDisplayEngine;
        AudioEngine * m_pAudioEngine;
        TestHelper * m_pTestHelper;
       
        bool m_bAudioEnabled;
        std::string m_CurDirName;
        bool m_bStopping;
        NodeRegistry m_NodeRegistry;

        TrackerEventSource * m_pTracker;

        int addTimeout(Timeout* pTimeout);
        void removeTimeout(Timeout* pTimeout);
        void handleTimers();
        bool m_bInHandleTimers;
        bool m_bCurrentTimeoutDeleted;

        std::vector<Timeout *> m_PendingTimeouts;
        std::vector<Timeout *> m_NewTimeouts; // Timeouts to be added this frame.

        // Configuration variables.
        DisplayParams m_DP;
        AudioParams m_AP;
        GLConfig m_GLConfig;

        bool m_bStopOnEscape;
        bool m_bIsPlaying;

        // Time calculation
        bool m_bFakeFPS;
        double m_FakeFPS;
        long long m_FrameTime;
        long long m_PlayStartTime;
        long long m_NumFrames;

        double m_Volume;

        bool m_bDirtyDTD;
        xmlDtdPtr m_dtd;

        bool m_bPythonAvailable;

        std::vector<OffscreenScenePtr> m_pScenes;

        static Player * s_pPlayer;
        friend void deletePlayer();
        
        EventDispatcherPtr m_pEventDispatcher;
        std::map<int, VisibleNodeWeakPtr> m_pEventCaptureNode;
        
        MouseState m_MouseState;

        // These are maps for each cursor id.
        std::map<int, CursorStatePtr> m_pLastCursorStates;
        PyObject * m_EventHookPyFunc;
};

}
#endif
