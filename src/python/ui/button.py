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
# Original author of this file is Henrik Thoms

import libavg

from helper import *

g_log = libavg.Logger.get()


class Button(libavg.DivNode):
    STATE_DISABLED = 1
    STATE_UP       = 2
    STATE_DOWN     = 3
    
    def __init__(self, upNode = None, downNode = None, disabledNode = None, 
                 pressHandler = None, clickHandler = None, **kwargs):
        libavg.DivNode.__init__(self, **kwargs)
        self.crop = False
        
        self.__upNode       = upNode
        self.__downNode     = downNode
        self.__disabledNode = disabledNode
        
        self.__defaultHandler = lambda event: None
        self.__pressCallback = optionalCallback(pressHandler, self.__defaultHandler)
        self.__clickCallback = optionalCallback(clickHandler, self.__defaultHandler)

        self.__capturedCursorIds = set()
        self.__overCursorIds = set()
        
        self.__isCheckable = False
        self.__isToggled = False

        self.__isOver = False
        self.__state = Button.STATE_UP
        
        if self.__upNode and self.__downNode:
            self.__setupNodes()
        self.__activateEventHandlers()
        
    def setEventHandler(self, type, source, func):
        raise RuntimeError("Setting event handlers for buttons is not supported")
    
    def getUpNode(self):
        return self.__upNode
    
    def getDownNode(self):
        return self.__downNode
    
    def getDisabledNode(self):
        return self.__disabledNode
    
    def setNodes(self, upNode, downNode, disabledNode = None):
        self.__upNode = upNode
        self.__downNode = downNode
        self.__disabledNode = disabledNode
        self.__setupNodes()
        
    def setPressHandler(self, handler):
        self.__pressCallback = optionalCallback(handler, self.__defaultHandler)
        
    def setClickHandler(self, handler):
        self.__clickCallback = optionalCallback(handler, self.__defaultHandler)
    
    def setCheckable(self, val):
        self.__isCheckable = val
        
    def isCheckable(self):
        return self.__isCheckable
    
    def setChecked(self, val):
        assert(self.__isCheckable)
        assert(self.isEnabled())
        self.__isToggled = val
        state = Button.STATE_DOWN if self.__isToggled else Button.STATE_UP
        self.__changeState(state)
    
    def isChecked(self):
        assert(self.__isCheckable)
        # XXX: Shouldn't this check self.__isToggled?
        return self.__state == Button.STATE_DOWN
    
    def setEnabled(self, isEnabled):
        if (isEnabled == self.isEnabled()):
            # No state change
            return

        state = Button.STATE_DISABLED if not(isEnabled) else Button.STATE_UP
        self.__changeState(state)
        
        if self.__state == Button.STATE_DISABLED:
            self.__deactivateEventHandlers()
        else:
            self.__activateEventHandlers()
        
    def isEnabled(self):
        return self.__state != Button.STATE_DISABLED
    
    def setDebug(self, debug):
        self.elementoutlinecolor = 'FF0000' if debug else ''
    
    def isDebug(self):
        return self.elementoutlinecolor != ''
    
    def __setupNodes(self):
        while self.getNumChildren() > 0:
            self.removeChild(self.getChild(0))
            
        self.appendChild(self.__upNode)
        self.appendChild(self.__downNode)

        if self.__disabledNode:
            self.appendChild(self.__disabledNode)
        
        self.__updateSize()
        self.__updateNodesVisibility()


    def __updateNodesVisibility(self):
        if self.__state == Button.STATE_UP:
            self.__setNodesVisibility(self.__upNode)

        elif self.__state == Button.STATE_DOWN:
            self.__setNodesVisibility(self.__downNode)

        elif self.__state == Button.STATE_DISABLED:
            self.__setNodesVisibility(self.__disabledNode)
        else:
            # This state doesn't exist.
            assert(False)
        
    def __updateSize(self):
        self.size = self.__upNode.size
    
    def __setNodesVisibility(self, node):
        nodes = (self.__upNode, self.__downNode, self.__disabledNode)
        for element in nodes:
            if not element:
                continue
            
            element.active = False
            if element == node:
                element.active = True
            
    def __changeState(self, state):
        self.__state = state
        self.__updateNodesVisibility()
    
    def __captureCursor(self, id):
        self.__capturedCursorIds.add(id)
        self.setEventCapture(id)
    
    def __releaseCapturedCursor(self, id):
        self.__capturedCursorIds.remove(id)
        self.releaseEventCapture(id)
    
    def __isCursorCaptured(self, id):
        return id in self.__capturedCursorIds
    
    def __getNumberOfCapturedCursors(self):
        return len(self.__capturedCursorIds)

    def __hasCapturedCursor(self):
        return len(self.__capturedCursorIds) != 0
    
    def __pressHandler(self, event):
        if not self.__isCursorCaptured(event.cursorid):
            self.__captureCursor(event.cursorid)
        
        if event.cursorid not in self.__overCursorIds:
            self.__overCursorIds.add(event.cursorid)
        
        if self.__getNumberOfCapturedCursors() > 1:
            return
        
        self.__changeState(Button.STATE_DOWN)
        self.__pressCallback(event)
        return True
    
    def __releaseHandler(self, event):
        numberOfCapturedCursors = self.__getNumberOfCapturedCursors()
        numberOfOverCursors = len(self.__overCursorIds)
        
        if self.__isCursorCaptured(event.cursorid):
            self.__releaseCapturedCursor(event.cursorid)

        if event.cursorid in self.__overCursorIds:
            self.__overCursorIds.remove(event.cursorid)
            
        if numberOfCapturedCursors > 1:
            return

        if  numberOfCapturedCursors == 0:
            return

        if  numberOfOverCursors == 0:
            return
        
        newState = Button.STATE_UP
        if self.isCheckable():
            self.__isToggled = not self.__isToggled
            if self.__isToggled:
                newState = Button.STATE_DOWN
            
        self.__changeState(newState)
        
        if self.__hasCapturedCursor():
            pass

        if len(self.__overCursorIds):
            pass
        
        self.__clickCallback(event)
        return True
    
    def __overHandler(self, event):
        if event.cursorid not in self.__overCursorIds:
            self.__overCursorIds.add(event.cursorid)
            
        if self.__hasCapturedCursor() and len(self.__overCursorIds):
            self.__changeState(Button.STATE_DOWN)
            
        self.__isOver = not(self.__isOver)
        return True
    
    def __outHandler(self, event):
        if event.cursorid in self.__overCursorIds:
            self.__overCursorIds.remove(event.cursorid)
        
        if self.__hasCapturedCursor() and not len(self.__overCursorIds):
            newState = Button.STATE_UP
            if self.isCheckable():
                if self.__isToggled:
                    newState = Button.STATE_DOWN
            self.__changeState(newState)

        self.__isOver = not(self.__isOver)
        return True
    
    def __activateEventHandlers(self):
        def setOneHandler(type, handler):
            self.connectEventHandler(type, libavg.MOUSE | libavg.TOUCH, self, handler)

        setOneHandler(libavg.CURSORDOWN, self.__pressHandler)
        setOneHandler(libavg.CURSORUP, self.__releaseHandler)
        setOneHandler(libavg.CURSOROVER, self.__overHandler)
        setOneHandler(libavg.CURSOROUT, self.__outHandler)
    
    def __deactivateEventHandlers(self):
        for id in self.__capturedCursorIds:
            self.releaseEventCapture(id)
        self.__capturedCursorIds = set()
        self.disconnectEventHandler(self)

