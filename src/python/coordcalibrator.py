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
#
# Released with permission from Archimedes-Solutions GmbH

from libavg import avg

gPlayer = None

class CoordCalibrator:
    def __init__(self, Tracker, Player):
        global gPlayer
        gPlayer = Player
        self.__Tracker = Tracker
        self.__CurPointIndex = 0
        self.__CPPCal = self.__Tracker.startCalibration()
        self.__LastCenter = None
        self.__NumMessages = 0
        self._mycursor = None
        mainNode = gPlayer.getElementByID("cal_coordcalibrator")
        mainNode.active = True
        mainNode.opacity = 1
        mainNode.setEventHandler(avg.CURSORDOWN, avg.TOUCH, self.onTouchDown)
        mainNode.setEventHandler(avg.CURSORMOTION, avg.TOUCH, self.onTouchMove)
        mainNode.setEventHandler(avg.CURSORUP, avg.TOUCH, self.onTouchUp)
        self.__crosshair = gPlayer.getElementByID("cal_crosshair")
        self.__feedback = gPlayer.getElementByID("cal_feedback")
        self.__feedback.opacity = 0
        self.__addMessage("Starting calibration.")
        self.__moveMarker()
        
        
    def endCalibration(self):
                
        if gPlayer != None:
            gPlayer.getElementByID("cal_coordcalibrator").active = False
            gPlayer.getElementByID("cal_coordcalibrator").opacity = 0
            MsgsNode = gPlayer.getElementByID("cal_messages")
            for i in range(0, MsgsNode.getNumChildren()):
                MsgsNode.removeChild(0)

    def __moveMarker(self):
        self.__crosshair.x, self.__crosshair.y = self.__CPPCal.getDisplayPoint()
        self.__crosshair.x, self.__crosshair.y = self.__feedback.x, self.__feedback.y = \
                (self.__crosshair.x-7, self.__crosshair.y-7)
        self.__addMessage("Calibrating point "+str(self.__CurPointIndex))

    def __addMessage(self, text):
        MsgsNode = gPlayer.getElementByID("cal_messages")
        if self.__NumMessages > 38:
            for i in range(0, MsgsNode.getNumChildren()-1):
                MsgsNode.getChild(i).text = MsgsNode.getChild(i+1).text
            MsgsNode.removeChild(MsgsNode.getNumChildren()-1)
        else:
            self.__NumMessages += 1
        Node = gPlayer.createNode(
                "<words fontsize='10' font='Eurostile' color='00FF00'/>")
        Node.x = 0
        Node.y = self.__NumMessages*13
        Node.text = text
        MsgsNode.appendChild(Node)

    def onTouchDown(self, Event):
        if Event.source != avg.TOUCH:
            return
        if not self._mycursor:
            self._mycursor = Event.cursorid
        else:
            return
        self.__LastCenter = Event.center
        self.__addMessage("  Touch at %(x).2f, %(y).2f" % { "x": Event.center[0], "y": Event.center[1]})
        self.__feedback.opacity = 1

    def onTouchMove(self,Event):
        if Event.source != avg.TOUCH:
            return
        if self._mycursor == Event.cursorid:
            self.__LastCenter = Event.center

    def onTouchUp(self, Event):
        if Event.source != avg.TOUCH:
            return
        self.__addMessage("touchup")
        self.__feedback.opacity = 0
        if self._mycursor:
            self._mycursor = None
        else:
            return

    def onKeyUp(self, Event):
        if Event.keystring == "space":
            if self.__LastCenter:
                self.__CPPCal.setCamPoint(self.__LastCenter)
                self.__addMessage ("  Using: %(x).2f, %(y).2f" %
                        { "x": self.__LastCenter[0], "y": self.__LastCenter[1]})
                self._mycursor = None
                self.__LastCenter = None
            Ok = self.__CPPCal.nextPoint()
            self.__CurPointIndex += 1
            if not(Ok):
                self.__Tracker.endCalibration()
                self.endCalibration()
            else:
                self.__moveMarker()
            return Ok
        elif Event.keystring == "a":
            self.__Tracker.abortCalibration()
            self.endCalibration()
            return False
        return True
    
