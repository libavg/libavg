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
#
# Original author of this file is Henrik Thoms

from libavg import avg, statemachine, utils
import gesture
from helper import *

g_Player = avg.Player.get()

class Button(avg.DivNode):
    STATE_DISABLED = 1
    STATE_UP       = 2
    STATE_DOWN     = 3
    
    def __init__(self, upNode=None, downNode=None, disabledNode=None, 
            activeAreaNode=None, pressHandler=None, clickHandler=None,
            stateChangeHandler=None, parent=None, **kwargs):
        super(Button, self).__init__(**kwargs)
        self.registerInstance(self, parent)

        self.__upNode = upNode
        self.__downNode = downNode
        self.__disabledNode = disabledNode
        self.__activeAreaNode = activeAreaNode
        
        self.__pressCallback = utils.methodref(pressHandler)
        self.__clickCallback = utils.methodref(clickHandler)
        self.__stateChangeCallback = utils.methodref(stateChangeHandler)

        self.__capturedCursorIds = set()
        self.__overCursorIds = set()
        
        self.__isCheckable = False
        self.__isToggled = False

        self.__isOver = False
        self.__state = Button.STATE_UP
        
        if self.__upNode and self.__downNode:
            self.__setupNodes()

    def delete(self):
        self.__pressCallback = None
        self.__clickCallback = None
        self.__stateChangeCallback = None
        self.__deactivateEventHandlers()

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
        self.__pressCallback = utils.methodref(handler)
        
    def setClickHandler(self, handler):
        self.__clickCallback = utils.methodref(handler)
    
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
            self.__activeAreaNode = avg.RectNode(opacity=0, size=self.__upNode.size)
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
        utils.callWeakRef(self.__stateChangeCallback, state)
    
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
            utils.callWeakRef(self.__pressCallback, event)
    
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
            
            utils.callWeakRef(self.__clickCallback, event)
    
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
            self.__activeAreaNode.connectEventHandler(type, avg.MOUSE | avg.TOUCH, 
                    self, handler)

        setOneHandler(avg.CURSORDOWN, self.__pressHandler)
        setOneHandler(avg.CURSORUP, self.__releaseHandler)
        setOneHandler(avg.CURSOROVER, self.__overHandler)
        setOneHandler(avg.CURSOROUT, self.__outHandler)
    
    def __deactivateEventHandlers(self):
        for id in self.__capturedCursorIds:
            self.releaseEventCapture(id)
        self.__capturedCursorIds = set()
        self.__activeAreaNode.disconnectEventHandler(self)


class TouchButton(avg.DivNode):

    def __init__(self, upNode, downNode, disabledNode=None, activeAreaNode=None, 
            fatFingerEnlarge=False, clickHandler=None, parent=None, **kwargs):
        super(TouchButton, self).__init__(**kwargs)
        self.registerInstance(self, parent)

        self.__upNode = upNode
        self.__downNode = downNode
        self.__disabledNode = disabledNode
        self.__activeAreaNode = activeAreaNode
        
        self.__clickHandler = utils.methodref(clickHandler)

        self.__stateMachine = statemachine.StateMachine("TouchButton", "UP")
        self.__stateMachine.addState("UP", ("DOWN", "DISABLED"),
                enterFunc=self.__enterUp, leaveFunc=self.__leaveUp)
        self.__stateMachine.addState("DOWN", ("UP", "DISABLED"),
                enterFunc=self.__enterDown, leaveFunc=self.__leaveDown)
        self.__stateMachine.addState("DISABLED", ("UP",),
                enterFunc=self.__enterDisabled, leaveFunc=self.__leaveDisabled)

        self.appendChild(self.__upNode)
        self.__upNode.active = True
        self.appendChild(self.__downNode)
        self.__downNode.active = False

        if self.__disabledNode:
            self.appendChild(self.__disabledNode)
            self.__disabledNode.active = False
        
        if fatFingerEnlarge:
            if self.__activeAreaNode != None:
                raise(RuntimeError(
                    "TouchButton: Can't specify both fatFingerEnlarge and activeAreaNode"))
            size = upNode.size
            minSize = 20*g_Player.getPixelsPerMM()
            size = avg.Point2D(max(minSize, size.x), max(minSize, size.y))
            self.__activeAreaNode = avg.RectNode(size=size, opacity=0, parent=self)
        else:
            if self.__activeAreaNode == None:
                self.__activeAreaNode = self.__upNode
            else:
                self.appendChild(self.__activeAreaNode)

        self.__tapRecognizer = gesture.TapRecognizer(self.__activeAreaNode,
                possibleHandler=self.__onDown, 
                detectedHandler=self.__onTap, 
                failHandler=self.__onTapFail)

    @classmethod
    def fromSrc(cls, upSrc, downSrc, disabledSrc=None, **kwargs):
        upNode = avg.ImageNode(href=upSrc)
        downNode = avg.ImageNode(href=downSrc)
        if disabledSrc != None:
            disabledNode = avg.ImageNode(href=disabledSrc)
        else:
            disabledNode = None
        return TouchButton(upNode=upNode, downNode=downNode, disabledNode=disabledNode,
                **kwargs)

    def getEnabled(self):
        return self.__stateMachine.state != "DISABLED"

    def setEnabled(self, enabled):
        if enabled:
            if self.__stateMachine.state == "DISABLED":
                self.__stateMachine.changeState("UP")
        else:
            if self.__stateMachine.state != "DISABLED":
                self.__stateMachine.changeState("DISABLED")

    enabled = property(getEnabled, setEnabled)

    def __onDown(self, event):
        self.__stateMachine.changeState("DOWN")

    def __onTap(self, event):
        self.__stateMachine.changeState("UP")
        utils.callWeakRef(self.__clickHandler, event)

    def __onTapFail(self, event):
        self.__stateMachine.changeState("UP")

    def __enterUp(self):
        self.__upNode.active = True

    def __leaveUp(self):
        self.__upNode.active = False

    def __enterDown(self):
        self.__downNode.active = True

    def __leaveDown(self):
        self.__downNode.active = False

    def __enterDisabled(self):
        if self.__disabledNode:
            self.__disabledNode.active = True
        self.__tapRecognizer.enable(False)

    def __leaveDisabled(self):
        if self.__disabledNode:
            self.__disabledNode.active = False
        self.__tapRecognizer.enable(True)


class ToggleButton(avg.DivNode):

    def __init__(self, uncheckedUpNode, uncheckedDownNode, checkedUpNode, checkedDownNode,
            uncheckedDisabledNode=None, checkedDisabledNode=None, activeAreaNode=None,
            enabled=True, fatFingerEnlarge=False, checkHandler=None, uncheckHandler=None,
            checked=False, parent=None, **kwargs):
        super(ToggleButton, self).__init__(**kwargs)
        self.registerInstance(self, parent)

        self.__uncheckedUpNode = uncheckedUpNode
        self.__uncheckedDownNode = uncheckedDownNode
        self.__checkedUpNode = checkedUpNode
        self.__checkedDownNode = checkedDownNode
        self.__uncheckedDisabledNode = uncheckedDisabledNode
        self.__checkedDisabledNode = checkedDisabledNode

        self.__activeAreaNode = activeAreaNode
        
        self.__checkHandler = utils.methodref(checkHandler)
        self.__uncheckHandler = utils.methodref(uncheckHandler)

        self.__stateMachine = statemachine.StateMachine("ToggleButton", "UNCHECKED_UP")
        self.__stateMachine.addState("UNCHECKED_UP", ("UNCHECKED_DOWN",
                "UNCHECKED_DISABLED"), enterFunc=self.__enterUncheckedUp,
                leaveFunc=self.__leaveUncheckedUp)
        self.__stateMachine.addState("UNCHECKED_DOWN", ("UNCHECKED_UP",
                "UNCHECKED_DISABLED", "CHECKED_UP"), enterFunc=self.__enterUncheckedDown,
                leaveFunc=self.__leaveUncheckedDown)
        self.__stateMachine.addState("CHECKED_UP", ("CHECKED_DOWN", "CHECKED_DISABLED"),
                enterFunc=self.__enterCheckedUp, leaveFunc=self.__leaveCheckedUp)
        self.__stateMachine.addState("CHECKED_DOWN", ("CHECKED_UP", "UNCHECKED_UP",
                "CHECKED_DISABLED"), enterFunc=self.__enterCheckedDown,
                leaveFunc=self.__leaveCheckedDown)

        self.__stateMachine.addState("UNCHECKED_DISABLED", ("UNCHECKED_UP",),
                enterFunc=self.__enterUncheckedDisabled,
                leaveFunc=self.__leaveUncheckedDisabled)
        self.__stateMachine.addState("CHECKED_DISABLED", ("CHECKED_UP", ),
                enterFunc=self.__enterCheckedDisabled,
                leaveFunc=self.__leaveCheckedDisabled)

        if(checkedDisabledNode == None):
            self.__checkedDisabledNode = avg.ImageNode()             
        if(uncheckedDisabledNode == None):
            self.__uncheckedDisabledNode = avg.ImageNode()                   

        self.appendChild(self.__uncheckedUpNode)
        self.appendChild(self.__checkedUpNode)
        self.appendChild(self.__uncheckedDownNode)
        self.appendChild(self.__checkedDownNode)
        self.appendChild(self.__uncheckedDisabledNode)
        self.appendChild(self.__checkedDisabledNode)

        self.__uncheckedUpNode.active = True
        self.__checkedUpNode.active = False
        self.__uncheckedDownNode.active = False
        self.__checkedDownNode.active = False
        self.__checkedDisabledNode.active = False
        self.__uncheckedDisabledNode.active = False
        
        if fatFingerEnlarge:
            if self.__activeAreaNode != None:
                raise(RuntimeError(
                    "ToggleButton: Can't specify both fatFingerEnlarge and activeAreaNode"))
            size = upNode.size
            minSize = 20*g_Player.getPixelsPerMM()
            size = avg.Point2D(max(minSize, size.x), max(minSize, size.y))
            self.__activeAreaNode = avg.RectNode(size=size, opacity=0, parent=self)
        else:
            if self.__activeAreaNode == None:
                self.__activeAreaNode = self
            else:
                self.appendChild(self.__activeAreaNode)

        self.__tapRecognizer = gesture.TapRecognizer(self.__activeAreaNode,
                possibleHandler=self.__onDown, 
                detectedHandler=self.__onTap, 
                failHandler=self.__onTapFail)

        if not enabled:
            self.__stateMachine.changeState("UNCHECKED_DISABLED")
        if checked:
            self.setChecked(True)

    @classmethod
    def fromSrc(cls, uncheckedUpSrc, uncheckedDownSrc, checkedUpSrc, checkedDownSrc,
            uncheckedDisabledSrc=None, checkedDisabledSrc=None, **kwargs):

        uncheckedUpNode = avg.ImageNode(href=uncheckedUpSrc)
        uncheckedDownNode = avg.ImageNode(href=uncheckedDownSrc)
        checkedUpNode = avg.ImageNode(href=checkedUpSrc)
        checkedDownNode = avg.ImageNode(href=checkedDownSrc)

        if uncheckedDisabledSrc != None:
            uncheckedDisabledNode = avg.ImageNode(href=uncheckedDisabledSrc)
        else:
            uncheckedDisabledNode = None
        if checkedDisabledSrc != None:
            checkedDisabledNode = avg.ImageNode(href=checkedDisabledSrc)
        else:
            checkedDisabledNode = None

        return ToggleButton(uncheckedUpNode=uncheckedUpNode,
                uncheckedDownNode=uncheckedDownNode, checkedUpNode=checkedUpNode,
                checkedDownNode=checkedDownNode,
                uncheckedDisabledNode=uncheckedDisabledNode,
                checkedDisabledNode=checkedDisabledNode, **kwargs)

    def getEnabled(self):
        return (self.__stateMachine.state != "CHECKED_DISABLED" and
                self.__stateMachine.state != "UNCHECKED_DISABLED")

    def setEnabled(self, enabled):
        if enabled:
            if self.__stateMachine.state == "CHECKED_DISABLED":
                self.__stateMachine.changeState("CHECKED_UP")
            elif self.__stateMachine.state == "UNCHECKED_DISABLED":
                self.__stateMachine.changeState("UNCHECKED_UP")
        else:
            if (self.__stateMachine.state == "CHECKED_UP" or
                    self.__stateMachine.state == "CHECKED_DOWN") :
                self.__stateMachine.changeState("CHECKED_DISABLED")
            elif (self.__stateMachine.state == "UNCHECKED_UP" or
                    self.__stateMachine.state == "UNCHECKED_DOWN") :
                self.__stateMachine.changeState("UNCHECKED_DISABLED")

    enabled = property(getEnabled, setEnabled)

    def getChecked(self):
        return (self.__stateMachine.state != "UNCHECKED_UP" and
                self.__stateMachine.state != "UNCHECKED_DOWN" and
                self.__stateMachine.state != "UNCHECKED_DISABLED")

    def setChecked(self, checked):
        oldEnabled = self.getEnabled()
        if checked:
            if self.__stateMachine.state == "UNCHECKED_DISABLED":
                self.__stateMachine.changeState("UNCHECKED_UP")
            if self.__stateMachine.state == "UNCHECKED_UP":
                self.__stateMachine.changeState("UNCHECKED_DOWN")
            self.__stateMachine.changeState("CHECKED_UP")
            if not oldEnabled:
                self.__stateMachine.changeState("CHECKED_DISABLED")
        else:
            if self.__stateMachine.state == "CHECKED_DISABLED":
                self.__stateMachine.changeState("CHECKED_UP")
            if self.__stateMachine.state == "CHECKED_UP":
                self.__stateMachine.changeState("CHECKED_DOWN")
            self.__stateMachine.changeState("UNCHECKED_UP")
            if not oldEnabled:
                self.__stateMachine.changeState("UNCHECKED_DISABLED")

    checked = property(getChecked, setChecked)

    def getState(self):
        return self.__stateMachine.state

    def __enterUncheckedUp(self):
        self.__uncheckedUpNode.active = True

    def __leaveUncheckedUp(self):
        self.__uncheckedUpNode.active = False

    def __enterUncheckedDown(self):
        self.__uncheckedDownNode.active = True

    def __leaveUncheckedDown(self):
        self.__uncheckedDownNode.active = False

    def __enterCheckedUp(self):
        self.__checkedUpNode.active = True

    def __leaveCheckedUp(self):
        self.__checkedUpNode.active = False

    def __enterCheckedDown(self):
        self.__checkedDownNode.active = True

    def __leaveCheckedDown(self):
        self.__checkedDownNode.active = False

    def __enterUncheckedDisabled(self):
        self.__uncheckedDisabledNode.active = True
        self.__tapRecognizer.enable(False)

    def __leaveUncheckedDisabled(self):
        self.__uncheckedDisabledNode.active = False
        self.__tapRecognizer.enable(True)

    def __enterCheckedDisabled(self):
        self.__checkedDisabledNode.active = True
        self.__tapRecognizer.enable(False)

    def __leaveCheckedDisabled(self):
        self.__checkedDisabledNode.active = False
        self.__tapRecognizer.enable(True)

    def __onDown(self, event):
        if self.__stateMachine.state == "UNCHECKED_UP":
            self.__stateMachine.changeState("UNCHECKED_DOWN")
        elif self.__stateMachine.state == "CHECKED_UP":
            self.__stateMachine.changeState("CHECKED_DOWN")

    def __onTap(self, event):
        if self.__stateMachine.state == "UNCHECKED_DOWN":
            self.__stateMachine.changeState("CHECKED_UP")
            utils.callWeakRef(self.__checkHandler, event)
        elif self.__stateMachine.state == "CHECKED_DOWN":
            self.__stateMachine.changeState("UNCHECKED_UP")
            utils.callWeakRef(self.__uncheckHandler, event)

    def __onTapFail(self, event):
        if self.__stateMachine.state == "UNCHECKED_DOWN":
            self.__stateMachine.changeState("UNCHECKED_UP")
        elif self.__stateMachine.state == "CHECKED_DOWN":
            self.__stateMachine.changeState("CHECKED_UP")
