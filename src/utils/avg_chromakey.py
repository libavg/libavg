#!/usr/bin/env python
# -*- coding: utf-8 -*-

# libavg - Media Playback Engine.
# Copyright (C) 2003-2013 Ulrich von Zadow
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

from libavg import avg, app, widget
from libavg import parsecamargs

GUI_SIZE=(300, 526)


class FXSlider(avg.DivNode):
    def __init__(self, row, min, max, fxNode, fxAttrName, caption, isInt, parent=None,
            **kwargs):
        super(FXSlider, self).__init__(**kwargs)
        if parent:
            parent.appendChild(self)
        avg.RectNode(pos=(0,8), size=(280,38), color="808080", strokewidth=2, parent=self)
        textBgRect = avg.RectNode(pos=(8,2), fillcolor="000000", fillopacity=1, 
                strokewidth=0, parent=self)
        caption = avg.WordsNode(pos=(10,0), text=caption, parent=self)
        textBgRect.size = caption.getMediaSize() + (4,2)
        self.__words = avg.WordsNode(pos=(240,23), parent=self)
        self.__slider = widget.Slider(width=220, range=(min,max), pos=(15,20), parent=self)
        self.__slider.subscribe(self.__slider.THUMB_POS_CHANGED, self.__onSliderMove)
        self.pos = (0, row*46)
        self.__fxNode = fxNode
        self.__fxAttrName = fxAttrName
        self.__caption = caption
        self.__isInt = isInt
        self.__slider.thumbPos = getattr(self.__fxNode, fxAttrName)
        self.__onSliderMove(self.__slider.thumbPos)

    def __onSliderMove(self, thumbPos):
        if self.__isInt:
            setattr(self.__fxNode, self.__fxAttrName, int(thumbPos))
            self.__words.text = "%i"%thumbPos
        else:
            setattr(self.__fxNode, self.__fxAttrName, thumbPos)
            self.__words.text = "%.2f"%thumbPos


def colorToString(colorTuple):
    s = "%02X%02X%02X"%colorTuple[:-1]
    return s


class Chromakey(app.MainDiv):
    def onArgvParserCreated(self, parser):
        parser.set_usage(usage)
        parsecamargs.addOptions(parser)

    def onArgvParsed(self, options, args, parser):
        if options.driver is None:
            parser.print_help()
            print
            print "ERROR: at least '--driver' must be specified"
            exit(1)

        self.__optWidth = options.width
        self.__optHeight = options.height
        self.__optsCam = {
                "driver": options.driver,
                "device": options.device,
                "unit": options.unit,
                "fw800": options.fw800,
                "pixelformat": options.pixelFormat,
                "framerate": options.framerate}

        self.settings.set("app_resolution", "%dx%d"
                %(GUI_SIZE[0]+options.width, max(GUI_SIZE[1], options.height)))

    def onInit(self):
        avg.RectNode(size=(self.__optWidth,self.__optHeight), fillcolor="FF0000",
                fillopacity=1, strokewidth=0, parent=self)
        self.__camNode = avg.CameraNode(
                 capturewidth=self.__optWidth, captureheight=self.__optHeight,
                 width=self.__optWidth, height=self.__optHeight, parent=self,
                 **self.__optsCam)
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
        self.__guiDiv = avg.DivNode(pos=(self.__optWidth+10,10), parent=self)

        self.__colorWords = avg.WordsNode(pos=(0,14), parent=self.__guiDiv)
        self.__colorWords.text = "Key Color: "+self.__filter.color
        self.__colorRect = avg.RectNode(pos=(200,12), size=(20, 20), 
                fillcolor=self.__filter.color, fillopacity=1, 
                color="FFFFFF", parent=self.__guiDiv)
        self.__camNode.subscribe(avg.Node.CURSOR_DOWN, self.__onColorDown)

        FXSlider(1, 0.0, 1.0, self.__filter, "htolerance", "Hue Tolerance", 
                False, parent=self.__guiDiv)
        FXSlider(2, 0.0, 1.0, self.__filter, "stolerance", "Saturation Tolerance", 
                False, parent=self.__guiDiv)
        FXSlider(3, 0.0, 1.0, self.__filter, "ltolerance", "Lightness Tolerance", 
                False, parent=self.__guiDiv)
        FXSlider(4, 0.0, 1.0, self.__filter, "softness", "Softness", 
                False, parent=self.__guiDiv)
        FXSlider(5, 0, 8, self.__filter, "erosion", "Erosion", 
                True, parent=self.__guiDiv)
        FXSlider(6, 0.0, 1.0, self.__filter, "spillthreshold", "Spill Suppression", 
                False, parent=self.__guiDiv)

        button = widget.TextButton(pos=(0,332), text="Whitebalance", size=(100,22), 
                parent=self.__guiDiv)
        button.subscribe(button.CLICKED, self.__onWhitebalance)
        button = widget.TextButton(pos=(110,332), text="Dump Config", size=(100,22), 
                parent=self.__guiDiv)
        button.subscribe(button.CLICKED, self.__dumpConfig)

        FXSlider(9, 0, 500, self.__camNode, "shutter", "Shutter", 
                True, parent=self.__guiDiv)
        FXSlider(10, 128, 1023, self.__camNode, "gain", "Gain", 
                True, parent=self.__guiDiv)

    def __onColorDown(self, event):
        pos = self.__camNode.getRelPos(event.pos)
        bmp = self.__camNode.getBitmap()
        color = bmp.getPixel(pos)
        colorString = colorToString(color)
        self.__filter.color = colorString
        self.__colorWords.text = "Key Color: "+colorString
        self.__colorRect.fillcolor = colorString

    def __onWhitebalance(self):
        self.__camNode.setWhitebalance(
                self.__camNode.getWhitebalanceU(), self.__camNode.getWhitebalanceV())
        self.__camNode.doOneShotWhitebalance()

    def __dumpConfig(self):
        print "Camera:"
        print "  device=", self.__camNode.device
        print "  shutter=", self.__camNode.shutter
        print "  gain=", self.__camNode.gain
        print "  White Balance: (u=", self.__camNode.getWhitebalanceU(), ", v=", \
                self.__camNode.getWhitebalanceV()
        
        print "Chromakey:"
        print "  color=", self.__filter.color
        print "  htolerance=", self.__filter.htolerance
        print "  stolerance=", self.__filter.stolerance
        print "  ltolerance=", self.__filter.ltolerance
        print "  softness=", self.__filter.softness
        print "  erosion=", self.__filter.erosion
        print "  spillthreshold=", self.__filter.spillthreshold


usage = """%prog [options]

avg_chromakey.py is a configuration utility for the libavg chromakey filter.
The chromakey filter allows implementation of green- or bluescreens with
libavg.

This utility shows a camera image with chromakey applied to it and allows the
user to adjust the camera and filter parameters. The parameters can be dumped
to the console for easy inclusion in libavg scripts.
"""


if __name__ == "__main__":
    app.App().run(Chromakey())

