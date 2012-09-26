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

from libavg import avg, statemachine, player
import gesture


class SwitchNode(avg.DivNode):

    def __init__(self, nodeMap=None, visibleid=None, parent=None, **kwargs):
        super(SwitchNode, self).__init__(**kwargs)
        self.registerInstance(self, parent)
  
        self.__nodeMap = nodeMap
        if nodeMap:
            self.setNodeMap(nodeMap)
        if visibleid:
            self.setVisibleID(visibleid)

    def setNodeMap(self, nodeMap):
        self.__nodeMap = nodeMap
        for node in self.__nodeMap.itervalues():
            if node:
                # Only insert child if it hasn't been inserted yet.
                try:
                    self.indexOf(node)
                except RuntimeError:
                    self.appendChild(node)
        if self.size != (0,0):
            size = self.size
        else:
            key = list(self.__nodeMap.keys())[0]
            size = self.__nodeMap[key].size
        self.setSize(size)

    def getVisibleID(self):
        return self.__visibleid

    def setVisibleID(self, visibleid):
        if not (visibleid in self.__nodeMap):
            raise RuntimeError("'%s' is not a registered id." % visibleid)
        self.__visibleid = visibleid
        for node in self.__nodeMap.itervalues():
            node.active = False
        self.__nodeMap[visibleid].active = True

    visibleid = property(getVisibleID, setVisibleID)

    def getWidth(self):
        return self.__baseWidth

    def setWidth(self, width):
        self.__baseWidth = width
        self.__setChildSizes()

    __baseWidth = avg.DivNode.width
    width = property(getWidth, setWidth)

    def getHeight(self):
        return self.__baseHeight

    def setHeight(self, height):
        self.__baseHeight = height
        self.__setChildSizes()

    __baseHeight = avg.DivNode.height
    height = property(getHeight, setHeight)

    def getSize(self):
        return self.__baseSize

    def setSize(self, size):
        self.__baseSize = size
        self.__setChildSizes()
        
    __baseSize = avg.DivNode.size
    size = property(getSize, setSize)   

    def __setChildSizes(self):
        if self.__nodeMap:
            for node in self.__nodeMap.itervalues():
                if node:
                    node.size = self.__baseSize
                    # Hack to support min. size in AccordionNodes
                    self.__baseSize = node.size

class _ButtonBase(avg.DivNode):

    PRESSED = avg.Publisher.genMessageID()
    RELEASED = avg.Publisher.genMessageID()

    def __init__(self, parent=None, **kwargs):
        super(_ButtonBase, self).__init__(**kwargs)
        self.registerInstance(self, parent)
        self.publish(self.PRESSED)
        self.publish(self.RELEASED)
        
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

    CLICKED = avg.Publisher.genMessageID()

    def __init__(self, upNode, downNode, disabledNode=None, activeAreaNode=None, 
            enabled=True, fatFingerEnlarge=False, **kwargs):
        super(Button, self).__init__(**kwargs)

        if disabledNode == None:
            disabledNode = upNode

        nodeMap = {
            "UP": upNode, 
            "DOWN": downNode, 
            "DISABLED": disabledNode
        }
        self.__switchNode = SwitchNode(nodeMap=nodeMap, visibleid="UP", parent=self)
        self.publish(self.CLICKED)

        self.__stateMachine = statemachine.StateMachine("Button", "UP")
        self.__stateMachine.addState("UP", ("DOWN", "DISABLED"),
                enterFunc=self._enterUp, leaveFunc=self._leaveUp)
        self.__stateMachine.addState("DOWN", ("UP", "DISABLED"),
                enterFunc=self._enterDown, leaveFunc=self._leaveDown)
        self.__stateMachine.addState("DISABLED", ("UP",),
                enterFunc=self._enterDisabled, leaveFunc=self._leaveDisabled)

        self._setActiveArea(upNode, activeAreaNode, fatFingerEnlarge)

        if not(enabled):
            self.setEnabled(False)

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

    def _onDown(self):
        self.__stateMachine.changeState("DOWN")
        self.notifySubscribers(self.PRESSED, [])

    def _onTap(self):
        self.__stateMachine.changeState("UP")
        self.notifySubscribers(self.CLICKED, [])
        self.notifySubscribers(self.RELEASED, [])

    def _onTapFail(self):
        self.__stateMachine.changeState("UP")
        self.notifySubscribers(self.RELEASED, [])

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

    def __setActiveNode(self):
        self.__switchNode.visibleid = self.__stateMachine.state


class BmpButton(Button):
    def __init__(self, upSrc, downSrc, disabledSrc=None, **kwargs):
        upNode = avg.ImageNode(href=upSrc)
        downNode = avg.ImageNode(href=downSrc)
        if disabledSrc != None:
            disabledNode = avg.ImageNode(href=disabledSrc)
        else:
            disabledNode = None
        super(BmpButton, self).__init__(upNode=upNode, downNode=downNode, 
                disabledNode=disabledNode, **kwargs)


class ToggleButton(_ButtonBase):
    
    TOGGLED = avg.Publisher.genMessageID()

    def __init__(self, uncheckedUpNode, uncheckedDownNode, checkedUpNode, checkedDownNode,
            uncheckedDisabledNode=None, checkedDisabledNode=None, activeAreaNode=None,
            enabled=True, fatFingerEnlarge=False, checked=False, **kwargs):
        super(ToggleButton, self).__init__(**kwargs)
        nodeMap = {
            "UNCHECKED_UP": uncheckedUpNode, 
            "UNCHECKED_DOWN": uncheckedDownNode, 
            "CHECKED_UP": checkedUpNode, 
            "CHECKED_DOWN": checkedDownNode, 
            "UNCHECKED_DISABLED": uncheckedDisabledNode, 
            "CHECKED_DISABLED": checkedDisabledNode, 
        }
        if uncheckedDisabledNode == None:
            nodeMap["UNCHECKED_DISABLED"] = uncheckedUpNode
        if checkedDisabledNode == None:
            nodeMap["CHECKED_DISABLED"] = checkedUpNode
        self.__switchNode = SwitchNode(nodeMap=nodeMap, visibleid="UNCHECKED_UP", 
                parent=self)

        self.publish(ToggleButton.TOGGLED)

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

        self._setActiveArea(uncheckedUpNode, activeAreaNode, fatFingerEnlarge)

        if not enabled:
            self.__stateMachine.changeState("UNCHECKED_DISABLED")
        if checked:
            self.setChecked(True)

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

    def _onDown(self):
        if self.__stateMachine.state == "UNCHECKED_UP":
            self.__stateMachine.changeState("UNCHECKED_DOWN")
        elif self.__stateMachine.state == "CHECKED_UP":
            self.__stateMachine.changeState("CHECKED_DOWN")
        self.notifySubscribers(self.PRESSED, [])

    def _onTap(self):
        if self.__stateMachine.state == "UNCHECKED_DOWN":
            self.__stateMachine.changeState("CHECKED_UP")
            self.notifySubscribers(ToggleButton.TOGGLED, [True])
        elif self.__stateMachine.state == "CHECKED_DOWN":
            self.__stateMachine.changeState("UNCHECKED_UP")
            self.notifySubscribers(ToggleButton.TOGGLED, [False])
        self.notifySubscribers(self.RELEASED, [])

    def _onTapFail(self):
        if self.__stateMachine.state == "UNCHECKED_DOWN":
            self.__stateMachine.changeState("UNCHECKED_UP")
        elif self.__stateMachine.state == "CHECKED_DOWN":
            self.__stateMachine.changeState("CHECKED_UP")
        self.notifySubscribers(self.RELEASED, [])
    
    def __setActiveNode(self):
        self.__switchNode.visibleid = self.__stateMachine.state


class BmpToggleButton(ToggleButton):
    def __init__(self, uncheckedUpSrc, uncheckedDownSrc, checkedUpSrc, checkedDownSrc,
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

        super(BmpToggleButton, self).__init__(uncheckedUpNode=uncheckedUpNode,
                uncheckedDownNode=uncheckedDownNode, 
                checkedUpNode=checkedUpNode,
                checkedDownNode=checkedDownNode,
                uncheckedDisabledNode=uncheckedDisabledNode,
                checkedDisabledNode=checkedDisabledNode, 
                **kwargs)

