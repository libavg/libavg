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
#include "EventDispatcher.h"
#include "KeyEvent.h"
#include "MouseEvent.h"
#include "CursorState.h"
#include "MouseState.h"
#include "TestHelper.h"

#include "../audio/AudioParams.h"

#include <libxml/parser.h>
#include <boost/shared_ptr.hpp>

#include <string>
#include <vector>

namespace avg {

class AudioEngine;
class Node;
class VisibleNode;
class Canvas;
class MainCanvas;
class OffscreenCanvas;
class TrackerInputDevice;
class MultitouchInputDevice;
class IFrameEndListener;
class IPlaybackEndListener;
class IPreRenderListener;

typedef boost::shared_ptr<Node> NodePtr;
typedef boost::weak_ptr<Node> NodeWeakPtr;
typedef boost::shared_ptr<VisibleNode> VisibleNodePtr;
typedef boost::weak_ptr<VisibleNode> VisibleNodeWeakPtr;
typedef boost::shared_ptr<Canvas> CanvasPtr;
typedef boost::shared_ptr<MainCanvas> MainCanvasPtr;
typedef boost::shared_ptr<OffscreenCanvas> OffscreenCanvasPtr;


class AVG_API Player
{
    public:
        Player();
        virtual ~Player();
        static Player* get();
        static bool exists();

        void setResolution(bool bFullscreen,
                int width=0, int height=0, int bpp=0);
        void setWindowFrame(bool bHasWindowFrame);
        void setWindowPos(int x=0, int y=0);
        void setOGLOptions(bool bUsePOTTextures, bool bUseShaders, 
                bool bUsePixelBuffers, int multiSampleSamples);
        void setMultiSampleSamples(int multiSampleSamples);
        void setAudioOptions(int samplerate, int channels);
        DPoint getScreenResolution();

        CanvasPtr loadFile(const std::string& sFilename);
        CanvasPtr loadString(const std::string& sAVG);

        OffscreenCanvasPtr loadCanvasFile(const std::string& sFilename);
        OffscreenCanvasPtr loadCanvasString(const std::string& sAVG);
        void deleteCanvas(const std::string& sID);
        CanvasPtr getMainCanvas() const;
        OffscreenCanvasPtr getCanvas(const std::string& sID) const;
        void newCanvasDependency(const OffscreenCanvasPtr pCanvas);

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
        long long getTimeSinceLastFrame();
        double getFrameDuration();

        void registerNodeType(NodeDefinition Def, const char* pParentNames[] = 0);
        
        NodePtr createNode(const std::string& sType, const boost::python::dict& PyDict);
        NodePtr createNodeFromXmlString(const std::string& sXML);
        
        int setInterval(int time, PyObject * pyfunc);
        int setTimeout(int time, PyObject * pyfunc);
        int setOnFrameHandler(PyObject * pyfunc);
        bool clearInterval(int id);

        void addInputDevice(IInputDevicePtr pSource);
        MouseEventPtr getMouseState() const;
        TrackerInputDevice * addTracker();
        TrackerInputDevice * getTracker();
        void enableMultitouch();
        bool isMultitouchAvailable() const;
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
        bool isUsingShaders();
        void setGamma(double red, double green, double blue);
        DisplayEngine * getDisplayEngine() const;
        void setStopOnEscape(bool bStop);
        bool getStopOnEscape() const;
        void setVolume(double volume);
        double getVolume() const;

        OffscreenCanvasPtr getCanvasFromURL(const std::string& sURL);

        std::string getCurDirName();
        std::string getRootMediaDir();
        const NodeDefinition& getNodeDef(const std::string& sType);

        void disablePython();

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

        bool handleEvent(EventPtr pEvent);
        void handleCursorEvent(boost::shared_ptr<DivNode> pDivNode, CursorEventPtr pEvent, bool bOnlyCheckCursorOver=false);
        
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
        OffscreenCanvasPtr registerOffscreenCanvas(NodePtr pNode);
        OffscreenCanvasPtr findCanvas(const std::string& sID) const;
        void endFrame();

        void sendFakeEvents();
        void sendOver(CursorEventPtr pOtherEvent, Event::Type type, VisibleNodePtr pNode);
        void handleCursorEvent(CursorEventPtr pEvent, bool bOnlyCheckCursorOver=false);

        void dispatchOffscreenRendering(OffscreenCanvas* pOffscreenCanvas);

        void errorIfPlaying(const std::string& sFunc) const;

        MainCanvasPtr m_pMainCanvas;

        DisplayEnginePtr m_pDisplayEngine;
        AudioEngine * m_pAudioEngine;
        TestHelperPtr m_pTestHelper;
       
        std::string m_CurDirName;
        bool m_bStopping;
        NodeRegistry m_NodeRegistry;

        IInputDevicePtr m_pMultitouchInputDevice;

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

        std::vector<OffscreenCanvasPtr> m_pCanvases;

        static Player * s_pPlayer;
        friend void deletePlayer();
        
        EventDispatcherPtr m_pEventDispatcher;
        struct EventCaptureInfo {
            EventCaptureInfo(const VisibleNodeWeakPtr& pNode);

            VisibleNodeWeakPtr m_pNode;
            int m_CaptureCount;
        };
        typedef boost::shared_ptr<EventCaptureInfo> EventCaptureInfoPtr;
        
        std::map<int, EventCaptureInfoPtr> m_EventCaptureInfoMap;
        
        MouseState m_MouseState;

        // These are maps for each cursor id.
        std::map<int, CursorStatePtr> m_pLastCursorStates;
        PyObject * m_EventHookPyFunc;
};

}
#endif
