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
this class provides a test to emulate one or two TOUCH/TRACK events. 
by pressing "ctrl left/right" the TOUCH events will be switched into TRACK events and the 
other way around.
by pressing "shift left/right" a second event is created whenever the mousebutton (left) 
is clicked. 
'''

from libavg import avg, Point2D, player


class MTemu(object):

    mouseState = 'Up'
    cursorID = 0
    dualTouch = False
    secondTouch = False
    source = avg.Event.TOUCH

    def __init__(self):
        self.__oldEventHook = player.getEventHook()
        player.setEventHook(self.__onEvent)

        root = player.getRootNode()
        posX = root.size.x - 15
        posY = root.size.y - 20

        self.__layer = avg.WordsNode(text='Multitouch emulation active',
                pos=(posX, posY),
                alignment = 'right',
                color='DDDDDD',
                sensitive=False,
                fontsize=18,
                parent=root)

    def deinit(self):
        player.setEventHook(self.__oldEventHook)
        self.__layer.unlink()
        if self.mouseState == 'Down':
            self.__releaseTouch(self.cursorID)
            if self.secondTouch:
                self.__releaseTouch(self.cursorID+1)

    def toggleSource(self):
        '''
        Switch between avg.Event.TOUCH and avg.Event.TRACK - source
        '''
        if self.mouseState == 'Down':
            self.__releaseTouch(self.cursorID)
            if self.secondTouch:
                self.__releaseTouch(self.cursorID+1)
            self.mouseState = 'Up'
            self.secondTouch = False
        self.source = avg.Event.TOUCH if self.source == avg.Event.TRACK else avg.Event.TRACK

    def toggleDualTouch(self):
        self.dualTouch = not(self.dualTouch)
        if self.mouseState == 'Down':
            if self.secondTouch:
                self.__releaseTouch(self.cursorID+1)
            else:
                self.__sendFakeTouch(self.cursorID+1, Point2D(0,0),
                        avg.Event.CURSOR_DOWN, mirror=True)
            self.secondTouch = not(self.secondTouch)

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
        if self.mouseState == 'Up' and event.button == 1:
            self.__sendFakeTouch(self.cursorID, event.pos, event.type)
            if self.dualTouch and not self.secondTouch:
                self.__sendFakeTouch(self.cursorID+1, event.pos, event.type,
                        True)
                self.secondTouch = True
            self.mouseState = 'Down'

    def __onMouseMotion(self, event):
        if self.mouseState == 'Down':
            self.__sendFakeTouch(self.cursorID, event.pos, event.type)
            if self.dualTouch and self.secondTouch:
                self.__sendFakeTouch(self.cursorID+1, event.pos,
                        event.type, True)

    def __onMouseUp(self, event):
        if self.mouseState == 'Down' and event.button == 1:
            self.__sendFakeTouch(self.cursorID, event.pos, event.type)
            if self.dualTouch and self.secondTouch:
                self.__sendFakeTouch(self.cursorID+1, event.pos,
                        event.type, True)
                self.secondTouch = False
            self.mouseState = 'Up'
            self.cursorID += 2 #Even for left uneven for right touch

    def __sendFakeTouch(self, cursorID, pos, touchType, mirror=False):
        offset = Point2D(0,0)
        if self.dualTouch:
            offset = Point2D(40, 0)
        if mirror:
            pos = 2*(self._initialPos)-pos
            offset = -offset
        player.getTestHelper().fakeTouchEvent(cursorID,
                touchType, self.source, self.__clampPos(pos+offset))

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

