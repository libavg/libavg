#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import avg, ui
import libavg

RESOLUTION = avg.Point2D(800, 600)

def moveNodeToTop(node):
    parent = node.getParent()
    parent.reorderChild(node, parent.getNumChildren()-1)

class TextRect(avg.DivNode):
    def __init__(self, text, color, bgcolor, **kwargs):
        avg.DivNode.__init__(self, **kwargs)
        self.rect = avg.RectNode(size=self.size, fillopacity=1, fillcolor=bgcolor, 
                color=color, parent=self)
        self.words = avg.WordsNode(color=color, text=text, alignment="center", 
                parent=self)
        self.words.pos = (self.size-(0,self.words.size.y)) / 2

    def getSize(self):
        return self.__divSize

    def setSize(self, size):
        self.rect.size = size
        self.words.pos = (self.size-(0,self.words.size.y)) / 2
        self.__divSize = size
    __divSize = avg.DivNode.size
    size = property(getSize, setSize)


class TransformNode(TextRect):
    def __init__(self, text, ignoreScale, ignoreRotation, **kwargs):
        TextRect.__init__(self, text, "FFFFFF", "000000", **kwargs)

        self.transformer = ui.TransformRecognizer(
                node=self, 
                startHandler=self.__onStart,
                moveHandler=self.__onMove,
                upHandler=self.__onUp,
                ignoreScale=ignoreScale,
                ignoreRotation=ignoreRotation
                )

    def __onStart(self):
        self.baseTransform = ui.Mat3x3.fromNode(self)
        moveNodeToTop(self)

    def __onMove(self, transform):
        totalTransform = transform.applyMat(self.baseTransform)
        totalTransform.setNodeTransform(self)

    def __onUp(self, transform):
        pass


class DragNode(TextRect):
    def __init__(self, text, friction, **kwargs):
        TextRect.__init__(self, text, "FFFFFF", "000000", **kwargs)
    
        self.dragger = ui.DragRecognizer(
                node=self,
                startHandler=self.__onStart,
                moveHandler=self.__onMove,
                upHandler=self.__onUp,
                stopHandler=self.__onStop,
                friction=friction
                )

    def __onStart(self, event):
        self.__dragStartPos = self.pos

    def __onMove(self, event, offset):
        self.__safeMove(offset)

    def __onUp(self, event, offset):
        self.__safeMove(offset)

    def __onStop(self):
        pass

    def __safeMove(self, offset):
        self.pos = self.__dragStartPos + offset
        if self.pos.x < 0:
            self.pos = (0, self.pos.y)
        elif self.pos.x + self.size.x > RESOLUTION.x:
            self.pos = (RESOLUTION.x - self.size.x, self.pos.y)
        if self.pos.y < 0:
            self.pos = (self.pos.x, 0)
        elif self.pos.y + self.size.y > RESOLUTION.y:
            self.pos = (self.pos.x, RESOLUTION.y - self.size.y)


class GestureDemoApp(libavg.AVGApp):
    multitouch = True

    def init(self):
        TransformNode(text="TransformRecognizer",
                ignoreRotation=False,
                ignoreScale=False,
                pos=(20,20),
                size=(160,50), 
                parent=self._parentNode)

        TransformNode(text="TransformRecognizer<br/>ignoreRotation",
                ignoreRotation=True,
                ignoreScale=False,
                pos=(20,90),
                size=(160,50), 
                parent=self._parentNode)

        TransformNode(text="TransformRecognizer<br/>ignoreScale",
                ignoreRotation=False,
                ignoreScale=True,
                pos=(20,160),
                size=(160,50), 
                parent=self._parentNode)

        DragNode(text="DragRecognizer",
                pos=(300,20),
                size=(160,50),
                friction=-1,
                parent=self._parentNode)

        DragNode(text="DragRecognizer<br/>friction",
                pos=(300,90),
                size=(160,50),
                friction=0.05,
                parent=self._parentNode)


if __name__ == '__main__':
    GestureDemoApp.start(resolution=RESOLUTION)
