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


class SliderTrack(button.SwitchNode):
    def __init__(self, size, inset=(0,0), orientation=slider.Orientation.HORIZONTAL,
            sensitive=True, **kwargs):
        self.__inset = avg.Point2D(inset)
        self.__orientation = orientation
        style = avg.Style(pos=(0.5,0.5), size=size, fillopacity=1)
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
                sensitive=sensitive, size=size, **kwargs)
        self.pos = self.__inset
        self.size -= 2*self.__inset

    def getExtent(self):
        if self.__orientation == slider.Orientation.HORIZONTAL:
            return self.__enabledNode.width
        else:
            return self.__enabledNode.height

    def setExtent(self, extent):
        if self.__orientation == slider.Orientation.HORIZONTAL:
            self.size = avg.Point2D(extent-2*self.__inset.x, self.size.y)
        else:
            self.size = avg.Point2D(self.size.x, extent-2*self.__inset.y)
        for node in self.__enabledNode, self.__disabledNode:
            node.size = self.size

    extent = property(getExtent, setExtent)


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
            trackInset = (7, 8)
        else:
            trackInset = (8, 7)
        trackNode = SliderTrack(inset=trackInset, size=kwargs["size"],
                orientation=orientation)
        thumbNode = Slider.Thumb(orientation)

        super(Slider, self).__init__(orientation=orientation, trackNode=trackNode,
                thumbNode=thumbNode, **kwargs)

    def _getScrollRangeInPixels(self):
        if self._orientation == slider.Orientation.HORIZONTAL:
            return self.size.x - 2*7
        else:
            return self.size.y - 2*7


class ScrollBar(slider.ScrollBar):

    class Thumb(button.SwitchNode):
        def __init__(self, size, orientation, **kwargs):
            self.__orientation = orientation
            childSize = avg.Point2D(size) - (2,2)
            style = avg.Style(pos=(1.5,1.5), size=childSize, fillopacity=1)
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
                    nodeMap=nodeMap, visibleid="UP", size=size)

        def getExtent(self):
            if self.__orientation == slider.Orientation.HORIZONTAL:
                return self.width
            else:
                return self.height

        def setExtent(self, extent):
            if self.__orientation == slider.Orientation.HORIZONTAL:
                self.size = (extent, self.size.y)
            else:
                self.size = (self.size.x, extent)
            for node in self.__upNode, self.__downNode, self.__disabledNode:
                node.size = self.size - (2,2)

        extent = property(getExtent, setExtent)

    def __init__(self, size=(15,15), orientation=slider.Orientation.HORIZONTAL, 
            sensitive=True, **kwargs):
        trackNode = SliderTrack(size=size, orientation=orientation, sensitive=sensitive)
        thumbNode = ScrollBar.Thumb(size=size, orientation=orientation)

        super(ScrollBar, self).__init__(orientation=orientation, trackNode=trackNode,
                thumbNode=thumbNode, size=size, sensitive=sensitive, **kwargs)


class ScrollArea(scrollarea.ScrollArea):
    
    def __init__(self, contentNode, sensitiveScrollBars=True, parent=None, **kwargs):
        scrollPane = scrollarea.ScrollPane(contentNode)
        if sensitiveScrollBars:
            hScrollBar = ScrollBar(orientation=slider.Orientation.HORIZONTAL)
            vScrollBar = ScrollBar(orientation=slider.Orientation.VERTICAL)
        else:
            hScrollBar = ScrollBar(orientation=slider.Orientation.HORIZONTAL, size=(3,3),
                    sensitive=False)
            vScrollBar = ScrollBar(orientation=slider.Orientation.VERTICAL, size=(3,3),
                    sensitive=False)

        super(ScrollArea, self).__init__(scrollPane=scrollPane, hScrollBar=hScrollBar,
                vScrollBar=vScrollBar, **kwargs)
        self.registerInstance(self, parent)

