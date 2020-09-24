#!/usr/bin/env python
# -*- coding: utf-8 -*-

# libavg - Media Playback Engine.
# Copyright (C) 2012-2020 Ulrich von Zadow
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

class SimpleUI(app.MainDiv):
    def onInit(self):
        avg.RectNode(size=(1024,768), fillopacity=1, fillcolor="FFFFFF",
                parent=self)

        hScrollBar = widget.ScrollBar(pos=(10,10), width=150, parent=self)
        self.__addValueDisplay(hScrollBar, (175,12))

        vScrollBar = widget.ScrollBar(pos=(15,60), height=150, 
                orientation=widget.Orientation.VERTICAL, parent=self)
        vScrollBar.thumbExtent = 5
        vScrollBar.range = (10,0)
        self.__addValueDisplay(vScrollBar, (10,220))

        hSlider = widget.Slider(pos=(10,35), width=150, parent=self)
        self.__addValueDisplay(hSlider, (175,33))

        vSlider = widget.Slider(pos=(60.5,60), height=150, 
                orientation=widget.Orientation.VERTICAL, parent=self)
        vSlider.range = (1,0)
        self.__addValueDisplay(vSlider, (55,220))
        self.controls = [hScrollBar, vScrollBar, hSlider, vSlider]

        self.createScrollArea(avg.Point2D(220,10))

        checkBox = widget.CheckBox(pos=(10,270), text="Disable everything", 
                parent=self)
        checkBox.subscribe(widget.CheckBox.TOGGLED, self.onCheck)

    def setText(self, pos, node):
        node.text = "%.2f"%pos

    def setImageWidth(self, scrollArea, thumbPos):
        scrollArea.contentsize = (thumbPos, scrollArea.contentsize.y)
    
    def setImageHeight(self, scrollArea, thumbPos):
        scrollArea.contentsize = (scrollArea.contentsize.x, thumbPos)

    def createScrollArea(self, pos):
        image = avg.ImageNode(href="rgb24-64x64.png", size=(1024, 1024))
        scrollArea = widget.ScrollArea(contentNode=image, parent=self,
                pos=pos, size=(220,220))

        imageWidthSlider = widget.Slider(pos=pos+(0,230), width=220, 
                parent=self)
        imageWidthSlider.range = (100,1024)
        imageWidthSlider.thumbPos = 1024
        imageWidthSlider.subscribe(widget.ScrollBar.THUMB_POS_CHANGED, 
                lambda thumbPos, scrollArea=scrollArea: 
                    self.setImageWidth(scrollArea, thumbPos))

        imageHeightSlider = widget.Slider(pos=pos+(230,0), height=220,
                orientation=widget.Orientation.VERTICAL, parent=self)
        imageHeightSlider.range = (100,1024)
        imageHeightSlider.thumbPos = 1024
        imageHeightSlider.subscribe(widget.ScrollBar.THUMB_POS_CHANGED,
                lambda thumbPos, scrollArea=scrollArea: 
                    self.setImageHeight(scrollArea, thumbPos))
        self.controls.extend([scrollArea, imageWidthSlider, imageHeightSlider])

    def onCheck(self, isChecked):
        for node in self.controls:
            node.enabled = not(isChecked)

    def __addValueDisplay(self, scrollBar, pos):
        textNode = avg.WordsNode(pos=pos, color="000000", parent=self)
        scrollBar.subscribe(widget.ScrollBar.THUMB_POS_CHANGED, 
                lambda pos, node=textNode: self.setText(pos, node))
        self.setText(scrollBar.thumbPos, textNode)


app.App().run(SimpleUI(), app_resolution='1024x768')
