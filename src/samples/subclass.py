#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import avg, gesture
import libavg

RESOLUTION = avg.Point2D(800, 600)

class TextRect(avg.DivNode):
    def __init__(self, text, parent=None, **kwargs):
        super(TextRect, self).__init__(**kwargs)
        self.registerInstance(self, parent)
        self.rect = avg.RectNode(size=self.size, fillopacity=1, fillcolor="000000", 
                color="FFFFFF", parent=self)
        self.words = avg.WordsNode(color="FFFFFF", text=text, alignment="center", 
                parent=self)
        self.words.pos = (self.size-(0,self.words.size.y)) / 2

    def getSize(self):
        return self.__divSize

    def setSize(self, size):
        self.rect.size = size
        self.words.pos = (size-(0,self.words.size.y)) / 2
        self.__divSize = size
    __divSize = avg.DivNode.size
    size = property(getSize, setSize)


class SubclassDemoApp(libavg.AVGApp):

    def init(self):
        self.rect = TextRect(text="Hello World", pos=(20,20), size=(200,120), 
                parent=self._parentNode)
        gesture.TapRecognizer(node=self.rect, detectedHandler=self.onTap)
        
    def onTap(self, event):
        self.rect.size = self.rect.size + (10,10)

if __name__ == '__main__':
    SubclassDemoApp.start(resolution=RESOLUTION)
