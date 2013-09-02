# libavg - Media Playback Engine.
# Copyright (C) 2003-2011 Ulrich von Zadow
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

from libavg import avg, player

import apphelpers

g_KbManager = apphelpers.KeyboardManager.get()


class CoordCalibrator(object):
    def __init__(self, calibrationTerminatedCb):
        self.__calibrationTerminatedCb = calibrationTerminatedCb
        self.__CurPointIndex = 0
        self.__CPPCal = player.getTracker().startCalibration()
        self.__LastCenter = None
        self.__NumMessages = 0
        self._mycursor = None
        mainNode = player.getElementByID("cal_coordcalibrator")
        mainNode.active = True
        mainNode.opacity = 1
        mainNode.setEventHandler(avg.Event.CURSOR_DOWN, avg.Event.TOUCH, 
                self.__onTouchDown)
        mainNode.setEventHandler(avg.Event.CURSOR_MOTION, avg.Event.TOUCH, 
                self.__onTouchMove)
        mainNode.setEventHandler(avg.Event.CURSOR_UP, avg.Event.TOUCH, self.__onTouchUp)
        self.__crosshair = player.getElementByID("cal_crosshair")
        self.__feedback = player.getElementByID("cal_feedback")
        self.__feedback.opacity = 0
        self.__addMessage("Starting calibration.")
        self.__moveMarker()
        
        g_KbManager.push()
        g_KbManager.bindKey('space', self.__nextPoint, 'sample next point')
        g_KbManager.bindKey('a', self.__abortCalibration, 'abort calibration')
        
    def __endCalibration(self, isSuccessful):
        player.getElementByID("cal_coordcalibrator").active = False
        player.getElementByID("cal_coordcalibrator").opacity = 0
        MsgsNode = player.getElementByID("cal_messages")
        for i in range(0, MsgsNode.getNumChildren()):
            MsgsNode.removeChild(0)
        
        g_KbManager.pop()
        self.__calibrationTerminatedCb(isSuccessful)
    
    def __nextPoint(self):
        if self.__LastCenter:
            self.__CPPCal.setCamPoint(self.__LastCenter)
            self.__addMessage ("  Using: %(x).2f, %(y).2f" %
                    { "x": self.__LastCenter[0], "y": self.__LastCenter[1]})
            self._mycursor = None
            self.__LastCenter = None
            
        hasNextPoint = self.__CPPCal.nextPoint()

        if not hasNextPoint:
            # Note: may raise RuntimeError. A rollback doesn't appear to be possible,
            # which means crashing here is safer than handling the exception
            player.getTracker().endCalibration()
            self.__endCalibration(True)
        else:
            self.__CurPointIndex += 1
            self.__moveMarker()
    
    def __abortCalibration(self):
        player.getTracker().abortCalibration()
        self.__endCalibration(False)
        
    def __moveMarker(self):
        self.__crosshair.x, self.__crosshair.y = self.__CPPCal.getDisplayPoint()
        self.__crosshair.x, self.__crosshair.y = self.__feedback.x, self.__feedback.y = \
                (self.__crosshair.x-7, self.__crosshair.y-7)
        self.__addMessage("Calibrating point "+str(self.__CurPointIndex))

    def __addMessage(self, text):
        MsgsNode = player.getElementByID("cal_messages")
        if self.__NumMessages > 38:
            for i in range(0, MsgsNode.getNumChildren()-1):
                MsgsNode.getChild(i).text = MsgsNode.getChild(i+1).text
            MsgsNode.removeChild(MsgsNode.getNumChildren()-1)
        else:
            self.__NumMessages += 1
        Node = player.createNode(
                "<words fontsize='10' font='Eurostile' color='00FF00'/>")
        Node.x = 0
        Node.y = self.__NumMessages*13
        Node.text = text
        MsgsNode.appendChild(Node)

    def __onTouchDown(self, Event):
        if Event.source != avg.Event.TOUCH:
            return
        if not self._mycursor:
            self._mycursor = Event.cursorid
        else:
            return
        self.__LastCenter = Event.center
        self.__addMessage("  Touch at %(x).2f, %(y).2f" % {
                "x": Event.center[0], "y": Event.center[1]})
        self.__feedback.opacity = 1

    def __onTouchMove(self,Event):
        if Event.source != avg.Event.TOUCH:
            return
        if self._mycursor == Event.cursorid:
            self.__LastCenter = Event.center

    def __onTouchUp(self, Event):
        if Event.source != avg.Event.TOUCH:
            return
        self.__addMessage("touchup")
        self.__feedback.opacity = 0
        if self._mycursor:
            self._mycursor = None
        else:
            return
