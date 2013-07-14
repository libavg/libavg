#!/usr/bin/env python
# -*- coding: utf-8 -*-

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
# Original author of this file is Sebastian Maulbeck
# <sm (at) archimedes-solutions (dot) de>

'''
Multitouch emulation helper, supporting pinch gestures
'''

from libavg import avg, Point2D, player


class MTemu(object):
    MOUSE_STATE_UP = 'MOUSE_STATE_UP'
    MOUSE_STATE_DOWN = 'MOUSE_STATE_DOWN'

    def __init__(self):
        self.__mouseState = self.MOUSE_STATE_UP
        self.__cursorID = 0
        self.__dualTouch = False
        self.__secondTouch = False
        self.__source = avg.Event.TOUCH

        self.__oldEventHook = player.getEventHook()
        player.setEventHook(self.__onEvent)

        root = player.getRootNode()
        self.__caption = avg.WordsNode(pos=(root.size.x - 15, root.size.y - 20),
                alignment = 'right',
                color='DDDDDD',
                sensitive=False,
                fontsize=14,
                parent=root)
        self.__updateCaption()

    def deinit(self):
        player.setEventHook(self.__oldEventHook)
        self.__caption.unlink()
        if self.__mouseState == self.MOUSE_STATE_DOWN:
            self.__releaseTouch(self.__cursorID)
            if self.__secondTouch:
                self.__releaseTouch(self.__cursorID+1)

    def toggleSource(self):
        '''
        Switch between avg.Event.TOUCH and avg.Event.TRACK - source
        '''
        self.__clearSourceState()
        self.__source = (avg.Event.TOUCH if self.__source == avg.Event.TRACK
                else avg.Event.TRACK)
        self.__updateCaption()

    def toggleDualTouch(self):
        self.__dualTouch = not(self.__dualTouch)
        self.__clearDualtouchState()

    def enableDualTouch(self):
        self.__dualTouch = True
        self.__clearDualtouchState()

    def disableDualTouch(self):
        self.__dualTouch = False
        self.__clearDualtouchState()

    def __clearSourceState(self):
        if self.__mouseState == self.MOUSE_STATE_DOWN:
            self.__releaseTouch(self.__cursorID)
            if self.__secondTouch:
                self.__releaseTouch(self.__cursorID+1)
            self.__mouseState = self.MOUSE_STATE_UP
            self.__secondTouch = False

    def __clearDualtouchState(self):
        if self.__mouseState == self.MOUSE_STATE_DOWN:
            if self.__secondTouch:
                self.__releaseTouch(self.__cursorID+1)
            else:
                self.__sendFakeTouch(self.__cursorID+1, Point2D(0,0),
                        avg.Event.CURSOR_DOWN, mirror=True)
            self.__secondTouch = not(self.__secondTouch)

    def __updateCaption(self):
        self.__caption.text = 'Multitouch emulation (%s source)' % self.__source

    def __onEvent(self, event):
        if event.source == avg.Event.MOUSE:
            if event.type == avg.Event.CURSOR_DOWN:
                self.__onMouseDown(event)
            elif event.type == avg.Event.CURSOR_MOTION:
                self.__onMouseMotion(event)
            elif event.type == avg.Event.CURSOR_UP:
                self.__onMouseUp(event)
            return True
        else:
            return False

    def __onMouseDown(self, event):
        self._initialPos = event.pos
        if self.__mouseState == self.MOUSE_STATE_UP and event.button == 1:
            self.__sendFakeTouch(self.__cursorID, event.pos, event.type)
            if self.__dualTouch and not self.__secondTouch:
                self.__sendFakeTouch(self.__cursorID+1, event.pos, event.type,
                        True)
                self.__secondTouch = True
            self.__mouseState = self.MOUSE_STATE_DOWN

    def __onMouseMotion(self, event):
        if self.__mouseState == self.MOUSE_STATE_DOWN:
            self.__sendFakeTouch(self.__cursorID, event.pos, event.type)
            if self.__dualTouch and self.__secondTouch:
                self.__sendFakeTouch(self.__cursorID+1, event.pos,
                        event.type, True)

    def __onMouseUp(self, event):
        if self.__mouseState == self.MOUSE_STATE_DOWN and event.button == 1:
            self.__sendFakeTouch(self.__cursorID, event.pos, event.type)
            if self.__dualTouch and self.__secondTouch:
                self.__sendFakeTouch(self.__cursorID+1, event.pos,
                        event.type, True)
                self.__secondTouch = False
            self.__mouseState = self.MOUSE_STATE_UP
            self.__cursorID += 2 #Even for left uneven for right touch

    def __sendFakeTouch(self, cursorID, pos, touchType, mirror=False):
        offset = Point2D(0,0)
        if self.__dualTouch:
            offset = Point2D(40, 0)
        if mirror:
            pos = 2*(self._initialPos)-pos
            offset = -offset
        player.getTestHelper().fakeTouchEvent(cursorID,
                touchType, self.__source, self.__clampPos(pos+offset))

    def __releaseTouch(self, cursorID):
       self.__sendFakeTouch(cursorID, Point2D(0,0), avg.Event.CURSOR_UP)

    def __clampPos(self, pos):
        if pos[0] < 0:
            pos[0] = 0
        if pos[1] < 0:
            pos[1] = 0
        if pos[0] >= player.getRootNode().size[0]:
            pos[0] = player.getRootNode().size[0]-1
        if pos[1] >= player.getRootNode().size[1]:
            pos[1] = player.getRootNode().size[1]-1
        return pos

