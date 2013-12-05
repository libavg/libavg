//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2011 Ulrich von Zadow
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
#include "Publisher.h"
#include "Timeout.h"
#include "TypeRegistry.h"
#include "DisplayParams.h"
#include "CursorState.h"
#include "TestHelper.h"
#include "BoostPython.h"

#include "../audio/AudioParams.h"
#include "../graphics/GLConfig.h"

#include <libxml/parser.h>
#include <boost/shared_ptr.hpp>
#include <boost/thread/thread.hpp>

#include <string>
#include <vector>
#include <set>

namespace avg {

class AudioEngine;
class Node;
class Canvas;
class MainCanvas;
class OffscreenCanvas;
class TrackerInputDevice;
class MultitouchInputDevice;
class IFrameEndListener;
class IPlaybackEndListener;
class IPreRenderListener;
class Contact;
class EventDispatcher;
class MouseEvent;
class CursorEvent;
class SDLDisplayEngine;
class Display;

typedef boost::shared_ptr<Node> NodePtr;
typedef boost::weak_ptr<Node> NodeWeakPtr;
typedef boost::shared_ptr<Canvas> CanvasPtr;
typedef boost::shared_ptr<MainCanvas> MainCanvasPtr;
typedef boost::shared_ptr<OffscreenCanvas> OffscreenCanvasPtr;
typedef boost::shared_ptr<class Contact> ContactPtr;
typedef boost::shared_ptr<EventDispatcher> EventDispatcherPtr;
typedef boost::shared_ptr<MouseEvent> MouseEventPtr;
typedef boost::shared_ptr<CursorEvent> CursorEventPtr;
typedef boost::shared_ptr<SDLDisplayEngine> SDLDisplayEnginePtr;
typedef boost::shared_ptr<Display> DisplayPtr;

class AVG_API Player: public Publisher
{
    public:
        Player();
        virtual ~Player();
        static Player* get();
        static bool exists();

        void setResolution(bool bFullscreen,
                int width=0, int height=0, int bpp=0);
        bool isFullscreen();
        void setWindowFrame(bool bHasWindowFrame);
        void setWindowPos(int x=0, int y=0);
        void setWindowTitle(const std::string& sTitle);
        void useGLES(bool bGLES);
        void setOGLOptions(bool bUsePOTTextures, bool bUsePixelBuffers, 
                int multiSampleSamples, GLConfig::ShaderUsage shaderUsage,
                bool bUseDebugContext);
        void setMultiSampleSamples(int multiSampleSamples);
        void setAudioOptions(int samplerate, int channels);
        void enableGLErrorChecks(bool bEnable);
        glm::vec2 getScreenResolution();
        float getPixelsPerMM();
        glm::vec2 getPhysicalScreenDimensions();
        void assumePixelsPerMM(float ppmm);

        CanvasPtr loadFile(const std::string& sFilename);
        CanvasPtr loadString(const std::string& sAVG);

        OffscreenCanvasPtr loadCanvasFile(const std::string& sFilename);
        OffscreenCanvasPtr loadCanvasString(const std::string& sAVG);
        CanvasPtr createMainCanvas(const py::dict& params);
        OffscreenCanvasPtr createCanvas(const py::dict& params);
        void deleteCanvas(const std::string& sID);
        CanvasPtr getMainCanvas() const;
        OffscreenCanvasPtr getCanvas(const std::string& sID) const;
        void newCanvasDependency();

        void play();
        void stop();
        bool isStopping();
        void initPlayback(const std::string& sShaderPath = "");
        void cleanup(bool bIsAbort);
        bool isPlaying();
        void setFramerate(float rate);
        void setVBlankFramerate(int rate);
        float getEffectiveFramerate();
        TestHelper * getTestHelper();
        void setFakeFPS(float fps);
        long long getFrameTime();
        float getFrameDuration();

        NodePtr createNode(const std::string& sType, const py::dict& PyDict,
                const py::object& self=py::object());
        NodePtr createNodeFromXmlString(const std::string& sXML);
        
        int setInterval(int time, PyObject * pyfunc);
        int setTimeout(int time, PyObject * pyfunc);
        int setOnFrameHandler(PyObject * pyfunc);
        bool clearInterval(int id);
        void callFromThread(PyObject * pyfunc);

        void addInputDevice(IInputDevicePtr pSource);
        MouseEventPtr getMouseState() const;
        EventPtr getCurrentEvent() const;
        TrackerInputDevice * getTracker();
        void enableMultitouch();
        void enableMouse(bool enabled);
        bool isMultitouchAvailable() const;
        void setEventCapture(NodePtr pNode, int cursorID);
        void releaseEventCapture(int cursorID);
        bool isCaptured(int cursorID);
        void removeDeadEventCaptures();
        EventPtr getCurEvent() const;
        void setMousePos(const IntPoint& pos);
        int getKeyModifierState() const;

        BitmapPtr screenshot();
        void setCursor(const Bitmap* pBmp, IntPoint hotSpot);
        void showCursor(bool bShow);
        bool isCursorShown();

        NodePtr getElementByID(const std::string& id);
        AVGNodePtr getRootNode();
        void doFrame(bool bFirstFrame);
        float getFramerate();
        float getVideoRefreshRate();
        size_t getVideoMemInstalled();
        size_t getVideoMemUsed();
        void setGamma(float red, float green, float blue);
        SDLDisplayEngine * getDisplayEngine() const;
        void keepWindowOpen();
        void setStopOnEscape(bool bStop);
        bool getStopOnEscape() const;
        void setVolume(float volume);
        float getVolume() const;
        std::string getConfigOption(const std::string& sSubsys, const std::string& sName)
                const;
        bool isUsingGLES() const;
        bool areFullShadersSupported() const;

        OffscreenCanvasPtr getCanvasFromURL(const std::string& sURL);

        std::string getCurDirName();
        std::string getRootMediaDir();

        void disablePython();
        void startTraversingTree();
        void endTraversingTree();
        bool isTraversingTree() const;

        py::object loadPlugin(const std::string& name);
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

    private:
        void initConfig();
        void initGraphics(const std::string& sShaderPath);
        void initAudio();
        void initMainCanvas(NodePtr pRootNode);

        NodePtr loadMainNodeFromFile(const std::string& sFilename);
        NodePtr loadMainNodeFromString(const std::string& sAVG);
        NodePtr internalLoad(const std::string& sAVG, const std::string& sFilename);
        SDLDisplayEnginePtr safeGetDisplayEngine();

        NodePtr createNodeFromXml(const xmlDocPtr xmlDoc,
                const xmlNodePtr xmlNode);
        OffscreenCanvasPtr registerOffscreenCanvas(NodePtr pNode);
        OffscreenCanvasPtr findCanvas(const std::string& sID) const;
        void endFrame();

        void sendFakeEvents();
        void sendOver(CursorEventPtr pOtherEvent, Event::Type type, NodePtr pNode);
        void handleCursorEvent(CursorEventPtr pEvent, bool bOnlyCheckCursorOver=false);

        void dispatchOffscreenRendering(OffscreenCanvas* pOffscreenCanvas);

        void errorIfPlaying(const std::string& sFunc) const;

        MainCanvasPtr m_pMainCanvas;

        SDLDisplayEnginePtr m_pDisplayEngine;
        bool m_bDisplayEngineBroken;
        TestHelperPtr m_pTestHelper;
       
        std::string m_CurDirName;
        bool m_bIsTraversingTree;
        bool m_bStopping;

        IInputDevicePtr m_pMultitouchInputDevice;

        // Timeout handling
        int internalSetTimeout(int time, PyObject * pyfunc, bool bIsInterval);
        int addTimeout(Timeout* pTimeout);
        void handleTimers();
        bool m_bInHandleTimers;
        bool m_bCurrentTimeoutDeleted;

        std::vector<Timeout *> m_PendingTimeouts;
        std::vector<Timeout *> m_NewTimeouts; // Timeouts to be added this frame.
        std::vector<Timeout *> m_AsyncCalls;
        boost::mutex m_AsyncCallMutex;

        // Configuration variables.
        DisplayParams m_DP;
        AudioParams m_AP;
        GLConfig m_GLConfig;

        bool m_bKeepWindowOpen;
        bool m_bStopOnEscape;
        bool m_bIsPlaying;

        // Time calculation
        bool m_bFakeFPS;
        float m_FakeFPS;
        long long m_FrameTime;
        long long m_PlayStartTime;
        long long m_NumFrames;

        float m_Volume;

        bool m_bPythonAvailable;

        std::vector<OffscreenCanvasPtr> m_pCanvases;

        static Player * s_pPlayer;
        friend void deletePlayer();
        
        EventDispatcherPtr m_pEventDispatcher;
        struct EventCaptureInfo {
            EventCaptureInfo(const NodeWeakPtr& pNode);

            NodePtr m_pNode;
            int m_CaptureCount;
        };
        typedef boost::shared_ptr<EventCaptureInfo> EventCaptureInfoPtr;
        
        std::map<int, EventCaptureInfoPtr> m_EventCaptureInfoMap;

        MouseEventPtr m_pLastMouseEvent;
        EventPtr m_pCurrentEvent;

        // The indexes of this map are cursorids.
        std::map<int, CursorStatePtr> m_pLastCursorStates;

        PyObject * m_EventHookPyFunc;
        bool m_bMouseEnabled;
};

}
#endif
