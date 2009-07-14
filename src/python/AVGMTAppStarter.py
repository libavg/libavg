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
import math
from libavg import avg
from AVGAppStarter import AVGAppStarter
try:
    from alib.calibrator.CamCalibrator import Calibrator
except ImportError:
    Calibrator = None
    #from .camcalibrator import Calibrator

g_player = avg.Player.get()

class AVGMTAppStarter (AVGAppStarter):
    def __init__(self, *args, **kwargs):
        AVGMTAppStarter.instance = self
        super(AVGMTAppStarter, self).__init__(*args, **kwargs)

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
                g_player.setOnFrameHandler(self.__updateTrackerImage)
        self.__trackerImageNode.opacity = 1
        self.tracker.setDebugImages(False, True)

    def hideTrackerImage(self):
        if not self.__showTrackerImage:
            return
        self.__showTrackerImage = False
        if self.__updateTrackerImageInterval:
            g_player.clearInterval(self.__updateTrackerImageInterval)
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
        node.size = g_player.getRootNode().size

        grid = node.getOrigVertexCoords()
        grid = [ [ transformPos(pos) for pos in line ] for line in grid]
        node.setWarpedVertexCoords(grid)

    def _onBeforePlay(self):
        # we must add the tracker first, calibrator depends on it
        self.tracker = g_player.addTracker()

        if Calibrator:
            self.__calibratorNode = g_player.createNode('div',{
                'opacity': 0,
                'active': False,
                })
            rootNode = g_player.getRootNode()
            rootNode.appendChild(self.__calibratorNode)
            self.__calibratorNode.size = rootNode.size
            self.__calibrator = Calibrator(self.__calibratorNode)
            self.__calibrator.setOnCalibrationSuccess(self.__onCalibrationSuccess)
        else:
            self.__calibrator = None

        self.__showTrackerImage = False
        self.__updateTrackerImageInterval = None
        self.__trackerImageNode = g_player.createNode('image', {'sensitive': False})
        g_player.getRootNode().appendChild(self.__trackerImageNode)

        self.__updateTrackerImageFixup()

        self.bindKey('h', self.tracker.resetHistory)
        self.bindKey('d', self.toggleTrackerImage)
        if self.__calibrator:
            self.bindKey('c', self.__enterCalibrator)

    def __updateTrackerImageFixup(self):
        # finger bitmap might need to be rotated/flipped
        trackerAngle = float(self.tracker.getParam('/transform/angle/@value'))
        angle = round(trackerAngle/math.pi) * math.pi
        self.__trackerImageNode.angle = angle
        self.trackerFlipX = float(self.tracker.getParam('/transform/displayscale/@x')) < 0
        self.trackerFlipY = float(self.tracker.getParam('/transform/displayscale/@y')) < 0

    def __onCalibrationSuccess(self):
        self.__updateTrackerImageFixup()

    def __enterCalibrator(self):
        def leaveCalibrator():
            self.unbindKey('e')
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

        self.bindKey('e', leaveCalibrator)
        self._activeApp = self.__calibrator
        self.__calibrator.enter()
        self._appInstance.leave()
        self.__calibratorNode.opacity = 1
        self.__calibratorNode.active = True
        self._appNode.opacity = 0
        self._appNode.active = False

AVGMTAppStarter.instance = None

