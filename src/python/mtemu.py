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

"""
this class provides a test to emulate one or two TOUCH/TRACK events. 
by pressing "ctrl left/right" the TOUCH events will be switched into TRACK events and the 
other way around.
by pressing "shift left/right" a second event is created whenever the mousebutton (left) 
is clicked. 

Note: remove any mouse event handling from your application to avoid emulation issues

For example:

1) avg.MOUSE | avg.TOUCH filters can be safely reduced to avg.TOUCH
2) the current libavg button module breaks the emulation
3) the current libavg Grabbable class breaks the emulation, unless is created with 
source = avg.TOUCH parameter
"""

from libavg import avg, Point2D

g_Player = avg.Player.get()

class MTemu(object):

    mouseState = 'Up'
    cursorID = 0
    lastCursorPos = None
    dualTouch = False
    secondTouch = False
    source = avg.TOUCH

    def __init__(self):
        self.__rootNode = g_Player.getRootNode()
        self.__rootNode.connectEventHandler(avg.CURSORUP, avg.MOUSE,
                                            self, self.__onMouseUp)
        self.__rootNode.connectEventHandler(avg.CURSORDOWN, avg.MOUSE,
                                            self, self.__onMouseDown)
        self.__rootNode.connectEventHandler(avg.CURSORMOTION, avg.MOUSE,
                                            self, self.__onMouseMotion)
        posX = self.__rootNode.size.x * 2/5
        posY = self.__rootNode.size.y-40

        self.__layer = avg.WordsNode(text='Multitouch emulation active',
                                     pos=(posX, posY),
                                     color='DDDDDD',
                                     sensitive=False,
                                     fontsize=20,
                                     parent=self.__rootNode)

    def deinit(self):
        self.__rootNode.disconnectEventHandler(self)
        self.__rootNode = None
        self.__layer.unlink()
        if self.mouseState == 'Down':
            self._releaseTouch(self.cursorID)
            if self.secondTouch:
                self.releaseTouch(self.cursorID+1)

    def __onMouseDown(self, event):
        if self.mouseState == 'Up' and event.button == 1:
            self._sendFakeTouch(self.cursorID, event.pos, event.type)
            if self.dualTouch and not self.secondTouch:
                self._sendFakeTouch(self.cursorID+1, event.pos, event.type,
                                    True)
                self.secondTouch = True
            self.mouseState = 'Down'
            self.lastCursorPos = event.pos

    def __onMouseMotion(self, event):
        if self.mouseState == 'Down':
            self._sendFakeTouch(self.cursorID, event.pos, event.type)
            if self.dualTouch and self.secondTouch:
                self._sendFakeTouch(self.cursorID+1, event.pos,
                                    event.type, True)
            self.lastCursorPos = event.pos

    def __onMouseUp(self, event):
        if self.mouseState == 'Down' and event.button == 1:
            self._sendFakeTouch(self.cursorID, event.pos, event.type)
            if self.dualTouch and self.secondTouch:
                self._sendFakeTouch(self.cursorID+1, event.pos,
                                    event.type, True)
                self.secondTouch = False
            self.mouseState = 'Up'
            self.cursorID += 2 #Even for left uneven for right touch
            self.lastCursorPos = None

    def toggleSource(self):
        """
        Switch between avg.TOUC and avg.TRACK - source
        """
        if self.mouseState == 'Down':
            self._releaseTouch(self.cursorID)
            if self.secondTouch:
                self._releaseTouch(self.cursorID+1)
            self.mouseState = 'Up'
            self.secondTouch = False
        self.source = avg.TOUCH if self.source == avg.TRACK else avg.TRACK

    def toggleDualTouch(self):
        self.dualTouch = not(self.dualTouch)
        if self.mouseState == 'Down':
            if self.secondTouch:
                self._releaseTouch(self.cursorID+1)
            else:
                self._sendFakeTouch(self.cursorID+1, Point2D(0,0),
                                   avg.CURSORDOWN, mirror=True)
            self.secondTouch = not(self.secondTouch)

    def _sendFakeTouch(self, cursorID, pos, touchType, mirror=False):
        if mirror:
            pos = pos + Point2D(-20,-20)
            pos = self.__rootNode.size - Point2D(20,20) - pos
            pos = self.__clampPos(pos)
        if not self.lastCursorPos:
            self.lastCursorPos = self.__clampPos(pos)
        g_Player.getTestHelper().fakeTouchEvent(cursorID,
                                    touchType, self.source, pos,
                                    Point2D(0,0))
    def _releaseTouch(self, cursorID):
       self._sendFakeTouch(cursorID, Point2D(0,0),
                                   avg.CURSORUP)

    def __clampPos(self, pos):
        if pos[0] < 0:
            pos[0] = 0
        if pos[1] < 0:
            pos[1] = 0
        if pos[0] >= g_Player.getRootNode().size[0]:
            pos[0] = g_Player.getRootNode().size[0]-1
        if pos[1] >= g_Player.getRootNode().size[1]:
            pos[1] = g_Player.getRootNode().size[1]-1
        return pos

