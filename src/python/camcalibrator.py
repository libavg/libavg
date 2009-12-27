# -*- coding: utf-8 -*-
#
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


import os
from . import avg, Point2D
from .trackerhelper import TrackerImageFlipper
from .AVGApp import AVGApp
from .AVGAppUtil import getMediaDir
from collections import deque

datadir = getMediaDir(__file__, 'data')


g_player = avg.Player.get()
g_tracker = None
g_trackerParameters = [
        {'nameFmt':"Track Threshold: %.0f",
            'path':"/tracker/track/threshold/@value",
            'min':1, 'max':255, 'increment':1},
        {'nameFmt':"Touch Threshold: %.0f",
            'path':"/tracker/touch/threshold/@value",
            'min':1, 'max':255, 'increment':1},
        {'nameFmt':"Brightness: %.0f",
            'path':"/camera/brightness/@value",
            'min':1, 'max':255, 'increment':1},
        {'nameFmt':"Shutter: %.0f",
            'path':"/camera/shutter/@value",
            'min':1, 'max':533, 'increment':1},
        {'nameFmt':"Gain: %.0f",
            'path':"/camera/gain/@value",
            'min':16, 'max':64, 'increment':1},
        ]

class CamCalibrator(AVGApp):
    def __init__(self, parentNode, appStarter):
        AVGApp.__init__(self, parentNode)

    def init(self):
        global g_tracker
        print "CAMCALIBRATOR INIT"
        print "CAMCALIBRATOR INIT"
        print "CAMCALIBRATOR INIT"
        print "CAMCALIBRATOR INIT"
        print "CAMCALIBRATOR INIT"
        g_tracker = g_player.getTracker()
        self._parentNode.mediadir = datadir
        self.__trackerImageFlipper = TrackerImageFlipper()
        avgfd = open(os.path.join(datadir, "camcalibrator.avg"))
        avg_contents = avgfd.read()
        avgfd.close()
        self.__mainNode = g_player.createNode(avg_contents)
        self._parentNode.appendChild(self.__mainNode)

        self.__curParam = 0
        self.__saveIndex = 0
        self.__showCoordCalibrator = False

        coordDiv = g_player.getElementByID("coordcalibrator")
        coordDiv.opacity = 0
        coordDiv.active = False
        self.__coordCalibrator = CoordCalibrator(coordDiv)
        self.__coordCalibrator.init()
        self.__onCalibrationSuccess = lambda: None

    def _enter(self):
        self.__onFrameHandler = g_player.setOnFrameHandler(self.__updateBitmaps)
        g_tracker.setDebugImages(True, True)
        self.__displayParams() # XXX

    def _leave(self):
        g_player.clearInterval(self.__onFrameHandler)
        # XXX: extremely ugly kludge! change libavg toggleTrackerImage() api?
        g_tracker.setDebugImages(False, False)
        from .AVGMTAppStarter import AVGMTAppStarter # kludge to avoid circular import
        AVGMTAppStarter.instance.toggleTrackerImage()
        AVGMTAppStarter.instance.toggleTrackerImage()

    def __getParam(self, Path):
        return int(g_tracker.getParam(Path))

    def __setParam(self, Path, Val):
        g_tracker.setParam(Path, str(Val))

    def __displayParams(self):
        for (i, param) in enumerate(g_trackerParameters):
            node = g_player.getElementByID("param%u" % i)
            val = float(self.__getParam(param['path']))
            node.text = param['nameFmt'] % val
            if self.__curParam == i:
                node.color = "FFFFFF"
            else:
                node.color = "A0A0FF"

    def __changeParam(self, Change):
        curParam = g_trackerParameters[self.__curParam]
        Val = self.__getParam(curParam['path'])
        Val += Change*curParam['increment']
        if Val < curParam['min']:
            Val = curParam['min']
        if Val > curParam['max']:
            Val = curParam['max']
        self.__setParam(curParam['path'], Val)

    def __updateBitmaps(self):
        for nodeID, imageID in (
                ("camera", avg.IMG_CAMERA),
                ("distorted", avg.IMG_DISTORTED),
                ("nohistory", avg.IMG_NOHISTORY),
                ("histogram", avg.IMG_HISTOGRAM),
                ("bkgnd", avg.IMG_DISTORTED),
                ):
            node = g_player.getElementByID(nodeID)
            oldSize = node.size
            self.__trackerImageFlipper.loadTrackerImage(
                    node = node,
                    imageID = imageID)
            node.size = oldSize

    def __saveImages(self):
        self.__saveIndex += 1
        for (imageID, name) in [
                (avg.IMG_CAMERA, "camera"),
                (avg.IMG_DISTORTED,"distorted"),
                (avg.IMG_NOHISTORY,"nohistory"),
                (avg.IMG_HIGHPASS,"highpass"),
                (avg.IMG_FINGERS,"fingers"),
                ]:
            bitmap = g_tracker.getImage(imageID)
            bitmap.save("img_%u_%s.png" % (self.__saveIndex, name))

    def setOnCalibrationSuccess(self, callback):
        self.__onCalibrationSuccess = callback

    def toggleCoordCalibrator(self):
        self.__showCoordCalibrator ^= True
        coordDiv = g_player.getElementByID("coordcalibrator")
        if self.__showCoordCalibrator: # enabling
            coordDiv.opacity = 1
            coordDiv.active = True
            def afterCoordCal():
                self.toggleCoordCalibrator()
                self.__onCalibrationSuccess()
                self.__trackerImageFlipper.readConfig()
            self.__coordCalibrator.enter(onLeave = afterCoordCal)
        else: # disabling
            coordDiv.opacity = 0
            coordDiv.active = False

    def onKey(self, event):
        if self.__showCoordCalibrator:
            if self.__coordCalibrator.onKey(event):
                return True
        if event.keystring == "c":
            self.toggleCoordCalibrator()
        elif event.keystring == "up":
            if self.__curParam > 0:
                self.__curParam -= 1
        elif event.keystring == "down":
            if self.__curParam < len(g_trackerParameters)-1:
                self.__curParam += 1
        elif event.keystring == "left":
            self.__changeParam(-1)
        elif event.keystring == "right":
            self.__changeParam(1)
        elif event.keystring == "page up":
            self.__changeParam(10)
        elif event.keystring == "page down":
            self.__changeParam(-10)
        elif event.keystring == "w":
            self.__saveImages()
            print "Images saved."
        elif event.keystring == "s":
            g_tracker.saveConfig()
            print "tracker config saved."
        elif event.keystring == "d":
            from .AVGMTAppStarter import AVGMTAppStarter # kludge to avoid circular import
            AVGMTAppStarter.instance.toggleTrackerImage()
            g_tracker.setDebugImages(True, True)
        else:
            return False
        self.__displayParams() # XXX
        return True

class MessageBox(object):
    def __init__(self, parentNode, pos, maxLines):
        self.__divNode = g_player.createNode('div',{})
        self.__divNode.pos = pos
        parentNode.appendChild(self.__divNode)
        self.__nodes = deque()
        self.__maxLines = maxLines
        self.__nodeOffset = 13

    def clear(self):
        for node in self.__nodes:
            node.unlink()
        self.__nodes.clear()

    def write(self, msg):
        if len(self.__nodes) > self.__maxLines:
            oldestLine = self.__nodes.popleft()
            oldestLine.unlink()
            for node in self.__nodes:
                node.y -= self.__nodeOffset

        y = len(self.__nodes) * self.__nodeOffset
        node = g_player.createNode('words', {
            'fontsize': 10,
            'text': msg,
            'font': 'Eurostile',
            'color': '00ff00',
            'pos': Point2D(0, y),
            })
        self.__divNode.appendChild(node)
        self.__nodes.append(node)


class CoordCalibrator(AVGApp):
    def init(self):
        self._parentNode.mediadir = datadir
        self.__msgBox = MessageBox(parentNode = self._parentNode,
                pos = Point2D(100, 100), maxLines = 38)
        self.__crosshair = g_player.createNode("image",
                {'href': 'crosshair.png'})
        self._parentNode.appendChild(self.__crosshair)
        for type_ in avg.CURSORDOWN, avg.CURSORMOTION, avg.CURSORUP:
            self._parentNode.setEventHandler(type_, avg.TOUCH, self.__onCursorEvent)

    def _enter(self):
        self.__savedShutter = g_tracker.getParam("/camera/shutter/@value")
        g_tracker.setParam("/camera/shutter/@value", "11")
        self.__savedGain = g_tracker.getParam("/camera/gain/@value")
        g_tracker.setParam("/camera/gain/@value", "23")

        # start after modifed tracker settings take effect
        g_player.setTimeout(0, self.__startCalibration)

    def __startCalibration(self):
        self.__CPPCal = g_tracker.startCalibration()
        self.__msgBox.write("Starting calibration.")
        self.__curPointIndex = 0
        self.__lastCenter = None
        self.__moveMarker()

    def _leave(self):
        g_tracker.setParam("/camera/shutter/@value", self.__savedShutter)
        g_tracker.setParam("/camera/gain/@value", self.__savedGain)
        self.__msgBox.clear()

    def __nextPoint(self):
        if self.__lastCenter:
            self.__msgBox.write("Using %.2f, %.2f" % self.__lastCenter)
            self.__CPPCal.setCamPoint(self.__lastCenter)
        self.__lastCenter = None
        calibrationDone = not self.__CPPCal.nextPoint()
        self.__moveMarker()
        if calibrationDone:
            g_tracker.endCalibration()
            self.leave()

    def __moveMarker(self):
        newPos =  self.__CPPCal.getDisplayPoint()
        self.__crosshair.pos = Point2D(newPos) - self.__crosshair.size / 2
        self.__curPointIndex += 1
        self.__msgBox.write("Calibrating point "+str(self.__curPointIndex))

    def __onCursorEvent(self, event):
        self.__lastCenter = event.center[0], event.center[1]
        self.__msgBox.write("cursor event with center %.2f, %.2f" % self.__lastCenter)

    def onKey(self, event):
        if event.keystring == "space":
            self.__nextPoint()
        elif event.keystring == "e":
            g_tracker.abortCalibration()
            self.leave()
        return True # ignore all other keys!

