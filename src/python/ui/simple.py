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
from libavg.ui import button

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

