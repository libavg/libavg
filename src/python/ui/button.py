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
from libavg import statemachine
import gesture
from helper import *


class Button(libavg.DivNode):
    STATE_DISABLED = 1
    STATE_UP       = 2
    STATE_DOWN     = 3
    
    def __init__(self, upNode = None, downNode = None, disabledNode = None, 
            activeAreaNode = None, pressHandler = None, clickHandler = None,
            stateChangeHandler = None, **kwargs):
        libavg.DivNode.__init__(self, **kwargs)
        
        self.__upNode = upNode
        self.__downNode = downNode
        self.__disabledNode = disabledNode
        self.__activeAreaNode = activeAreaNode
        
        self.__defaultHandler = lambda event: None
        self.__pressCallback = optionalCallback(pressHandler, self.__defaultHandler)
        self.__clickCallback = optionalCallback(clickHandler, self.__defaultHandler)
        self.__stateChangeCallback = optionalCallback(stateChangeHandler, 
                lambda state: None)

        self.__capturedCursorIds = set()
        self.__overCursorIds = set()
        
        self.__isCheckable = False
        self.__isToggled = False

        self.__isOver = False
        self.__state = Button.STATE_UP
        
        if self.__upNode and self.__downNode:
            self.__setupNodes()
        
    def setEventHandler(self, type, source, func):
        raise RuntimeError("Setting event handlers for buttons is not supported")
    
    def getUpNode(self):
        return self.__upNode
    
    def getDownNode(self):
        return self.__downNode
    
    def getDisabledNode(self):
        return self.__disabledNode
    
    def setNodes(self, upNode, downNode, disabledNode = None, activeAreaNode = None):
        if self.__activeAreaNode and self.isEnabled():
            self.__deactivateEventHandlers()
        self.__upNode = upNode
        self.__downNode = downNode
        self.__disabledNode = disabledNode
        self.__activeAreaNode = activeAreaNode
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
            self.__overCursorIds = set()
        else:
            self.__activateEventHandlers()
        
    def isEnabled(self):
        return self.__state != Button.STATE_DISABLED
    
    def __setupNodes(self):
        while self.getNumChildren() > 0:
            self.removeChild(self.getChild(0))
            
        self.appendChild(self.__upNode)
        self.appendChild(self.__downNode)

        if self.__disabledNode:
            self.appendChild(self.__disabledNode)
        
        if self.__activeAreaNode == None:
            self.__activeAreaNode = libavg.RectNode(opacity=0, size=self.__upNode.size)
        self.appendChild(self.__activeAreaNode)

        self.size = self.__activeAreaNode.size
        self.__updateNodesVisibility()
        if self.isEnabled():
            self.__activateEventHandlers()

    def __updateNodesVisibility(self):
        for element in (self.__upNode, self.__downNode, self.__disabledNode):
            if element:
                element.active = False

        if self.__state == Button.STATE_UP:
            activeNode = self.__upNode
        elif self.__state == Button.STATE_DOWN:
            activeNode = self.__downNode
        elif self.__state == Button.STATE_DISABLED:
            activeNode = self.__disabledNode
        else:
            # This state doesn't exist.
            assert(False)

        activeNode.active = True
        
    def __changeState(self, state):
        self.__state = state
        self.__updateNodesVisibility()
        self.__stateChangeCallback(state)
    
    def __captureCursor(self, id):
        self.__capturedCursorIds.add(id)
        self.__activeAreaNode.setEventCapture(id)
    
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
        
        if self.__getNumberOfCapturedCursors() <= 1:
            self.__changeState(Button.STATE_DOWN)
            self.__pressCallback(event)
    
    def __releaseHandler(self, event):
        numberOfCapturedCursors = self.__getNumberOfCapturedCursors()
        numberOfOverCursors = len(self.__overCursorIds)
        
        if self.__isCursorCaptured(event.cursorid):
            self.__releaseCapturedCursor(event.cursorid)

        if event.cursorid in self.__overCursorIds:
            self.__overCursorIds.remove(event.cursorid)
            
        if numberOfCapturedCursors == 1 and numberOfOverCursors == 1:
            newState = Button.STATE_UP
            if self.isCheckable():
                self.__isToggled = not self.__isToggled
                if self.__isToggled:
                    newState = Button.STATE_DOWN
                
            self.__changeState(newState)
            
            self.__clickCallback(event)
    
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
            self.__activeAreaNode.connectEventHandler(type, libavg.MOUSE | libavg.TOUCH, 
                    self, handler)

        setOneHandler(libavg.CURSORDOWN, self.__pressHandler)
        setOneHandler(libavg.CURSORUP, self.__releaseHandler)
        setOneHandler(libavg.CURSOROVER, self.__overHandler)
        setOneHandler(libavg.CURSOROUT, self.__outHandler)
    
    def __deactivateEventHandlers(self):
        for id in self.__capturedCursorIds:
            self.releaseEventCapture(id)
        self.__capturedCursorIds = set()
        self.__activeAreaNode.disconnectEventHandler(self)


class TouchButton(libavg.DivNode):

    def __init__(self, upNode, downNode, disabledNode = None, activeAreaNode = None, 
            fatFingerEnlarge=False, clickHandler = None, **kwargs):
        libavg.DivNode.__init__(self, **kwargs)
        
        self.__upNode = upNode
        self.__downNode = downNode
        self.__disabledNode = disabledNode
        self.__activeAreaNode = activeAreaNode
        
        self.__clickHandler = optionalCallback(clickHandler, lambda:None)

        self.__isOver = False
        self.__stateMachine = statemachine.StateMachine("TouchButton", "UP")
        self.__stateMachine.addState("UP", {"DOWN": None, "DISABLED": None},
                enterFunc=self.enterUp, leaveFunc=self.leaveUp)
        self.__stateMachine.addState("DOWN", {"UP": None, "DISABLED": None},
                enterFunc=self.enterDown, leaveFunc=self.leaveDown)
        self.__stateMachine.addState("DISABLED", {"UP": None, "DOWN": None},
                enterFunc=self.enterDown, leaveFunc=self.leaveDown)

        self.appendChild(self.__upNode)
        self.__upNode.active = True
        self.appendChild(self.__downNode)
        self.__downNode.active = False

        if self.__disabledNode:
            self.appendChild(self.__disabledNode)
            self.__disabledNode.active = False
        
        if self.__activeAreaNode == None:
            self.__activeAreaNode = self.__upNode
        else:
            self.appendChild(self.__activeAreaNode)
        
        self.__tapRecognizer = gesture.TapRecognizer(self.__activeAreaNode,
                startHandler=self.__onStart, 
                tapHandler=self.__onTap, 
                failHandler=self.__onFail)

    def __onStart(self):
        self.__stateMachine.changeState("DOWN")

    def __onTap(self):
        self.__stateMachine.changeState("UP")
        self.__clickHandler()

    def __onFail(self):
        self.__stateMachine.changeState("UP")

    def enterUp(self):
        self.__upNode.active = True

    def leaveUp(self):
        self.__upNode.active = False

    def enterDown(self):
        self.__downNode.active = True

    def leaveDown(self):
        self.__downNode.active = False
