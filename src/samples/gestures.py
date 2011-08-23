#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import avg, ui
import libavg

class TextRect(avg.DivNode):
    def __init__(self, text, color, bgcolor, **kwargs):
        avg.DivNode.__init__(self, **kwargs)
        self.rect = avg.RectNode(size=self.size, fillopacity=1, fillcolor=bgcolor, 
                color=color, parent=self)
        self.words = avg.WordsNode(pos=self.size/2-(0,9), color=color, text=text, 
                alignment="center", parent=self)

    def getSize(self):
        return self.divSize

    def setSize(self, size):
        self.rect.size = size
        self.words.pos = size/2 - (0,9)
        self.divSize = size
    divSize = avg.DivNode.size
    size = property(getSize, setSize)


class TransformNode(TextRect):
    def __init__(self, text, **kwargs):
        TextRect.__init__(self, text, "FFFFFF", "000000", **kwargs)

        self.transformer = ui.TransformRecognizer(
                node=self, 
                startHandler=self.__onStart,
                moveHandler=self.__onMove,
                upHandler=self.__onUp)

    def __onStart(self):
        self.baseTransform = ui.Mat3x3.fromNode(self)

    def __onMove(self, transform):
        totalTransform = transform.applyMat(self.baseTransform)
        totalTransform.setNodeTransform(self)

    def __onUp(self, transform):
        pass


class GestureDemoApp(libavg.AVGApp):
    multitouch = True

    def init(self):
        self.transformNode = TransformNode(text="TransformRecognizer", pos=(20,20),
                size=(160,50), parent=self._parentNode)


if __name__ == '__main__':
    GestureDemoApp.start(resolution=(800,600))
