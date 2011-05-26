# libavg - Media Playback Engine.
# Copyright (C) 2003-2008 Ulrich von Zadow
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
# Current versions can be found at www.libavg.de
#
# Original author of this file is Martin Heistermann <mh at sponc dot de>
#

import os
import gc

from libavg import avg, Point2D
import graph
from mtemu import MTemu
import apphelpers

try:
    from alib.clicktest import ClickTest
except ImportError:
    ClickTest = None


DEFAULT_RESOLUTION = (640, 480)

g_player = avg.Player.get()
g_log = avg.Logger.get()


class AppStarter(object):
    """Starts an AVGApp"""
    def __init__(self, appClass, resolution=DEFAULT_RESOLUTION,
            debugWindowSize=None, fakeFullscreen=False):
        if fakeFullscreen:
            if os.name != 'nt':
                raise RuntimeError('fakeFullscreen works only under windows')
                
            g_player.setTimeout(0, self.__fakeFullscreen)

        resolution = Point2D(resolution)
        testMode = not 'AVG_DEPLOY' in os.environ
        
        if testMode and debugWindowSize is not None:
            debugWindowSize = Point2D(debugWindowSize)
        else:
            debugWindowSize = Point2D(0, 0)

        width = int(resolution.x)
        height = int(resolution.y)
        # dynamic avg creation in order to set resolution
        g_player.loadString("""
<?xml version="1.0"?>
<!DOCTYPE avg SYSTEM "../../libavg/doc/avg.dtd">
<avg width="%(width)u" height="%(height)u">
</avg>""" % {
            'width': width,
            'height': height,
            })
        rootNode = g_player.getRootNode()

        self._appNode = g_player.createNode('div',{
            'opacity': 0,
            'sensitive': False})
        # the app should know the size of its "root" node:
        self._appNode.size = rootNode.size
        rootNode.appendChild(self._appNode)

        self.__showMTEvents = False
        self.__touchViss = {} 
        self.__touchVisOverlay = avg.DivNode(sensitive=False, size=resolution,
                parent=rootNode)

        g_player.showCursor(testMode)

        if appClass.fakeFullscreen:
            fullscreen = False
        else:
            fullscreen = not testMode

        g_player.setResolution(
                fullscreen,
                int(debugWindowSize.x), int(debugWindowSize.y),
                0 # color depth
                )
                
        self._onBeforePlay()
        g_player.setTimeout(0, self._onStart)
        self._appInstance = appClass(self._appNode)
        self._keyManager = apphelpers.KeyManager(self._appInstance.onKeyDown,
                self._appInstance.onKeyUp)
        
        self._setupDefaultKeys()
        
        self._appInstance.setStarter(self)
        g_player.play()
        self._appInstance.exit()

    def _setupDefaultKeys(self):
        pass
        
    def _onBeforePlay(self):
        pass

    def _onStart(self):
        self._appInstance.init()
        self._appNode.opacity = 1
        self._appNode.sensitive = True
        self._activeApp = self._appInstance
        self._appInstance.enter()

    def __fakeFullscreen(self):
        import win32gui
        import win32con
        import win32api

        def findWindow(title):
            def enumWinProc(h, lparams): 
                lparams.append(h)
            winList=[]
            win32gui.EnumWindows(enumWinProc, winList)
            for hwnd in winList:
                curTitle = win32gui.GetWindowText(hwnd)
                if win32gui.IsWindowVisible(hwnd) and title == curTitle:
                    return hwnd
            return None

        hDesk = win32gui.GetDesktopWindow()
        (desktopLeft, desktopTop, desktopRight,
                desktopBottom) = win32gui.GetWindowRect(hDesk)
        w = findWindow("AVG Renderer")
        offSetX = 2
        offSetY = 3
        win32gui.SetWindowPos(w, win32con.HWND_TOP,
                -(win32api.GetSystemMetrics(win32con.SM_CYBORDER) + offSetX),
                -(win32api.GetSystemMetrics(win32con.SM_CYCAPTION) + offSetY), 
                desktopRight, desktopBottom + 30, 0)


class AVGAppStarter(AppStarter):
    def __init__(self, *args, **kwargs):
        self.__graphs = 0
        self._mtEmu = None
        self.__showingMemGraph = False
        self.__showingFrGraph = False
        self.__runningClickTest = False
        self.__notifyNode = None
        # self._initClickTest()

        super(AVGAppStarter, self).__init__(*args, **kwargs)
        
    def _setupDefaultKeys(self):
        self._keyManager.bindKey('o', self.__dumpObjects, 'Dump objects')
        self._keyManager.bindKey('m', self.__showMemoryUsage, 'Show memory usage')
        self._keyManager.bindKey('f', self.__showFrameRateUsage, 'Show frameTime usage')
        # self._keyManager.bindKey('.', self.__switchClickTest, 'Start clicktest')
        self._keyManager.bindKey('t', self.__switchMtemu, 'Activate multitouch emulation')  
        self._keyManager.bindKey('e', self.__switchShowMTEvents, 'Show multitouch events')  
        self._keyManager.bindKey('s', self.__screenshot, 'Take screenshot')  

    # def _initClickTest(self):
    #     if ClickTest:
    #         self._clickTest = ClickTest(self._appNode, multiClick=False)
    #     else:
    #         self._clickTest = None

    def __dumpObjects(self):
        gc.collect()
        testHelper = g_player.getTestHelper()
        testHelper.dumpObjects()
        print "Num anims: ", avg.getNumRunningAnims()
        print "Num python objects: ", len(gc.get_objects()) 

    def __showMemoryUsage(self):
        if self.__showingMemGraph:
            self.__memGraph.delete()
            self.__memGraph = None
            self.__graphs = self.__graphs -1
            if(self.__graphs == 1 ):
                self.__frGraph.setYpos(10)
        else:
            self.__memGraph = graph.MemGraph("Memory Graph",
                    getValue = avg.getMemoryUsage)         
            self.__graphs = self.__graphs +1          
            if(self.__graphs > 1 ):
                self.__memGraph.setYpos(190)       
        self.__showingMemGraph = not(self.__showingMemGraph)
     
        
    def __showFrameRateUsage(self):
        if self.__showingFrGraph:
            self.__frGraph.delete()
            self.__frGraph = None
            self.__graphs = self.__graphs -1
            if(self.__graphs == 1 ):
                self.__memGraph.setYpos(10)
        else:      
            self.__frGraph = graph.FrameRateGraph("FrameTime Graph",
                    getValue = g_player.getFrameTime)
            self.__graphs = self.__graphs +1 
            if(self.__graphs >1):               
                self.__frGraph.setYpos(190)           
        self.__showingFrGraph = not(self.__showingFrGraph)
    
    def __switchClickTest(self):
        if self._clickTest:
            if self.__runningClickTest:
                g_log.trace(g_log.APP, 'Stopping clicktest')
                self._clickTest.stop()
            else:
                g_log.trace(g_log.APP, 'Starting clicktest')
                self._clickTest.start()
            
            self.__runningClickTest = not self.__runningClickTest

    def __switchMtemu(self):
        if self._mtEmu is None:
            self._mtEmu = MTemu()
            self._keyManager.bindKey('left ctrl',
                    self._mtEmu.changeMode, 'switch event mode')
            self._keyManager.bindKey('right ctrl',
                    self._mtEmu.changeMode, 'switch event mode')
            self._keyManager.bindKey('left shift',
                    self._mtEmu.multiTouch, 'create 2nd event')
            self._keyManager.bindKey('right shift',
                    self._mtEmu.multiTouch, 'create 2nd event')
            self._keyManager.bindKey('left shift',
                    self._mtEmu.multiTouch, 'create 2nd event', 'up')
            self._keyManager.bindKey('right shift',
                    self._mtEmu.multiTouch, 'create 2nd event', 'up')
            
        else:
            self.unbindKey('left ctrl')
            self.unbindKey('right ctrl')
            self.unbindKey('left shift')
            self.unbindKey('right shift')
            self._mtEmu.delete()
            self._mtEmu = None
   
    def __switchShowMTEvents(self):
        self.__showMTEvents = not(self.__showMTEvents)
        if self.__showMTEvents:
            self.__oldEventHook = g_player.getEventHook()
            g_player.setEventHook(self.__showMTEventHook)
        else:
            g_player.setEventHook(self.__oldEventHook)
            for id, touchVis in self.__touchViss.items():
                touchVis.unlink(True)
            self.__touchViss = {}

    def __showMTEventHook(self, event):
        if (isinstance(event, avg.TouchEvent) and event.source == avg.TOUCH and
                (event.type == avg.CURSORDOWN or event.type == avg.CURSORMOTION or
                 event.type == avg.CURSORUP)):
            try:
                touchVis = self.__touchViss[event.cursorid]
            except KeyError:
                touchVis = apphelpers.TouchVisualization(event, 
                        parent=self.__touchVisOverlay)
                self.__touchViss[event.cursorid] = touchVis
            if event.type == avg.CURSORDOWN:
                pass
            elif event.type == avg.CURSORMOTION:
                touchVis.move(event)
            elif event.type == avg.CURSORUP:
                touchVis.unlink(True)
                del self.__touchViss[event.cursorid]
        if self.__oldEventHook:
            return self.__oldEventHook()
        else:
            return False

    def __killNotifyNode(self):
        if self.__notifyNode:
            self.__notifyNode.unlink()
            self.__notifyNode = None

    def __screenshot(self):
        fnum = 0
        fnameTemplate = 'screenshot-%03d.png'
        while os.path.exists(fnameTemplate % fnum):
            fnum += 1
        
        try:
            g_player.screenshot().save('screenshot-%03d.png' % fnum)
        except RuntimeError:
            text = 'Cannot save snapshot file'
        else:
            text='Screenshot saved as ' + fnameTemplate % fnum
        
        self.__killNotifyNode()
        
        self.__notifyNode = avg.WordsNode(
            text=text, x=g_player.getRootNode().width - 50,
            y=g_player.getRootNode().height - 50, alignment='right', fontsize=20,
            sensitive=False, parent=g_player.getRootNode())
            
        g_player.setTimeout(2000, self.__killNotifyNode)
