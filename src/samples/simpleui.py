#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import avg, AVGApp, ui
from libavg.ui import simple

class SimpleUIApp(AVGApp):
    def init(self):
        hScrollBar = simple.ScrollBar(pos=(10,10), size=(150,20), parent=self._parentNode)
        self.__addValueDisplay(hScrollBar, (175,12))

        vScrollBar = simple.ScrollBar(pos=(15,60), size=(20,150), 
                orientation=ui.Orientation.VERTICAL, parent=self._parentNode)
        vScrollBar.thumbextent = 5
        vScrollBar.range = (0,10)
        self.__addValueDisplay(vScrollBar, (10,220))

        hSlider = simple.Slider(pos=(10,35), size=(150,20), parent=self._parentNode)
        self.__addValueDisplay(hSlider, (175,38))

        vSlider = simple.Slider(pos=(60.5,60), size=(20,150), 
                orientation=ui.Orientation.VERTICAL, parent=self._parentNode)
        self.__addValueDisplay(vSlider, (55,220))

        image = avg.ImageNode(href="rgb24-64x64.png", size=(1024, 1024))
        self.scrollArea = simple.ScrollArea(contentNode=image, parent=self._parentNode,
                pos=(220,10), size=(220,220))

        imageWidthSlider = simple.Slider(pos=(220,240), size=(220,20), 
                parent=self._parentNode)
        imageWidthSlider.range = (100,1024)
        imageWidthSlider.thumbpos = 1024
        imageWidthSlider.subscribe(ui.ScrollBar.THUMB_POS_CHANGED, self.setImageWidth)

        imageHeightSlider = simple.Slider(pos=(450,10), size=(20,220), 
                orientation=ui.Orientation.VERTICAL, parent=self._parentNode)
        imageHeightSlider.range = (100,1024)
        imageHeightSlider.thumbpos = 1024
        imageHeightSlider.subscribe(ui.ScrollBar.THUMB_POS_CHANGED, self.setImageHeight)

    def setText(self, pos, node):
        node.text = "%.2f"%pos

    def setImageWidth(self, thumbPos):
        self.scrollArea.contentsize = (thumbPos, self.scrollArea.contentsize.y)
    
    def setImageHeight(self, thumbPos):
        self.scrollArea.contentsize = (self.scrollArea.contentsize.x, thumbPos)
    
    def __addValueDisplay(self, scrollBar, pos):
        textNode = avg.WordsNode(pos=pos, parent=self._parentNode)
        scrollBar.subscribe(ui.ScrollBar.THUMB_POS_CHANGED, 
                lambda pos, node=textNode: self.setText(pos, node))
        self.setText(scrollBar.thumbpos, textNode)


SimpleUIApp.start(resolution=(1024, 768))
