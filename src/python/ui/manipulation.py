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

g_Player = avg.Player.get()

class ManipulationProcessor(object):
    def __init__(self, node, eventSource, initialEvent):
        self._node = node
        self.__eventSource = eventSource
       
        self.__setEventHandlers(self._onDown, self.__onMove, self._onUp) 
        self.__isEnabled = True
        self.__cursorID = None
        if initialEvent:
            self._onDown(initialEvent)

    def enable(self, isEnabled):
        if isEnabled != self.__isEnabled:
            self.__isEnabled = isEnabled
            if isEnabled:
                self.__setEventHandlers(self._onDown, self.__onMove, self._onUp)
            else:
                if self.__cursorID:
                    self._node.releaseEventCapture(self.__cursorID)
                    self.__cursorID = None
                self._node.disconnectEventHandler(self)

    def _onDown(self, event):
        if self.__cursorID == None:
            self.__cursorID = event.cursorid
            self._node.setEventCapture(event.cursorid)
            return self._handleDown(event)

    def __onMove(self, event):
        if self.__cursorID == event.cursorid:
            return self._handleMove(event)

    def _onUp(self, event):
        if self.__cursorID == event.cursorid:
            self._node.releaseEventCapture(event.cursorid)
            self.__cursorID = None
            return self._handleUp(event)

    def _optionalCallback(self, handler, defaultHandler):
        if handler:
            return handler
        else:
            return defaultHandler

    def _handleUp(self, event):
        pass
    
    def _handleDown(self, event):
        pass
    
    def _handleMove(self, event):
        pass

    def __setEventHandlers(self, downHandler, moveHandler, upHandler):
        self._node.connectEventHandler(avg.CURSORDOWN, self.__eventSource,
                self, downHandler)
        self._node.connectEventHandler(avg.CURSORMOTION, self.__eventSource, 
                self, moveHandler)
        self._node.connectEventHandler(avg.CURSORUP, self.__eventSource, 
                self, upHandler)


class DragProcessor(ManipulationProcessor):
    def __init__(self, node, eventSource=avg.TOUCH | avg.MOUSE, startHandler=None,
            moveHandler=None, upHandler=None, stopHandler=None, initialEvent=None,
            friction=-1):
        self.__startHandler = self._optionalCallback(startHandler, lambda event:None)
        self.__moveHandler = self._optionalCallback(moveHandler, lambda event,offset:None)
        self.__stopHandler = self._optionalCallback(stopHandler, lambda:None)
        self.__upHandler = self._optionalCallback(upHandler, lambda event,offset:None)
        self.__friction = friction
        self.__inertiaHandlerID = None
        ManipulationProcessor.__init__(self, node, eventSource, initialEvent)

    def abortInertia(self):
        if self.__inertiaHandlerID:
            self.__stop()

    def _handleDown(self, event):
            if self.__inertiaHandlerID:
                self.__stopHandler()
                g_Player.clearInterval(self.__inertiaHandlerID)
            self.__dragStartPos = event.pos
            self.__startHandler(event)
            self.__speed = avg.Point2D(0,0)
            self.__frameHandlerID = g_Player.setOnFrameHandler(self.__onFrame)

    def _handleMove(self, event):
        # TODO: Offset is in the global coordinate system. We should really be using
        # the coordinate system we're in at the moment the drag starts. 
        offset = event.pos - self.__dragStartPos
        if self.__moveHandler:
            self.__moveHandler(event, offset)
        self.__speed += 0.1*event.speed

    def __onFrame(self):
        self.__speed *= 0.9

    def _handleUp(self, event):
        offset = event.pos - self.__dragStartPos
        self.__upHandler(event, offset)
        g_Player.clearInterval(self.__frameHandlerID)
        if self.__friction != -1:
            self.__inertiaHandlerID = g_Player.setOnFrameHandler(self.__handleInertia)
            self.__speed += 0.1*event.speed
            self.__offset = offset
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


class HoldProcessor(ManipulationProcessor):

    # States
    UP = 0          # No action pending
    DOWN = 1        # Down, but <  holdDelay
    HOLDING = 2     # Down, > holdDelay, < activateDelay
    ACTIVE = 3      # > activateDelay

    def __init__(self, node, holdDelay, activateDelay, eventSource=avg.TOUCH | avg.MOUSE, 
            startHandler=None, holdHandler=None, activateHandler=None, stopHandler=None,
            initialEvent=None):
        self.__startHandler = self._optionalCallback(startHandler, lambda pos:None)
        self.__holdHandler = self._optionalCallback(holdHandler, lambda t:None)
        self.__activateHandler = self._optionalCallback(activateHandler, lambda:None)
        self.__stopHandler = self._optionalCallback(stopHandler, lambda:None)

        self.__holdDelay = holdDelay
        self.__activateDelay = activateDelay

        self.__frameHandlerID = None
        self.__state = HoldProcessor.UP

        self.__relTime = 0
        self.__lastEvent = None
        ManipulationProcessor.__init__(self, node, eventSource, initialEvent)

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
        self.__changeState(HoldProcessor.DOWN)
        self.__frameHandlerID = g_Player.setOnFrameHandler(self.__onFrame)

    def _handleMove(self, event):
        self.__lastEvent = event
        offset = event.pos - self.__startPos
        if offset.getNorm() > 8:
            self.__startPos = event.pos
            self.__startTime = g_Player.getFrameTime()
            if self.__state != HoldProcessor.DOWN:
                self.__stopHandler()
                self.__changeState(HoldProcessor.DOWN)

    def __onFrame(self):
        self.__relTime = g_Player.getFrameTime() - self.__startTime
        if self.__state == HoldProcessor.DOWN:
            if self.__relTime > self.__holdDelay:
                holdOk = self.__startHandler(self.__startPos)
                if holdOk:
                    self.__changeState(HoldProcessor.HOLDING)
        if self.__state == HoldProcessor.HOLDING:
            if self.__relTime > self.__activateDelay:
                self.__changeState(HoldProcessor.ACTIVE)
                self.__activateHandler()
            else:
                self.__holdHandler(float(self.__relTime-self.__holdDelay)/
                        (self.__activateDelay-self.__holdDelay))

    def _handleUp(self, event):
        g_Player.clearInterval(self.__frameHandlerID)
        self.__frameHandlerID = None
        self.__lastEvent = None
        if self.__state != HoldProcessor.DOWN:
            self.__stopHandler()
        self.__changeState(HoldProcessor.UP)
        self.__relTime = 0

    def __changeState(self, newState):
#        print self, ": ", self.__state, " --> ", newState
        self.__state = newState

