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

class ManipulationProcessor:
    def __init__(self, node, eventSource):
        self.__node = node
        self.__eventSource = eventSource
       
        self.__setEventHandlers(self.__onDown, self.__onMove, self.__onUp) 
        self.__isEnabled = True
        self.__cursorID = None

    def enable(self, isEnabled):
        if isEnabled != self.__isEnabled:
            self.__isEnabled = isEnabled
            if isEnabled:
                self.__setEventHandlers(self.__onDown, self.__onMove, self.__onUp)
            else:
                if self.__cursorID:
                    self.__node.releaseEventCapture(self.__cursorID)
                    self.__cursorID = None
                self.__setEventHandlers(lambda event:None, lambda event:None, 
                        lambda event:None)

    def __onDown(self, event):
        if self.__cursorID == None:
            self.__cursorID = event.cursorid
            self.__node.setEventCapture(event.cursorid)
            self._handleDown(event)

    def __onMove(self, event):
        if self.__cursorID == event.cursorid:
            self._handleMove(event)

    def __onUp(self, event):
        if self.__cursorID == event.cursorid:
            self.__node.releaseEventCapture(event.cursorid)
            self.__cursorID = None
            self._handleUp(event)

    def __setEventHandlers(self, downHandler, moveHandler, upHandler):
        self.__node.setEventHandler(avg.CURSORDOWN, self.__eventSource, downHandler)
        self.__node.setEventHandler(avg.CURSORMOTION, self.__eventSource, moveHandler)
        self.__node.setEventHandler(avg.CURSORUP, self.__eventSource, upHandler)


class DragProcessor(ManipulationProcessor):
    def __init__(self, node, eventSource=avg.TOUCH | avg.MOUSE, startHandler=None,
            moveHandler=None, upHandler=None, stopHandler=None, friction=-1):
        ManipulationProcessor.__init__(self, node, eventSource)
        self.__startHandler = startHandler
        self.__moveHandler = moveHandler
        self.__stopHandler = stopHandler
        self.__upHandler = upHandler
        self.__friction = friction
        self.__inertiaHandlerID = None

    def abortInertia(self):
        if self.__inertiaHandlerID:
            self.__stop()

    def _handleDown(self, event):
            if self.__inertiaHandlerID:
                self.__stopHandler()
                g_Player.clearInterval(self.__inertiaHandlerID)
            self.__dragStartPos = event.pos
            if self.__startHandler:
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
        if self.__upHandler:
            self.__upHandler(event, offset)
        g_Player.clearInterval(self.__frameHandlerID)
        if self.__friction != -1:
            self.__inertiaHandlerID = g_Player.setOnFrameHandler(self.__handleInertia)
            self.__speed += 0.1*event.speed
            self.__offset = offset
        else:
            if self.__stopHandler:
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
        if self.__stopHandler:
            self.__stopHandler()
        g_Player.clearInterval(self.__inertiaHandlerID)
        self.__inertiaHandlerID = None


class HoldProcessor(ManipulationProcessor):

    # States
    UP = 0          # No action pending
    DOWN = 1        # Down, but <  activateDelay
    ACTIVE = 2      # > activateDelay
    ABORTED = 3     # Moved too far, but still down

    def __init__(self, node, activateDelay, eventSource=avg.TOUCH | avg.MOUSE, 
            startHandler=None, holdHandler=None, activateHandler=None, stopHandler=None):
        ManipulationProcessor.__init__(self, node, eventSource)
        self.__startHandler = startHandler
        self.__holdHandler = holdHandler
        self.__activateHandler = activateHandler
        self.__stopHandler = stopHandler

        self.__activateDelay = activateDelay

        self.__frameHandlerID = None
        self.__state = HoldProcessor.UP

    def _handleDown(self, event):
        self.__startPos = event.pos
        self.__frameHandlerID = g_Player.setOnFrameHandler(self.__onFrame)
        self.__downTime = g_Player.getFrameTime()
        self.__changeState(HoldProcessor.DOWN)

    def _handleMove(self, event):
        if self.__state == HoldProcessor.DOWN:
            offset = event.pos - self.__startPos
            if offset.getNorm() > 5:
                g_Player.clearInterval(self.__frameHandlerID)
                self.__changeState(HoldProcessor.ABORTED)

    def __onFrame(self):
        relTime = g_Player.getFrameTime()-self.__downTime
        if self.__state == HoldProcessor.DOWN:
            if relTime > self.__activateDelay:
                self.__changeState(HoldProcessor.ACTIVE)
            else:
                self.__holdHandler(float(relTime)/self.__activateDelay)

    def _handleUp(self, event):
        if self.__state != HoldProcessor.ABORTED:
            g_Player.clearInterval(self.__frameHandlerID)
        self.__changeState(HoldProcessor.UP)

    def __changeState(self, newState):
        if self.__state == HoldProcessor.UP and newState == HoldProcessor.DOWN:
            self.__startHandler()
        elif self.__state == HoldProcessor.DOWN and newState == HoldProcessor.ACTIVE:
            self.__activateHandler()
        elif self.__state == HoldProcessor.DOWN and newState == HoldProcessor.ABORTED:
            self.__stopHandler()
        elif self.__state != HoldProcessor.UP and newState == HoldProcessor.UP:
            if self.__state != HoldProcessor.ABORTED:
                self.__stopHandler()
        else:
            assert(False)
        self.__state = newState

