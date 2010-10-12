#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
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

import optparse
from libavg import avg, AVGApp
from libavg import parsecamargs

GUI_SIZE=(640, 200)

g_Player = avg.Player.get()

class Slider(avg.DivNode):
    def __init__(self, width, min, max, onChange, **kwargs):
        avg.DivNode.__init__(self, **kwargs)
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
        self.__isDragging = True
        self.__dragStartVal = self.__val

    def __onSliderMove(self, event):
        numPixelsMoved = float(event.pos.x-event.lastdownpos.x)
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
        ratio = (self.__val/(self.__max-self.__min))
        self.__slider.pos = (ratio*(self.size.x-14), 0)
        self.__onChange()


class FXSlider(avg.DivNode):
    def __init__(self, row, min, max, fxNode, fxAttrName, caption, isInt, **kwargs):
        avg.DivNode.__init__(self, **kwargs)
        self.__slider = Slider(420, min, max, self.__onSliderMove, pos=(200,0),
                parent=self)
        self.pos = (0, row*24)
        self.__words = avg.WordsNode(pos=(0,4), parent=self)
        self.__fxNode = fxNode
        self.__fxAttrName = fxAttrName
        self.__caption = caption
        self.__isInt = isInt
        self.__slider.val = getattr(self.__fxNode, fxAttrName)

    def __onSliderMove(self):
        if self.__isInt:
            setattr(self.__fxNode, self.__fxAttrName, int(self.__slider.val))
        else:
            setattr(self.__fxNode, self.__fxAttrName, self.__slider.val)
        self.__words.text = self.__caption%self.__slider.val

def colorToString(colorTuple):
    s = "%02X%02X%02X"%colorTuple[:-1]
    return s

class Chromakey(AVGApp):
    def init(self):
        self.__camNode = avg.CameraNode(driver=options.driver, device=options.device, 
                 unit=options.unit, fw800=options.fw800, 
                 capturewidth=options.width, captureheight=options.height, 
                 pixelformat=options.pixelFormat, framerate=options.framerate, 
                 width=options.width, height=options.height, parent=self._parentNode)
        self.__camNode.play()
        self.__filter = avg.ChromaKeyFXNode()
        self.__camNode.setEffect(self.__filter)
        self.__filter.color = "0000FF"
        self.__filter.htolerance = 0.05
        self.__filter.stolerance = 1.0 
        self.__filter.ltolerance = 1.0
        self.__filter.softness = 0.0

        self.__createGUI()

    def __createGUI(self):
        self.__guiDiv = avg.DivNode(pos=(10,options.height+10), parent=self._parentNode)

        self.__colorWords = avg.WordsNode(pos=(0,0), parent=self.__guiDiv)
        self.__colorRect = avg.RectNode(pos=(200,0), size=(20, 20), 
                fillcolor=self.__filter.color, fillopacity=1, 
                color="FFFFFF", parent=self.__guiDiv)
        self.__camNode.setEventHandler(avg.CURSORDOWN, avg.MOUSE, 
                self.__onColorDown)

        FXSlider(1, 0.0, 1.0, self.__filter, "htolerance", "Hue Tolerance: %.2f", 
                False, parent=self.__guiDiv)
        FXSlider(2, 0.0, 1.0, self.__filter, "stolerance", "Saturation Tolerance: %.2f", 
                False, parent=self.__guiDiv)
        FXSlider(3, 0.0, 1.0, self.__filter, "ltolerance", "Lightness Tolerance: %.2f", 
                False, parent=self.__guiDiv)
        FXSlider(4, 0.0, 1.0, self.__filter, "softness", "Softness: %.2f", 
                False, parent=self.__guiDiv)
        FXSlider(5, 0, 8, self.__filter, "erosion", "Erosion: %i", 
                True, parent=self.__guiDiv)
        FXSlider(6, 0.0, 1.0, self.__filter, "spillthreshold", "Spill Suppression: %.2f", 
                False, parent=self.__guiDiv)

    def __onColorDown(self, event):
        pos = self.__camNode.getRelPos(event.pos)
        bmp = self.__camNode.getBitmap()
        color = bmp.getPixel(pos)
        colorString = colorToString(color)
        self.__filter.color = colorString
        self.__colorWords.text = "Key Color: "+colorString
        self.__colorRect.fillcolor = colorString


parser = optparse.OptionParser()
parsecamargs.addOptions(parser)

(options, args) = parser.parse_args()
if options.driver is None:
    parser.print_help()
    print
    print "ERROR: at least '--driver' must be specified"
    exit()

resolution=(max(GUI_SIZE[0], options.width), GUI_SIZE[1]+options.height)

Chromakey.start(resolution=resolution)

