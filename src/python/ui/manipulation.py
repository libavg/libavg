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


class DragProcessor:
    def __init__(self, node, startHandler, moveHandler, endHandler):
        self.__node = node
        self.__startHandler = startHandler
        self.__moveHandler = moveHandler
        self.__endHandler = endHandler
        self.__setEventHandlers(self.__onDown, self.__onMove, self.__onUp)
        self.__dragCursorID = None

    def __onDown(self, event):
        if self.__dragCursorID == None:
            self.__dragCursorID = event.cursorid
            self.__dragStartPos = event.pos
            self.__node.setEventCapture(event.cursorid)
            self.__startHandler(event)

    def __onMove(self, event):
        if self.__dragCursorID == event.cursorid:
            offset = event.pos - self.__dragStartPos
            self.__moveHandler(event, offset)
        
    def __onUp(self, event):
        if self.__dragCursorID == event.cursorid:
            self.__node.releaseEventCapture(event.cursorid)
            self.__dragCursorID = None
            offset = event.pos - self.__dragStartPos
            self.__endHandler(event, offset)

    def __setEventHandlers(self, downHandler, moveHandler, upHandler):
        self.__node.setEventHandler(avg.CURSORDOWN, avg.TOUCH, downHandler)
        self.__node.setEventHandler(avg.CURSORMOTION, avg.TOUCH, moveHandler)
        self.__node.setEventHandler(avg.CURSORUP, avg.TOUCH, upHandler)


