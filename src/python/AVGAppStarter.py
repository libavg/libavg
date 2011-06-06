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
g_kbManager = apphelpers.KeyboardManager.get()

class AppStarter(object):
    '''Starts an AVGApp'''
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

        self._setupBaseDivs(resolution)

        g_player.showCursor(testMode)

        if fakeFullscreen:
            fullscreen = False
        else:
            fullscreen = not testMode

        g_player.setResolution(
                fullscreen,
                int(debugWindowSize.x), int(debugWindowSize.y),
                0 # color depth
                )

        self._startApp(appClass)

    def _startApp(self, appClass):
        self._onBeforePlay()
        g_player.setTimeout(0, self._onStart)
        self._appInstance = appClass(self._appNode)
        g_kbManager.setup(
                self._appInstance.onKeyDown,
                self._appInstance.onKeyUp)

        self._setupDefaultKeys()

        self._appInstance.setStarter(self)
        g_player.play()
        self._appInstance.exit()

    def _setupBaseDivs(self, resolution):
        g_player.loadString('''
<?xml version="1.0"?>
<!DOCTYPE avg SYSTEM "../../libavg/doc/avg.dtd">
<avg width="%s" height="%s">
</avg>''' % (resolution.x, resolution.y))

        rootNode = g_player.getRootNode()
        self._appNode = avg.DivNode(opacity=0, sensitive=False,
                size=rootNode.size, parent=rootNode)

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
        w = findWindow('AVG Renderer')
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
        self.__showMTEvents = False

        super(AVGAppStarter, self).__init__(*args, **kwargs)

    def _setupDefaultKeys(self):
        super(AVGAppStarter, self)._setupDefaultKeys()
        g_kbManager.bindKey('o', self.__dumpObjects, 'Dump objects')
        g_kbManager.bindKey('m', self.__showMemoryUsage, 'Show memory usage')
        g_kbManager.bindKey('f', self.__showFrameRateUsage, 'Show frameTime usage')
        g_kbManager.bindKey('.', self.__switchClickTest, 'Start clicktest')
        g_kbManager.bindKey('t', self.__switchMtemu, 'Activate multitouch emulation')
        g_kbManager.bindKey('e', self.__switchShowMTEvents, 'Show multitouch events')
        g_kbManager.bindKey('s', self.__screenshot, 'Take screenshot')

    def _onBeforePlay(self):
        super(AVGAppStarter, self)._onBeforePlay()
        rootNode = g_player.getRootNode()
        self.__touchViss = {}
        self.__touchVisOverlay = avg.DivNode(sensitive=False,
                size=rootNode.size,
                parent=rootNode)
        self.__initClickTest()

    def __initClickTest(self):
        if ClickTest:
            self._clickTest = ClickTest(self._appNode, multiClick=False)
        else:
            self._clickTest = None

    def __onTouchDown(self, event):
        touchVis = apphelpers.TouchVisualization(event,
                        parent=self.__touchVisOverlay)
        self.__touchViss[event.cursorid] = touchVis

    def __onTouchUp(self, event):
        if event.cursorid in self.__touchViss:
            self.__touchViss[event.cursorid].unlink(True)
            self.__touchViss[event.cursorid] = None
            del self.__touchViss[event.cursorid]

    def __onTouchMotion(self, event):
        if event.cursorid in self.__touchViss:
            self.__touchViss[event.cursorid].move(event)

    def __dumpObjects(self):
        gc.collect()
        testHelper = g_player.getTestHelper()
        testHelper.dumpObjects()
        print 'Num anims: ', avg.getNumRunningAnims()
        print 'Num python objects: ', len(gc.get_objects())

    def __showMemoryUsage(self):
        if self.__showingMemGraph:
            self.__memGraph.delete()
            self.__memGraph = None
            self.__graphs = self.__graphs -1
            if(self.__graphs == 1 ):
                self.__frGraph.setYpos(10)
        else:
            self.__memGraph = graph.MemGraph('Memory Graph',
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
            self.__frGraph = graph.FrameRateGraph('FrameTime Graph',
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
            g_kbManager.bindKey('left shift', self._mtEmu.toggleDualTouch,
                    'Toggle Multitouch Emulation')
            g_kbManager.bindKey('right shift', self._mtEmu.toggleDualTouch,
                    'Toggle Multitouch Emulation')
            g_kbManager.bindKey('left ctrl', self._mtEmu.toggleSource,
                    'Toggle Touch Source')
            g_kbManager.bindKey('right ctrl', self._mtEmu.toggleSource,
                    'Toggle Touch Source')

        else:
            self._mtEmu.deinit()
            g_kbManager.unbindKey('left ctrl')
            g_kbManager.unbindKey('right ctrl')
            g_kbManager.unbindKey('left shift')
            g_kbManager.unbindKey('right shift')

            del self._mtEmu
            self._mtEmu = None

    def __switchShowMTEvents(self):
        rootNode = g_player.getRootNode()
        self.__showMTEvents = not(self.__showMTEvents)
        if self.__showMTEvents:
            self.__touchVisOverlay = avg.DivNode(sensitive=False, size=self._appNode.size,
                    parent=rootNode, elementoutlinecolor='FFFFAA')
            avg.RectNode(parent = self.__touchVisOverlay, size=self._appNode.size,
                    fillopacity=0.2, fillcolor='000000')
            rootNode.connectEventHandler(avg.CURSORUP, avg.TOUCH | avg.TRACK,
                    self, self.__onTouchUp)
            rootNode.connectEventHandler(avg.CURSORDOWN, avg.TOUCH | avg.TRACK,
                    self, self.__onTouchDown)
            rootNode.connectEventHandler(avg.CURSORMOTION, avg.TOUCH | avg.TRACK,
                    self, self.__onTouchMotion)
        else:
            rootNode.disconnectEventHandler(self, self.__onTouchDown)
            rootNode.disconnectEventHandler(self, self.__onTouchUp)
            rootNode.disconnectEventHandler(self, self.__onTouchMotion)
            self.__touchVisOverlay.unlink(True)

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
            text = 'Screenshot saved as ' + fnameTemplate % fnum

        self.__killNotifyNode()

        self.__notifyNode = avg.WordsNode(
            text=text, x=g_player.getRootNode().width - 50,
            y=g_player.getRootNode().height - 50, alignment='right', fontsize=20,
            sensitive=False, parent=g_player.getRootNode())

        g_player.setTimeout(2000, self.__killNotifyNode)
