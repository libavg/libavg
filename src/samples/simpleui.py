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
        vScrollBar.range = (10,0)
        self.__addValueDisplay(vScrollBar, (10,220))

        hSlider = simple.Slider(pos=(10,35), size=(150,20), parent=self._parentNode)
        self.__addValueDisplay(hSlider, (175,38))

        vSlider = simple.Slider(pos=(60.5,60), size=(20,150), 
                orientation=ui.Orientation.VERTICAL, parent=self._parentNode)
        vSlider.range = (1,0)
        self.__addValueDisplay(vSlider, (55,220))
        self.controls = [hScrollBar, vScrollBar, hSlider, vSlider]

        self.createScrollArea(avg.Point2D(220,10), True)
        self.createScrollArea(avg.Point2D(500,10), False)

        checkBox = simple.CheckBox(pos=(10,270), text="Disable everything", 
                parent=self._parentNode)
        checkBox.subscribe(simple.CheckBox.TOGGLED, self.onCheck)

    def setText(self, pos, node):
        node.text = "%.2f"%pos

    def setImageWidth(self, scrollArea, thumbPos):
        scrollArea.contentsize = (thumbPos, scrollArea.contentsize.y)
    
    def setImageHeight(self, scrollArea, thumbPos):
        scrollArea.contentsize = (scrollArea.contentsize.x, thumbPos)

    def createScrollArea(self, pos, sensitiveScrollBars):
        image = avg.ImageNode(href="rgb24-64x64.png", size=(1024, 1024))
        scrollArea = simple.ScrollArea(contentNode=image, parent=self._parentNode,
                pos=pos, size=(220,220), sensitiveScrollBars=sensitiveScrollBars)

        imageWidthSlider = simple.Slider(pos=pos+(0,230), size=(220,20), 
                parent=self._parentNode)
        imageWidthSlider.range = (100,1024)
        imageWidthSlider.thumbpos = 1024
        imageWidthSlider.subscribe(ui.ScrollBar.THUMB_POS_CHANGED, 
                lambda thumbPos, scrollArea=scrollArea: 
                    self.setImageWidth(scrollArea, thumbPos))

        imageHeightSlider = simple.Slider(pos=pos+(230,0), size=(20,220), 
                orientation=ui.Orientation.VERTICAL, parent=self._parentNode)
        imageHeightSlider.range = (100,1024)
        imageHeightSlider.thumbpos = 1024
        imageHeightSlider.subscribe(ui.ScrollBar.THUMB_POS_CHANGED,
                lambda thumbPos, scrollArea=scrollArea: 
                    self.setImageHeight(scrollArea, thumbPos))
        self.controls.extend([scrollArea, imageWidthSlider, imageHeightSlider])

    def onCheck(self, isChecked):
        for node in self.controls:
            node.enabled = not(isChecked)

    def __addValueDisplay(self, scrollBar, pos):
        textNode = avg.WordsNode(pos=pos, parent=self._parentNode)
        scrollBar.subscribe(ui.ScrollBar.THUMB_POS_CHANGED, 
                lambda pos, node=textNode: self.setText(pos, node))
        self.setText(scrollBar.thumbpos, textNode)


SimpleUIApp.start(resolution=(1024, 768))
