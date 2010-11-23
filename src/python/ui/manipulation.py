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

class DragProcessor:
    def __init__(self, node, eventSource=avg.TOUCH | avg.MOUSE, startHandler=None,
            moveHandler=None, upHandler=None, stopHandler=None, friction=-1):
        self.__node = node
        self.__eventSource = eventSource
        self.__startHandler = startHandler
        self.__moveHandler = moveHandler
        self.__stopHandler = stopHandler
        self.__upHandler = upHandler
        self.__setEventHandlers(self.__onDown, self.__onMove, self.__onUp)
        self.__dragCursorID = None
        self.__friction = friction
        self.__isEnabled = True
        self.__inertiaHandlerID = None

    def enable(self, isEnabled):
        if isEnabled != self.__isEnabled:
            self.__isEnabled = isEnabled
            if isEnabled:
                self.__setEventHandlers(self.__onDown, self.__onMove, self.__onUp)
            else:
                if self.__dragCursorID:
                    self.__node.releaseEventCapture(self.__dragCursorID)
                    self.__dragCursorID = None
                self.__setEventHandlers(lambda event:None, lambda event:None, 
                        lambda event:None)

    def __onDown(self, event):
        if self.__dragCursorID == None:
            if self.__inertiaHandlerID:
                self.__stopHandler()
                g_Player.clearInterval(self.__inertiaHandlerID)
            self.__dragCursorID = event.cursorid
            self.__dragStartPos = event.pos
            self.__node.setEventCapture(event.cursorid)
            if self.__startHandler:
                self.__startHandler(event)
            self.__speed = avg.Point2D(0,0)
            self.__frameHandlerID = g_Player.setOnFrameHandler(self.__onFrame)

    def __onMove(self, event):
        if self.__dragCursorID == event.cursorid:
            offset = event.pos - self.__dragStartPos
            if self.__moveHandler:
                self.__moveHandler(event, offset)
            self.__speed += 0.1*event.speed

    def __onFrame(self):
        self.__speed *= 0.9

    def __onUp(self, event):
        if self.__dragCursorID == event.cursorid:
            self.__node.releaseEventCapture(event.cursorid)
            self.__dragCursorID = None
            offset = event.pos - self.__dragStartPos
            if self.__upHandler:
                self.__upHandler(event, offset)
            if self.__friction != -1:
                g_Player.clearInterval(self.__frameHandlerID)
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
            self.__speed = avg.Point2D(0,0)
            if self.__stopHandler:
                self.__stopHandler()
            g_Player.clearInterval(self.__inertiaHandlerID)
            self.__inertiaHandlerID = None

    def __setEventHandlers(self, downHandler, moveHandler, upHandler):
        self.__node.setEventHandler(avg.CURSORDOWN, self.__eventSource, downHandler)
        self.__node.setEventHandler(avg.CURSORMOTION, self.__eventSource, moveHandler)
        self.__node.setEventHandler(avg.CURSORUP, self.__eventSource, upHandler)

