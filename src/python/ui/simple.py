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

from libavg import avg
from libavg.ui import button, slider, scrollarea


class TextButton(button.Button):
    def __init__(self, text, **kwargs):
        size = kwargs["size"]
        upNode = avg.DivNode(size=size)
        avg.RectNode(size=size, fillcolor="FFFFFF", fillopacity=1, color="FFFFFF",
                parent=upNode)
        avg.WordsNode(pos=(4,3), text=text, color="000000", parent=upNode)
        downNode = avg.DivNode(size=size)
        avg.RectNode(size=size, fillcolor="000000", fillopacity=1, color="FFFFFF",
                parent=downNode)
        avg.WordsNode(pos=(4,3), text=text, color="FFFFFF", parent=downNode)
        kwargs["upNode"] = upNode
        kwargs["downNode"] = downNode
        super(TextButton, self).__init__(**kwargs)


class CheckBox(button.ToggleButton):

    UP = 0
    DOWN = 1
    DISABLED = 2

    def __init__(self, text, **kwargs):
        uncheckedUpNode = self.__createNode(CheckBox.UP, checked=False)
        uncheckedDownNode = self.__createNode(CheckBox.DOWN, checked=False)
        checkedUpNode = self.__createNode(CheckBox.UP, checked=True)
        checkedDownNode = self.__createNode(CheckBox.DOWN, checked=True)
        uncheckedDisabledNode = self.__createNode(CheckBox.DISABLED, checked=False)
        checkedDisabledNode = self.__createNode(CheckBox.DISABLED, checked=True)

        super(CheckBox, self).__init__(uncheckedUpNode, uncheckedDownNode, checkedUpNode,
                checkedDownNode, uncheckedDisabledNode, checkedDisabledNode,
                **kwargs)

        self._textNode = avg.WordsNode(pos=(19,0), text=text, parent=self)
        self.__oldEnabled = self.enabled

    def __createNode(self, state, checked):
        if state == CheckBox.UP:
            color = "FFFFFF"
            fillcolor="000000"
        elif state == CheckBox.DOWN:
            color = "FFFFFF"
            fillcolor = "808080"
        elif state == CheckBox.DISABLED:
            color = "808080"
            fillcolor = "404040"

        node = avg.DivNode(size=(15,15))
        avg.RectNode(pos=(0.5,0.5), size=(14,14), color=color, fillcolor=fillcolor, 
                fillopacity=1, parent=node)

        if checked:
            avg.LineNode(pos1=(2.5,2.5), pos2=(12.5,12.5), color=color, parent=node)
            avg.LineNode(pos1=(2.5,12.5), pos2=(12.5,2.5), color=color, parent=node)

        return node

    def _enterUncheckedUp(self):
        self.__checkEnabledChange()
        super(CheckBox, self)._enterUncheckedUp()

    def _enterUncheckedDown(self):
        self.__checkEnabledChange()
        super(CheckBox, self)._enterUncheckedDown()

    def _enterCheckedUp(self):
        self.__checkEnabledChange()
        super(CheckBox, self)._enterCheckedUp()

    def _enterCheckedDown(self):
        self.__checkEnabledChange()
        super(CheckBox, self)._enterCheckedDown()

    def _enterUncheckedDisabled(self):
        self.__checkEnabledChange()
        super(CheckBox, self)._enterUncheckedDisabled()

    def _enterCheckedDisabled(self):
        self.__checkEnabledChange()
        super(CheckBox, self)._enterCheckedDisabled()

    def __checkEnabledChange(self):
        if self.__oldEnabled != self.enabled:
            if self.enabled:
                self._textNode.color = "FFFFFF"
            else:
                self._textNode.color = "808080"
            self.__oldEnabled = self.enabled


class SliderTrack(button.SwitchNode):
    def __init__(self, margin=(0,0), orientation=slider.Orientation.HORIZONTAL,
            sensitive=True, **kwargs):
        self.__margin = avg.Point2D(margin)
        self.__orientation = orientation
        style = avg.Style(pos=(0.5,0.5), fillopacity=1)
        if sensitive:
            self.__enabledNode = avg.RectNode(fillcolor="000000", color="FFFFFF",
                    style=style)
        else:
            self.__enabledNode = avg.RectNode(fillcolor="404040", strokewidth=0,
                    style=style)
        self.__disabledNode = avg.RectNode(fillcolor="000000", color="808080",
                style=style)
        nodeMap = {
            "ENABLED": self.__enabledNode,
            "DISABLED": self.__disabledNode
        }
        super(SliderTrack, self).__init__(nodeMap=nodeMap, visibleid="ENABLED", 
                sensitive=sensitive, **kwargs)
        self.pos = self.__margin
        self.size -= 2*self.__margin

    def getSize(self):
        return self.__baseSize + 2*self.__margin

    def setSize(self, size):
        self.__baseSize = size - 2*self.__margin
    __baseSize = button.SwitchNode.size
    size = property(getSize, setSize)   


class Slider(slider.Slider):
    
    class Thumb(button.SwitchNode):

        def __init__(self, orientation, **kwargs):
            if orientation == slider.Orientation.HORIZONTAL:
                pos=((1,0), (13,0), (7,18))
                size=(14,20)
            else:
                pos=((18,1), (18,13), (1,7))
                size=(20,14)
            
            style = avg.Style(pos=pos, fillopacity=1)
            self.__upNode = avg.PolygonNode(fillcolor="808080", color="FFFFFF",
                    style=style)
            self.__downNode = avg.PolygonNode(fillcolor="C0C0C0", color="FFFFFF",
                    style=style)
            self.__disabledNode = avg.PolygonNode(fillcolor="404040", color="808080",
                    style=style)
            nodeMap = {
                "UP": self.__upNode,
                "DOWN": self.__downNode,
                "DISABLED": self.__disabledNode
            }
            super(Slider.Thumb, self).__init__(
                    nodeMap=nodeMap, visibleid="UP", size=size, **kwargs)

    def __init__(self, orientation=slider.Orientation.HORIZONTAL, **kwargs):
        
        if orientation == slider.Orientation.HORIZONTAL:
            trackMargin = (7, 8)
        else:
            trackMargin = (8, 7)
        trackNode = SliderTrack(margin=trackMargin, size=kwargs["size"],
                orientation=orientation)
        thumbNode = Slider.Thumb(orientation)

        super(Slider, self).__init__(orientation=orientation, trackNode=trackNode,
                thumbNode=thumbNode, **kwargs)


class ScrollBar(slider.ScrollBar):

    class Thumb(button.SwitchNode):
        def __init__(self, size, orientation, **kwargs):
            self.__orientation = orientation
            style = avg.Style(pos=(1.5,1.5), fillopacity=1)
            self.__upNode = avg.RectNode(fillcolor="808080", color="808080", style=style)
            self.__downNode = avg.RectNode(fillcolor="C0C0C0", color="C0C0C0", 
                    style=style)
            self.__disabledNode = avg.RectNode(fillcolor="404040", color="404040",
                    style=style)
            nodeMap = {
                "UP": self.__upNode,
                "DOWN": self.__downNode,
                "DISABLED": self.__disabledNode
            }
            super(ScrollBar.Thumb, self).__init__(
                    nodeMap=nodeMap, visibleid="UP", size=size, **kwargs)
            self.setWidth(size[0])
            self.setHeight(size[1])
        
        def getWidth(self):
            return self.__baseWidth+2

        def setWidth(self, width):
            self.__baseWidth = max(width-2, 0)

        __baseWidth = button.SwitchNode.width
        width = property(getWidth, setWidth)

        def getHeight(self):
            return self.__baseHeight+2

        def setHeight(self, height):
            self.__baseHeight = max(height-2, 0)

        __baseHeight = button.SwitchNode.height
        height = property(getHeight, setHeight)

    def __init__(self, size=(15,15), orientation=slider.Orientation.HORIZONTAL, 
            sensitive=True, **kwargs):
        trackNode = SliderTrack(size=size, orientation=orientation, sensitive=sensitive)
        thumbNode = ScrollBar.Thumb(size=size, orientation=orientation)

        super(ScrollBar, self).__init__(orientation=orientation, trackNode=trackNode,
                thumbNode=thumbNode, size=size, sensitive=sensitive, **kwargs)


class ScrollArea(scrollarea.ScrollArea):
    
    def __init__(self, contentNode, sensitiveScrollBars=True, parent=None, **kwargs):
        if sensitiveScrollBars:
            hScrollBar = ScrollBar(orientation=slider.Orientation.HORIZONTAL)
            vScrollBar = ScrollBar(orientation=slider.Orientation.VERTICAL)
        else:
            hScrollBar = ScrollBar(orientation=slider.Orientation.HORIZONTAL, size=(3,3),
                    sensitive=False)
            vScrollBar = ScrollBar(orientation=slider.Orientation.VERTICAL, size=(3,3),
                    sensitive=False)

        super(ScrollArea, self).__init__(contentNode=contentNode, hScrollBar=hScrollBar,
                vScrollBar=vScrollBar, **kwargs)
        self.registerInstance(self, parent)
