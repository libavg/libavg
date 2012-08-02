#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import avg, AVGApp, ui
from libavg.ui import simple

class SimpleUIApp(AVGApp):
    def init(self):
        hScrollBar = simple.ScrollBar(pos=(10,10), size=(150,20), parent=self._parentNode)
        self.__addValueDisplay(hScrollBar, (170,12))

        vScrollBar = simple.ScrollBar(pos=(10,40), size=(20,150), 
                orientation=ui.Orientation.VERTICAL, parent=self._parentNode)
        vScrollBar.sliderExtent = 2
        vScrollBar.range = (0,10)
        self.__addValueDisplay(vScrollBar, (10,200))

    def setText(self, pos, node):
        node.text = "%.2f"%pos

    def __addValueDisplay(self, scrollBar, pos):
        textNode = avg.WordsNode(pos=pos, parent=self._parentNode)
        scrollBar.subscribe(ui.ScrollBar.SLIDER_POS_CHANGED, 
                lambda pos, node=textNode: self.setText(pos, node))
        self.setText(scrollBar.sliderPos, textNode)

SimpleUIApp.start(resolution=(640,480))
