# -*- coding: utf-8 -*-
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

from libavg import avg

from helper import *

g_Player = avg.Player.get()

class Recognizer(object):

    def __init__(self, node, eventSource, initialEvent):
        self._node = node
        self.__eventSource = eventSource
       
        self.__setEventHandler() 
        self.__isEnabled = True
        self.__isActive = False
        if initialEvent:
            self._onDown(initialEvent)

    def enable(self, isEnabled):
        if isEnabled != self.__isEnabled:
            self.__isEnabled = isEnabled
            if isEnabled:
                self.__setEventHandler()
            else:
                if self.__isActive:
                    self.__contact.disconnectListener(self.__listenerid)
                self.__isActive = False
                self._node.disconnectEventHandler(self)

    def _onDown(self, event):
        if not(self.__isActive):
            self.__isActive = True
            self.__listenerid = event.contact.connectListener(self._onMotion, self._onUp)
            self.__contact = event.contact
            return self._handleDown(event)

    def _onMotion(self, event):
        self._handleMove(event)

    def _onUp(self, event):
        assert(self.__isActive)
        self.__isActive = False
        event.contact.disconnectListener(self.__listenerid)
        self._handleUp(event)

    def _abort(self, event):
        event.contact.disconnectListener(self.__listenerid)

    def __setEventHandler(self):
        self._node.connectEventHandler(avg.CURSORDOWN, self.__eventSource, self, 
                self._onDown)


class DragRecognizer(Recognizer):

    def __init__(self, node, eventSource=avg.TOUCH | avg.MOUSE, startHandler=None,
            moveHandler=None, upHandler=None, stopHandler=None, initialEvent=None,
            friction=-1):
        self.__startHandler = optionalCallback(startHandler, lambda event:None)
        self.__moveHandler = optionalCallback(moveHandler, lambda event,offset:None)
        self.__stopHandler = optionalCallback(stopHandler, lambda:None)
        self.__upHandler = optionalCallback(upHandler, lambda event,offset:None)
        self.__friction = friction
        self.__inertiaHandlerID = None
        Recognizer.__init__(self, node, eventSource, initialEvent)

    def abortInertia(self):
        if self.__inertiaHandlerID:
            self.__stop()

    def _handleDown(self, event):
        if self.__inertiaHandlerID:
            self.__stopHandler()
            g_Player.clearInterval(self.__inertiaHandlerID)
        self.__dragStartPos = event.pos
        self.__dragStartMotionVec = event.contact.motionvec
        self.__startHandler(event)
        self.__speed = avg.Point2D(0,0)
        self.__frameHandlerID = g_Player.setOnFrameHandler(self.__onFrame)

    def _handleMove(self, event):
        # TODO: Offset is in the global coordinate system. We should really be using
        # the coordinate system we're in at the moment the drag starts. 
        self.__moveHandler(event, event.contact.motionvec-self.__dragStartMotionVec)
        self.__speed += 0.1*event.speed

    def __onFrame(self):
        self.__speed *= 0.9

    def _handleUp(self, event):
        self.__upHandler(event, event.contact.motionvec)
        g_Player.clearInterval(self.__frameHandlerID)
        if self.__friction != -1:
            self.__inertiaHandlerID = g_Player.setOnFrameHandler(self.__handleInertia)
            self.__speed += 0.1*event.speed
            self.__offset = event.contact.motionvec
        else:
            self.__stopHandler()

    def __handleInertia(self):
        norm = self.__speed.getNorm()
        if norm-self.__friction > 0:
            direction = self.__speed.getNormalized()
            self.__speed = direction*(norm-self.__friction)
            self.__offset += self.__speed * g_Player.getFrameDuration()
            if self.__moveHandler:
                self.__moveHandler(None, self.__offset)
        else:
            self.__stop()

    def __stop(self):
        self.__speed = avg.Point2D(0,0)
        self.__stopHandler()
        g_Player.clearInterval(self.__inertiaHandlerID)
        self.__inertiaHandlerID = None


class HoldRecognizer(Recognizer):

    # States
    UP = 0          # No action pending
    DOWN = 1        # Down, but <  holdDelay
    HOLDING = 2     # Down, > holdDelay, < activateDelay
    ACTIVE = 3      # > activateDelay

    def __init__(self, node, holdDelay, activateDelay, eventSource=avg.TOUCH | avg.MOUSE, 
            startHandler=None, holdHandler=None, activateHandler=None, stopHandler=None,
            initialEvent=None):
        self.__startHandler = optionalCallback(startHandler, lambda pos:None)
        self.__holdHandler = optionalCallback(holdHandler, lambda t:None)
        self.__activateHandler = optionalCallback(activateHandler, lambda:None)
        self.__stopHandler = optionalCallback(stopHandler, lambda:None)

        self.__holdDelay = holdDelay
        self.__activateDelay = activateDelay

        self.__frameHandlerID = None
        self.__state = HoldRecognizer.UP

        self.__relTime = 0
        self.__lastEvent = None
        Recognizer.__init__(self, node, eventSource, initialEvent)

    def abort(self):
        self._onUp(self.__lastEvent)

    def getRelTime(self):
        return self.__relTime

    def getLastEvent(self):
        return self.__lastEvent

    def _handleDown(self, event):
        self.__startPos = event.pos
        self.__startTime = g_Player.getFrameTime()
        self.__lastEvent = event
        self.__changeState(HoldRecognizer.DOWN)
        self.__frameHandlerID = g_Player.setOnFrameHandler(self.__onFrame)

    def _handleMove(self, event):
        self.__lastEvent = event
        if event.contact.distancefromstart > 8:
            self.__startPos = event.pos
            self.__startTime = g_Player.getFrameTime()
            if self.__state != HoldRecognizer.DOWN:
                self.__stopHandler()
                self.__changeState(HoldRecognizer.DOWN)

    def __onFrame(self):
        self.__relTime = g_Player.getFrameTime() - self.__startTime
        if self.__state == HoldRecognizer.DOWN:
            if self.__relTime > self.__holdDelay:
                holdOk = self.__startHandler(self.__startPos)
                if holdOk:
                    self.__changeState(HoldRecognizer.HOLDING)
        if self.__state == HoldRecognizer.HOLDING:
            if self.__relTime > self.__activateDelay:
                self.__changeState(HoldRecognizer.ACTIVE)
                self.__activateHandler()
            else:
                self.__holdHandler(float(self.__relTime-self.__holdDelay)/
                        (self.__activateDelay-self.__holdDelay))

    def _handleUp(self, event):
        g_Player.clearInterval(self.__frameHandlerID)
        self.__frameHandlerID = None
        self.__lastEvent = None
        if self.__state != HoldRecognizer.DOWN:
            self.__stopHandler()
        self.__changeState(HoldRecognizer.UP)
        self.__relTime = 0

    def __changeState(self, newState):
#        print self, ": ", self.__state, " --> ", newState
        self.__state = newState


class TapRecognizer(Recognizer):

    UP = 0
    POSSIBLE = 1

    MAX_DISTANCE_IN_MM = 5

    def __init__(self, node, eventSource=avg.TOUCH | avg.MOUSE, startHandler=None,
            tapHandler=None, failHandler=None, initialEvent=None):
        self.__startHandler = optionalCallback(startHandler, lambda:None)
        self.__tapHandler = optionalCallback(tapHandler, lambda:None)
        self.__failHandler = optionalCallback(failHandler, lambda:None)
        self.__state = TapRecognizer.UP
        self.__maxDistance = TapRecognizer.MAX_DISTANCE_IN_MM*g_Player.getPixelsPerMM()
        Recognizer.__init__(self, node, eventSource, initialEvent)

    def _handleDown(self, event):
        self.__state = TapRecognizer.POSSIBLE
        self.__startHandler()
    
    def _handleMove(self, event):
        if event.contact.distancefromstart > self.__maxDistance:
            self._abort(event)
            self.__fail(event)

    def _handleUp(self, event):
        if event.contact.distancefromstart > self.__maxDistance:
            self.__fail(event)
        else:
            self.__recognize()

    def __recognize(self):
        self.__tapHandler()
   
    def __fail(self, event):
        self.__failHandler()
        self.__state = TapRecognizer.UP

