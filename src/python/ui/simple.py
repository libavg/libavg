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
from libavg.ui import button, slider

class Slider(avg.DivNode):
    def __init__(self, width, min, max, onChange, parent=None, **kwargs):
        super(Slider, self).__init__(**kwargs)
        self.registerInstance(self, parent)
        
        self.__onChange = onChange
        self.size = (width, 20)
        self.__min = min
        self.__max = max
        self.__val = min
        avg.LineNode(pos1=(7,14), pos2=(width-7,14), color="FFFFFF", strokewidth=2, 
                parent=self)
        self.__slider = avg.DivNode(pos=(0,0), size=(14,20), parent=self)
        avg.PolygonNode(pos=((1,0), (13,0), (7,18)), fillopacity=1, fillcolor="FFFFFF",
                color="808080", parent=self.__slider)
        self.__slider.setEventHandler(avg.CURSORDOWN, avg.MOUSE, 
                self.__onSliderDown)
        self.setEventHandler(avg.CURSORDOWN, avg.MOUSE, self.__onBarDown)
        self.__isDragging = False

    def getVal(self):
        return self.__val

    def setVal(self, val):
        self.__val = val
        self.__positionSlider()

    val = property(getVal, setVal)

    def __onSliderDown(self, event):
        self.__slider.setEventCapture()
        self.__slider.setEventHandler(avg.CURSORMOTION, avg.MOUSE, self.__onSliderMove)
        self.__slider.setEventHandler(avg.CURSORUP, avg.MOUSE, self.__onSliderUp)
        self.__sliderDownPos = event.pos
        self.__isDragging = True
        self.__dragStartVal = self.__val

    def __onSliderMove(self, event):
        numPixelsMoved = float(event.pos.x-self.__sliderDownPos.x)
        self.__val = (self.__dragStartVal+numPixelsMoved/(self.size.x-14)
                *(self.__max-self.__min))
        self.__positionSlider()

    def __onSliderUp(self, event):
        self.__onSliderMove(event)
        self.__slider.releaseEventCapture()
        self.__slider.setEventHandler(avg.CURSORMOTION, avg.MOUSE, None)
        self.__slider.setEventHandler(avg.CURSORUP, avg.MOUSE, None)
        self.__isDragging = False

    def __onBarDown(self, event):
        if not(self.__isDragging):
            localPos = self.getRelPos(event.pos)
            ratio = (localPos.x-7)/(self.size.x-14)
            self.__val = self.__min+ratio*(self.__max-self.__min)
            print localPos, ", ", ratio, ", ", self.__val
            self.__positionSlider()

    def __positionSlider(self):
        if self.__val < self.__min:
            self.__val = self.__min
        elif self.__val > self.__max:
            self.__val = self.__max
        ratio = (float(self.__val-self.__min)/(self.__max-self.__min))
        self.__slider.pos = (ratio*(self.size.x-14), 0)
        self.__onChange()


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
    def __init__(self, size, orientation, **kwargs):
        self.__orientation = orientation
        style = avg.Style(pos=(0.5,0.5), size=size, fillopacity=1)
        self.__enabledNode = avg.RectNode(fillcolor="000000", color="FFFFFF",
                style=style)
        self.__disabledNode = avg.RectNode(fillcolor="404040", color="C0C0C0",
                style=style)
        nodeMap = {
            "ENABLED": self.__enabledNode,
            "DISABLED": self.__disabledNode
        }
        super(SliderTrack, self).__init__(nodeMap=nodeMap, visibleID="ENABLED", size=size,
                **kwargs)

    def getExtent(self):
        if self.__orientation == slider.Orientation.HORIZONTAL:
            return self.__enabledNode.width
        else:
            return self.__enabledNode.height

    def setExtent(self, extent):
        if self.__orientation == slider.Orientation.HORIZONTAL:
            self.size = (extent, self.size.y)
        else:
            self.size = (self.size.x, extent)
        for node in self.__enabledNode, self.__disabledNode:
            node.size = self.size

    extent = property(getExtent, setExtent)


class Slider(slider.Slider):
    
    class Thumb(button.SwitchNode):

        def __init__(self, orientation, **kwargs):
            if orientation == slider.Orientation.HORIZONTAL:
                pos=((1,0), (13,0), (7,18))
            else:
                pos=((18,1), (18,13), (1,7))
            
            style = avg.Style(pos=pos, fillopacity=1)
            self.__upNode = avg.PolygonNode(fillcolor="808080", color="FFFFFF",
                    style=style)
            self.__downNode = avg.PolygonNode(fillcolor="C0C0C0", color="FFFFFF",
                    style=style)
            self.__disabledNode = avg.PolygonNode(fillcolor="404040", color="C0C0C0",
                    style=style)
            nodeMap = {
                "UP": self.__upNode,
                "DOWN": self.__downNode,
                "DISABLED": self.__disabledNode
            }
            super(Slider.Thumb, self).__init__(
                    nodeMap=nodeMap, visibleID="UP", size=(14,20))

    def __init__(self, orientation=slider.Orientation.HORIZONTAL, **kwargs):
        
        size = avg.Point2D(kwargs["size"])
        if orientation == slider.Orientation.HORIZONTAL:
            trackPos = (7, 10)
            trackSize = (size.x, 4) 
        else:
            trackPos = (6, 7)
            trackSize = (4, size.y) 
        trackNode = SliderTrack(pos=trackPos, size=trackSize, orientation=orientation)
        thumbNode = Slider.Thumb(orientation)

        super(Slider, self).__init__(orientation=orientation, trackNode=trackNode,
                thumbNode=thumbNode, **kwargs)


class ScrollBar(slider.ScrollBar):

    class Thumb(button.SwitchNode):
        def __init__(self, size, orientation, **kwargs):
            self.__orientation = orientation
            childSize = avg.Point2D(size) - (2,2)
            style = avg.Style(pos=(1.5,1.5), size=childSize, fillopacity=1)
            self.__upNode = avg.RectNode(fillcolor="808080", color="808080", style=style)
            self.__downNode = avg.RectNode(fillcolor="C0C0C0", color="C0C0C0", 
                    style=style)
            self.__disabledNode = avg.RectNode(fillcolor="404040", color="C0C0C0",
                    style=style)
            nodeMap = {
                "UP": self.__upNode,
                "DOWN": self.__downNode,
                "DISABLED": self.__disabledNode
            }
            super(ScrollBar.Thumb, self).__init__(
                    nodeMap=nodeMap, visibleID="UP", size=size)

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


    def __init__(self, orientation=slider.Orientation.HORIZONTAL, **kwargs):
        trackNode = SliderTrack(kwargs["size"], orientation)
        thumbNode = ScrollBar.Thumb(kwargs["size"], orientation)

        super(ScrollBar, self).__init__(orientation=orientation, trackNode=trackNode,
                thumbNode=thumbNode, **kwargs)

