#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import avg, ui
import libavg

RESOLUTION = avg.Point2D(800, 600)

def moveNodeToTop(node):
    parent = node.getParent()
    parent.reorderChild(node, parent.getNumChildren()-1)

def moveNodeOnScreen(node):
    center = node.pos + node.size/2
    if center.x < 0:
        node.pos = (-node.size.x/2, node.pos.y)
    if center.x > RESOLUTION.x:
        node.pos = (RESOLUTION.x-node.size.x/2, node.pos.y)
    if center.y < 0:
        node.pos = (node.pos.x, -node.size.y/2)
    if center.y > RESOLUTION.y:
        node.pos = (node.pos.x, RESOLUTION.y-node.size.y/2)


class TextRect(avg.DivNode):
    def __init__(self, text, **kwargs):
        avg.DivNode.__init__(self, size=(150,40), **kwargs)
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


class TransformNode(TextRect):
    def __init__(self, text, ignoreScale, ignoreRotation, friction=-1, **kwargs):
        TextRect.__init__(self, text, **kwargs)
        self.__ignoreScale = ignoreScale
        self.__ignoreRotation = ignoreRotation

        ui.TransformRecognizer(
                eventNode=self, 
                startHandler=self.__onStart,
                moveHandler=self.__onMove,
                upHandler=self.__onUp,
                friction=friction
                )

    def __onStart(self):
        moveNodeToTop(self)

    def __onMove(self, transform):
        if self.__ignoreScale:
            transform.scale = 1
        if self.__ignoreRotation:
            transform.rot = 0
        transform.moveNode(self)
        moveNodeOnScreen(self)

    def __onUp(self, transform):
        pass


class TransformChildNode(avg.DivNode):
    def __init__(self, text, **kwargs):
        avg.DivNode.__init__(self, **kwargs)
        self.textRect = TextRect(text, parent=self)
        self.size = self.textRect.size

        self.inputNode = avg.RectNode(size=(self.size.x, self.size.y/2), 
                fillopacity=0.5, fillcolor="808080", strokewidth=0, parent=self)
        ui.TransformRecognizer(
                eventNode=self.inputNode,
                coordSysNode=self,
                startHandler=self.__onStart,
                moveHandler=self.__onMove,
                friction=0.05
                )
   
    def __onStart(self):
        self.baseTransform = ui.Mat3x3.fromNode(self)
        moveNodeToTop(self)

    def __onMove(self, transform):
        transform.moveNode(self)
        moveNodeOnScreen(self)
        self.textRect.size = self.size
        self.inputNode.size = (self.size.x, self.size.y/2)


class DragNode(TextRect):
    def __init__(self, text, friction=-1, **kwargs):
        TextRect.__init__(self, text, **kwargs)
    
        ui.DragRecognizer(
                eventNode=self,
                startHandler=self.__onStart,
                moveHandler=self.__onMove,
                upHandler=self.__onMove,
                stopHandler=self.__onStop,
                friction=friction
                )

    def __onStart(self, event):
        self.__dragStartPos = self.pos
        moveNodeToTop(self)

    def __onMove(self, event, offset):
        self.pos = self.__dragStartPos + offset
        moveNodeOnScreen(self)

    def __onStop(self):
        pass


class TapNode(TextRect):
    def __init__(self, text, isDoubleTap, **kwargs):
        TextRect.__init__(self, text, **kwargs)
    
        if isDoubleTap:
            ui.DoubletapRecognizer(node=self, startHandler=self.__onStart,
                    tapHandler=self.__onTap, failHandler=self.__onFail)
        else:
            ui.TapRecognizer(node=self, startHandler=self.__onStart,
                    tapHandler=self.__onTap, failHandler=self.__onFail)

    def __onStart(self):
        self.rect.fillcolor = "FFFFFF"
        self.words.color = "000000"

    def __onTap(self):
        self.rect.fillcolor = "000000"
        self.words.color = "FFFFFF"
        self.rect.color = "00FF00"

    def __onFail(self):
        self.rect.fillcolor = "000000"
        self.words.color = "FFFFFF"
        self.rect.color = "FF0000"


class GestureDemoApp(libavg.AVGApp):
    multitouch = True

    def init(self):
        TransformNode(text="TransformRecognizer",
                ignoreRotation=False,
                ignoreScale=False,
                pos=(20,20),
                parent=self._parentNode)

        TransformNode(text="TransformRecognizer<br/>ignoreRotation",
                ignoreRotation=True,
                ignoreScale=False,
                pos=(20,70),
                parent=self._parentNode)

        TransformNode(text="TransformRecognizer<br/>ignoreScale",
                ignoreRotation=False,
                ignoreScale=True,
                pos=(20,120),
                parent=self._parentNode)

        TransformNode(text="TransformRecognizer<br/>friction",
                ignoreRotation=False,
                ignoreScale=False,
                pos=(20,170),
                friction=0.02,
                parent=self._parentNode)

        TransformChildNode(text="TransformRecognizer<br/>child dragger",
                pos=(20,220),
                parent=self._parentNode)

        DragNode(text="DragRecognizer",
                pos=(200,20),
                parent=self._parentNode)

        DragNode(text="DragRecognizer<br/>friction",
                pos=(200,70),
                friction=0.05,
                parent=self._parentNode)

        TapNode(text="TapRecognizer",
                pos=(380,20),
                isDoubleTap=False,
                parent=self._parentNode)

        TapNode(text="DoubletapRecognizer",
                pos=(380,70),
                isDoubleTap=True,
                parent=self._parentNode)


if __name__ == '__main__':
    GestureDemoApp.start(resolution=RESOLUTION)
