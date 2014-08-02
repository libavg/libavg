# libavg - Media Playback Engine.
# Copyright (C) 2003-2014 Ulrich von Zadow
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
import math

from libavg import avg, Point2D, player
import graph
from mtemu import MTemu
import apphelpers


DEFAULT_RESOLUTION = (640, 480)

g_KbManager = apphelpers.KeyboardManager.get()


class AppStarter(object):
    '''Starts an AVGApp'''
    def __init__(self, appClass, resolution=DEFAULT_RESOLUTION,
            debugWindowSize=None, fakeFullscreen=False):

        resolution = Point2D(resolution)
        testMode = not 'AVG_DEPLOY' in os.environ

        if testMode and debugWindowSize is not None:
            debugWindowSize = Point2D(debugWindowSize)
        else:
            debugWindowSize = Point2D(0, 0)

        if fakeFullscreen:
            if os.name != 'nt':
                raise RuntimeError('Fakefullscreen is supported only on windows')
            elif not testMode:
                self.__enableFakeFullscreen()

            fullscreen = False
        else:
            fullscreen = not testMode

        player.enableMouse(not 'AVG_DISABLE_MOUSE' in os.environ)
        player.showCursor(testMode)
        self._setupBaseDivs(resolution)

        player.setResolution(
                fullscreen,
                int(debugWindowSize.x), int(debugWindowSize.y),
                0 # color depth
                )

        self._startApp(appClass)

    def _startApp(self, appClass):
        self._onBeforePlay()
        player.setTimeout(0, self._onStart)
        self._appInstance = appClass(self._appNode)
        g_KbManager.setup(
                self._appInstance.onKeyDown,
                self._appInstance.onKeyUp)

        self._setupDefaultKeys()

        self._appInstance.setStarter(self)
        player.play()
        self._appInstance.exit()
        g_KbManager.teardown()

    def _setupBaseDivs(self, resolution):
        player.loadString('''
<?xml version="1.0"?>
<!DOCTYPE avg SYSTEM "../../libavg/doc/avg.dtd">
<avg width="%s" height="%s">
</avg>''' % (resolution.x, resolution.y))

        rootNode = player.getRootNode()
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

    def __enableFakeFullscreen(self):
        player.setWindowPos(0, 0)
        player.setWindowFrame(False)


class AVGAppStarter(AppStarter):
    def __init__(self, *args, **kwargs):
        self.__graphs = []
        self._mtEmu = None
        self.__memGraph = None
        self.__vidMemGraph = None
        self.__frGraph = None
        self.__notifyNode = None
        self.__debugTouchVisOverlay = None

        super(AVGAppStarter, self).__init__(*args, **kwargs)

    def _setupDefaultKeys(self):
        super(AVGAppStarter, self)._setupDefaultKeys()
        g_KbManager.bindKey('o', self.__dumpObjects, 'Dump objects')
        g_KbManager.bindKey('m', self.showMemoryUsage, 'Show memory usage graph')

        g_KbManager.bindKey('f', self.showFrameRate, 'Show framerate graph')
        g_KbManager.bindKey('t', self.__switchMtemu, 'Activate multitouch emulation')
        g_KbManager.bindKey('e', self.__switchShowMTEvents, 'Show multitouch events')
        g_KbManager.bindKey('s', self.__screenshot, 'Take screenshot')

    def _onStart(self):
        try:
            player.getVideoMemUsed()
            g_KbManager.bindKey('v', self.showVideoMemoryUsage,
                    'Show video memory usage graph')
        except RuntimeError:
            # Video memory query not supported.
            pass

        AppStarter._onStart(self)

    def __dumpObjects(self):
        gc.collect()
        testHelper = player.getTestHelper()
        testHelper.dumpObjects()
        print 'Num anims: ', avg.getNumRunningAnims()
        print 'Num python objects: ', len(gc.get_objects())

    def showMemoryUsage(self):
        if self.__memGraph:
            self.__memGraph.unlink(True)
            self.__graphs.remove(self.__memGraph)
            self.__memGraph = None
        else:
            size = (self._appNode.width, self._appNode.height/6.0)
            self.__memGraph = graph.AveragingGraph(title = 'Memory Usage',
                    getValue = player.getMemoryUsage, parent=player.getRootNode(),
                    size=size)
            self.__graphs.append(self.__memGraph)
        self.__positionGraphs()

    def showVideoMemoryUsage(self):
        if self.__vidMemGraph:
            self.__vidMemGraph.unlink(True)
            self.__graphs.remove(self.__vidMemGraph)
            self.__vidMemGraph = None
        else:
            size = (self._appNode.width, self._appNode.height/6.0)
            self.__vidMemGraph = graph.AveragingGraph(title='Video Memory Usage',
                    getValue=player.getVideoMemUsed, parent=player.getRootNode(),
                    size=size)
            self.__graphs.append(self.__vidMemGraph)
        self.__positionGraphs()

    def showFrameRate(self):
        if self.__frGraph:
            self.__frGraph.unlink(True)
            self.__graphs.remove(self.__frGraph)
            self.__frGraph = None
        else:
            size = (self._appNode.width, self._appNode.height/6.0)
            self.__frGraph = graph.SlidingGraph(title = 'Time per Frame',
                    getValue = player.getFrameTime, parent = self._appNode, size=size)
            self.__graphs.append(self.__frGraph)
        self.__positionGraphs()

    def __positionGraphs(self):
        ypos = 10
        for gr in self.__graphs:
            gr.y = ypos
            ypos += gr.height + 10

    def __switchMtemu(self):
        if self._mtEmu is None:
            self._mtEmu = MTemu()
            g_KbManager.bindKey('left shift', self._mtEmu.toggleDualTouch,
                    'Toggle Multitouch Emulation')
            g_KbManager.bindKey('right shift', self._mtEmu.toggleDualTouch,
                    'Toggle Multitouch Emulation')
            g_KbManager.bindKey('left ctrl', self._mtEmu.toggleSource,
                    'Toggle Touch Source')
            g_KbManager.bindKey('right ctrl', self._mtEmu.toggleSource,
                    'Toggle Touch Source')
        else:
            self._mtEmu.deinit()
            g_KbManager.unbindKey('left ctrl')
            g_KbManager.unbindKey('right ctrl')
            g_KbManager.unbindKey('left shift')
            g_KbManager.unbindKey('right shift')

            del self._mtEmu
            self._mtEmu = None

    def __switchShowMTEvents(self):
        if self.__debugTouchVisOverlay is None:
            rootNode = player.getRootNode()
            self.__debugTouchVisOverlay = apphelpers.TouchVisualizationOverlay(
                    isDebug=True, visClass=apphelpers.DebugTouchVisualization,
                    size=self._appNode.size, parent=rootNode)
        else:
            self.__debugTouchVisOverlay.unlink(True)
            del self.__debugTouchVisOverlay
            self.__debugTouchVisOverlay = None

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
            player.screenshot().save('screenshot-%03d.png' % fnum)
        except RuntimeError:
            text = 'Cannot save snapshot file'
        else:
            text = 'Screenshot saved as ' + fnameTemplate % fnum

        self.__killNotifyNode()

        self.__notifyNode = avg.WordsNode(
            text=text, x=player.getRootNode().width - 50,
            y=player.getRootNode().height - 50, alignment='right', fontsize=20,
            sensitive=False, parent=player.getRootNode())

        player.setTimeout(2000, self.__killNotifyNode)


class AVGMTAppStarter(AVGAppStarter):

    def __init__(self, *args, **kwargs):
        self.__touchVisOverlay = None
        super(AVGMTAppStarter, self).__init__(*args, **kwargs)

    def setTouchVisualization(self, visClass):
        if not(self.__touchVisOverlay is None):
            self.__touchVisOverlay.unlink(True)
            del self.__touchVisOverlay
            self.__touchVisOverlay = None
        if not(visClass is None):
            rootNode = player.getRootNode()
            self.__touchVisOverlay = apphelpers.TouchVisualizationOverlay(
                    isDebug=False, visClass=visClass, size=self._appNode.size,
                    parent=rootNode)

    def toggleTrackerImage(self):
        if self.__showTrackerImage:
            self.hideTrackerImage()
        else:
            self.showTrackerImage()

    def showTrackerImage(self):
        if self.__showTrackerImage:
            return
        self.__showTrackerImage = True
        self.__updateTrackerImageInterval = \
                player.subscribe(player.ON_FRAME, self.__updateTrackerImage)
        self.__trackerImageNode.opacity = 1
        self.tracker.setDebugImages(False, True)

    def hideTrackerImage(self):
        if not self.__showTrackerImage:
            return
        self.__showTrackerImage = False
        if self.__updateTrackerImageInterval:
            player.clearInterval(self.__updateTrackerImageInterval)
            self.__updateTrackerImageInterval = None
        self.__trackerImageNode.opacity = 0
        self.tracker.setDebugImages(False, False)

    def __updateTrackerImage(self):
        def transformPos((x,y)):
            if self.trackerFlipX:
                x = 1 - x
            if self.trackerFlipY:
                y = 1 - y
            return (x, y)

        fingerBitmap = self.tracker.getImage(avg.IMG_FINGERS)
        node = self.__trackerImageNode
        node.setBitmap(fingerBitmap)
        node.pos = self.tracker.getDisplayROIPos()
        node.size = self.tracker.getDisplayROISize()

        grid = node.getOrigVertexCoords()
        grid = [ [ transformPos(pos) for pos in line ] for line in grid]
        node.setWarpedVertexCoords(grid)

    def _onStart(self):
        from camcalibrator import Calibrator

        # we must add the tracker first, calibrator depends on it
        try:
            player.enableMultitouch()
        except RuntimeError, err:
            avg.logger.warning(str(err))

        self.tracker = player.getTracker()

        if self.tracker:
            if Calibrator:
                self.__calibratorNode = player.createNode('div',{
                    'opacity': 0,
                    'active': False,
                    })
                rootNode = player.getRootNode()
                rootNode.appendChild(self.__calibratorNode)
                self.__calibratorNode.size = rootNode.size
                self.__calibrator = Calibrator(self.__calibratorNode, appStarter=self)
                self.__calibrator.setOnCalibrationSuccess(self.__onCalibrationSuccess)
                self.__calibrator.init()
            else:
                self.__calibrator = None

            self.__showTrackerImage = False
            self.__updateTrackerImageInterval = None
            self.__trackerImageNode = player.createNode('image', {'sensitive': False})
            player.getRootNode().appendChild(self.__trackerImageNode)

            self.__updateTrackerImageFixup()

            g_KbManager.bindKey('h', self.tracker.resetHistory, 'RESET tracker history')
            g_KbManager.bindKey('d', self.toggleTrackerImage, 'toggle tracker image')

            if self.__calibrator:
                g_KbManager.bindKey('c', self.__enterCalibrator, 'enter calibrator')
        AVGAppStarter._onStart(self)

    def __updateTrackerImageFixup(self):
        # finger bitmap might need to be rotated/flipped
        trackerAngle = float(self.tracker.getParam('/transform/angle/@value'))
        angle = round(trackerAngle/math.pi) * math.pi
        self.__trackerImageNode.angle = angle
        self.trackerFlipX = (float(self.tracker.getParam('/transform/displayscale/@x'))
                < 0)
        self.trackerFlipY = (float(self.tracker.getParam('/transform/displayscale/@y'))
                < 0)

    def __onCalibrationSuccess(self):
        self.__updateTrackerImageFixup()

    def __enterCalibrator(self):

        def leaveCalibrator():
            g_KbManager.unbindKey('e')
            self._activeApp = self._appInstance
            self._appInstance.enter()
            self.__calibrator.leave()
            self._appNode.opacity = 1
            self._appNode.active = True
            self.__calibratorNode.opacity = 0
            self.__calibratorNode.active = False

        if self.__calibrator.isRunning():
            print "calibrator already running!"
            return

        self._activeApp = self.__calibrator
        self.__calibrator.enter()
        g_KbManager.bindKey('e', leaveCalibrator, 'leave Calibrator')
        self._appInstance.leave()
        self.__calibratorNode.opacity = 1
        self.__calibratorNode.active = True
        self._appNode.opacity = 0
        self._appNode.active = False
