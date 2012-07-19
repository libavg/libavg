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

from libavg import avg, statemachine, utils, player
import gesture


class _ButtonBase(avg.DivNode):
    
    def __init__(self, parent=None, **kwargs):
        super(_ButtonBase, self).__init__(**kwargs)
        self.registerInstance(self, parent)
        
    def _setActiveArea(self, upNode, activeAreaNode, fatFingerEnlarge):
        self.__activeAreaNode = activeAreaNode

        if fatFingerEnlarge:
            if self.__activeAreaNode != None:
                raise(RuntimeError(
                    "Button: Can't specify both fatFingerEnlarge and activeAreaNode"))
            size = upNode.size
            minSize = 20*player.getPixelsPerMM()
            size = avg.Point2D(max(minSize, size.x), max(minSize, size.y))
            self.__activeAreaNode = avg.RectNode(size=size, opacity=0, parent=self)
        else:
            if self.__activeAreaNode == None:
                self.__activeAreaNode = self
            else:
                self.appendChild(self.__activeAreaNode)

        self._tapRecognizer = gesture.TapRecognizer(self.__activeAreaNode,
                possibleHandler=self._onDown, 
                detectedHandler=self._onTap, 
                failHandler=self._onTapFail)


class Button(_ButtonBase):

    def __init__(self, upNode, downNode, disabledNode=None, activeAreaNode=None, 
            enabled=True, fatFingerEnlarge=False, clickHandler=None,
            **kwargs):
        super(Button, self).__init__(**kwargs)

        self.__nodeMap = {
            "UP": upNode, 
            "DOWN": downNode, 
            "DISABLED": disabledNode
        }
        for node in self.__nodeMap.itervalues():
            if node:
                self.appendChild(node)
        if disabledNode == None:
            self.__nodeMap["DISABLED"] = upNode

        self.__clickHandler = utils.methodref(clickHandler)

        self.__stateMachine = statemachine.StateMachine("Button", "UP")
        self.__stateMachine.addState("UP", ("DOWN", "DISABLED"),
                enterFunc=self._enterUp, leaveFunc=self._leaveUp)
        self.__stateMachine.addState("DOWN", ("UP", "DISABLED"),
                enterFunc=self._enterDown, leaveFunc=self._leaveDown)
        self.__stateMachine.addState("DISABLED", ("UP",),
                enterFunc=self._enterDisabled, leaveFunc=self._leaveDisabled)

        self.__setActiveNode("UP")

        self._setActiveArea(upNode, activeAreaNode, fatFingerEnlarge)

        if not(enabled):
            self.setEnabled(False)

    @classmethod
    def fromSrc(cls, upSrc, downSrc, disabledSrc=None, **kwargs):
        upNode = avg.ImageNode(href=upSrc)
        downNode = avg.ImageNode(href=downSrc)
        if disabledSrc != None:
            disabledNode = avg.ImageNode(href=disabledSrc)
        else:
            disabledNode = None
        return Button(upNode=upNode, downNode=downNode, disabledNode=disabledNode,
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

    def _getState(self):
        return self.__stateMachine.state

    def _onDown(self, event):
        self.__stateMachine.changeState("DOWN")

    def _onTap(self, event):
        self.__stateMachine.changeState("UP")
        utils.callWeakRef(self.__clickHandler, event)

    def _onTapFail(self, event):
        self.__stateMachine.changeState("UP")

    def _enterUp(self):
        self.__setActiveNode()

    def _leaveUp(self):
        pass

    def _enterDown(self):
        self.__setActiveNode()

    def _leaveDown(self):
        pass

    def _enterDisabled(self):
        self.__setActiveNode()
        self._tapRecognizer.enable(False)

    def _leaveDisabled(self):
        self._tapRecognizer.enable(True)

    def __setActiveNode(self, state=None):
        if state == None:
            state = self.__stateMachine.state
        for node in self.__nodeMap.itervalues():
            node.active = False
        self.__nodeMap[state].active = True


class ToggleButton(_ButtonBase):

    def __init__(self, uncheckedUpNode, uncheckedDownNode, checkedUpNode, checkedDownNode,
            uncheckedDisabledNode=None, checkedDisabledNode=None, activeAreaNode=None,
            enabled=True, fatFingerEnlarge=False, checkHandler=None,
            checked=False, **kwargs):
        super(ToggleButton, self).__init__(**kwargs)

        self.__nodeMap = {
            "UNCHECKED_UP": uncheckedUpNode, 
            "UNCHECKED_DOWN": uncheckedDownNode, 
            "CHECKED_UP": checkedUpNode, 
            "CHECKED_DOWN": checkedDownNode, 
            "UNCHECKED_DISABLED": uncheckedDisabledNode, 
            "CHECKED_DISABLED": checkedDisabledNode, 
        }
        for node in self.__nodeMap.itervalues():
            if node:
                self.appendChild(node)
        if uncheckedDisabledNode == None:
            self.__nodeMap["UNCHECKED_DISABLED"] = uncheckedUpNode
        if checkedDisabledNode == None:
            self.__nodeMap["CHECKED_DISABLED"] = checkedUpNode

        self.__checkHandler = utils.methodref(checkHandler)

        self.__stateMachine = statemachine.StateMachine("ToggleButton", "UNCHECKED_UP")
        self.__stateMachine.addState("UNCHECKED_UP", ("UNCHECKED_DOWN",
                "UNCHECKED_DISABLED"), enterFunc=self._enterUncheckedUp,
                leaveFunc=self._leaveUncheckedUp)
        self.__stateMachine.addState("UNCHECKED_DOWN", ("UNCHECKED_UP",
                "UNCHECKED_DISABLED", "CHECKED_UP"), enterFunc=self._enterUncheckedDown,
                leaveFunc=self._leaveUncheckedDown)
        self.__stateMachine.addState("CHECKED_UP", ("CHECKED_DOWN", "CHECKED_DISABLED"),
                enterFunc=self._enterCheckedUp, leaveFunc=self._leaveCheckedUp)
        self.__stateMachine.addState("CHECKED_DOWN", ("CHECKED_UP", "UNCHECKED_UP",
                "CHECKED_DISABLED"), enterFunc=self._enterCheckedDown,
                leaveFunc=self._leaveCheckedDown)
        self.__stateMachine.addState("UNCHECKED_DISABLED", ("UNCHECKED_UP",),
                enterFunc=self._enterUncheckedDisabled,
                leaveFunc=self._leaveUncheckedDisabled)
        self.__stateMachine.addState("CHECKED_DISABLED", ("CHECKED_UP", ),
                enterFunc=self._enterCheckedDisabled,
                leaveFunc=self._leaveCheckedDisabled)

        self.__setActiveNode("UNCHECKED_UP")

        self._setActiveArea(uncheckedUpNode, activeAreaNode, fatFingerEnlarge)

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
                uncheckedDownNode=uncheckedDownNode, 
                checkedUpNode=checkedUpNode,
                checkedDownNode=checkedDownNode,
                uncheckedDisabledNode=uncheckedDisabledNode,
                checkedDisabledNode=checkedDisabledNode, 
                **kwargs)

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
            if self.__stateMachine.state != "CHECKED_UP":
                self.__stateMachine.changeState("CHECKED_UP")
            if not oldEnabled:
                self.__stateMachine.changeState("CHECKED_DISABLED")
        else:
            if self.__stateMachine.state == "CHECKED_DISABLED":
                self.__stateMachine.changeState("CHECKED_UP")
            if self.__stateMachine.state == "CHECKED_UP":
                self.__stateMachine.changeState("CHECKED_DOWN")
            if self.__stateMachine.state != "UNCHECKED_UP":
                self.__stateMachine.changeState("UNCHECKED_UP")
            if not oldEnabled:
                self.__stateMachine.changeState("UNCHECKED_DISABLED")

    checked = property(getChecked, setChecked)

    def _getState(self):
        return self.__stateMachine.state

    def _enterUncheckedUp(self):
        self.__setActiveNode()

    def _leaveUncheckedUp(self):
        pass

    def _enterUncheckedDown(self):
        self.__setActiveNode()

    def _leaveUncheckedDown(self):
        pass

    def _enterCheckedUp(self):
        self.__setActiveNode()

    def _leaveCheckedUp(self):
        pass

    def _enterCheckedDown(self):
        self.__setActiveNode()

    def _leaveCheckedDown(self):
        pass

    def _enterUncheckedDisabled(self):
        self.__setActiveNode()
        self._tapRecognizer.enable(False)

    def _leaveUncheckedDisabled(self):
        self._tapRecognizer.enable(True)

    def _enterCheckedDisabled(self):
        self.__setActiveNode()
        self._tapRecognizer.enable(False)

    def _leaveCheckedDisabled(self):
        self._tapRecognizer.enable(True)

    def _onDown(self, event):
        if self.__stateMachine.state == "UNCHECKED_UP":
            self.__stateMachine.changeState("UNCHECKED_DOWN")
        elif self.__stateMachine.state == "CHECKED_UP":
            self.__stateMachine.changeState("CHECKED_DOWN")

    def _onTap(self, event):
        if self.__stateMachine.state == "UNCHECKED_DOWN":
            self.__stateMachine.changeState("CHECKED_UP")
            utils.callWeakRef(self.__checkHandler, event, True)
        elif self.__stateMachine.state == "CHECKED_DOWN":
            self.__stateMachine.changeState("UNCHECKED_UP")
            utils.callWeakRef(self.__checkHandler, event, False)

    def _onTapFail(self, event):
        if self.__stateMachine.state == "UNCHECKED_DOWN":
            self.__stateMachine.changeState("UNCHECKED_UP")
        elif self.__stateMachine.state == "CHECKED_DOWN":
            self.__stateMachine.changeState("CHECKED_UP")
    
    def __setActiveNode(self, state=None):
        if state == None:
            state = self.__stateMachine.state
        for node in self.__nodeMap.itervalues():
            node.active = False
        self.__nodeMap[state].active = True

