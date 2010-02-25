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
g_Player = None

class Button(avg.DivNode):
    STATE_UP = 1
    STATE_DOWN = 2
    STATE_DISABLED = 3
    STATE_OUT = 4       # Button is pressed but cursor has moved outside the node.

    def __init__(self,
                upNode,
                downNode = None,
                disabledNode = None,
                onClick = lambda event: None,
                onDown = lambda event:None,
                isMultitouch = False,
                **kwargs):
        if ((downNode and upNode.size != downNode.size) or
                (disabledNode and upNode.size != disabledNode.size)):
            raise RuntimeError("The sizes of all nodes in a button must be equal")
        self.__upNode = upNode
        self.__downNode = downNode
        self.__disabledNode = disabledNode
        self.__onClickCallback = onClick
        self.__onDownCallback = onDown
        avg.DivNode.__init__(self, **kwargs)
        self.size = upNode.size
        self.appendChild(upNode)
        if downNode:
            self.appendChild(downNode)
        if disabledNode:
            self.appendChild(disabledNode)
        self.__state = None
        self.__cursorsClicking = set()
        self.__cursorsOverNode = set()
        self.__isMultitouch = isMultitouch
        self.__setState(Button.STATE_UP)

    def setEventHandler(self, type, source, func):
        raise RuntimeError("Setting event handlers for buttons is not supported")

    def setNodes(self, upNode, downNode = None, disabledNode = None):
        self.__upNode = upNode
        self.__downNode = downNode
        self.__disabledNode = disabledNode

    def getState(self):
        return self.__state

    def enable(self, enabled):
        if enabled:
            self.__setState(Button.STATE_UP)
        else:
            self.__setState(Button.STATE_DISABLED)
            for id in self.__cursorsClicking:
                self.releaseEventCapture(id)
            self.__cursorsClicking = set()
            self.__cursorsOverNode = set()

    def __onDown(self, event):
        if not(self.__isMultitouch) and not(event.button==1):
            return
        self.__setState(Button.STATE_DOWN)
        self.__onDownCallback(event)
        self.__cursorsClicking.add(event.cursorid)
        self.__cursorsOverNode.add(event.cursorid)
        self.setEventCapture(event.cursorid)

    def __onUp(self, event):
        if not(self.__isMultitouch) and not(event.button==1):
            return
        if event.cursorid in self.__cursorsClicking:
            self.releaseEventCapture(event.cursorid)
            self.__cursorsClicking.remove(event.cursorid)
            if len(self.__cursorsClicking) == 0:
                self.__setState(Button.STATE_UP)
                if event.cursorid in self.__cursorsOverNode:
                    self.__onClickCallback(event)
            if event.cursorid in self.__cursorsOverNode:
                self.__cursorsOverNode.remove(event.cursorid)

    def __onOut(self, event):
        if event.cursorid in self.__cursorsClicking:
            self.__cursorsOverNode.remove(event.cursorid)
        if len(self.__cursorsOverNode) == 0:
            self.__setState(Button.STATE_OUT)

    def __onOver(self, event):
        if len(self.__cursorsOverNode) == 0:
            self.__setState(Button.STATE_DOWN)
        if event.cursorid in self.__cursorsClicking:
            self.__cursorsOverNode.add(event.cursorid)

    def __setState(self, state):
#        print self.__state, " -> ", state
        for node in (self.__upNode, self.__downNode, self.__disabledNode):
            if node:
                node.opacity = 0
        if self.__isMultitouch:
            source = avg.TOUCH
        else:
            source = avg.MOUSE
        if state == Button.STATE_UP:
            curNode = self.__upNode
            avg.DivNode.setEventHandler(self, avg.CURSORDOWN, source, self.__onDown)
            avg.DivNode.setEventHandler(self, avg.CURSORUP, source, None)
            avg.DivNode.setEventHandler(self, avg.CURSOROVER, source, None)
            avg.DivNode.setEventHandler(self, avg.CURSOROUT, source, None)
        elif state == Button.STATE_DOWN:
            if self.__downNode:
                curNode = self.__downNode
            else:
                curNode = self.__upNode
            avg.DivNode.setEventHandler(self, avg.CURSORDOWN, source, self.__onDown)
            avg.DivNode.setEventHandler(self, avg.CURSORUP, source, self.__onUp)
            avg.DivNode.setEventHandler(self, avg.CURSOROVER, source, self.__onOver)
            avg.DivNode.setEventHandler(self, avg.CURSOROUT, source, self.__onOut)
        elif state == Button.STATE_DISABLED:
            curNode = self.__disabledNode
            avg.DivNode.setEventHandler(self, avg.CURSORDOWN, source, None)
            avg.DivNode.setEventHandler(self, avg.CURSORUP, source, None)
            avg.DivNode.setEventHandler(self, avg.CURSOROVER, source, None)
            avg.DivNode.setEventHandler(self, avg.CURSOROUT, source, None)
        elif state == Button.STATE_OUT:
            assert(self.__state == Button.STATE_DOWN)
            curNode = self.__upNode
        else:
            assert(False)
        curNode.opacity = 1
        self.__state = state 

