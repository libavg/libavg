#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import avg, gesture
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
    def __init__(self, text, parent=None, **kwargs):
        super(TextRect, self).__init__(size=(150,40), **kwargs)
        if parent:
            parent.appendChild(self)

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
        super(TransformNode, self).__init__(text, **kwargs)
        self.__ignoreScale = ignoreScale
        self.__ignoreRotation = ignoreRotation

        self.recognizer = gesture.TransformRecognizer(
                eventNode=self, 
                detectedHandler=self.__onDetected,
                moveHandler=self.__onMove,
                upHandler=self.__onUp,
                friction=friction
                )

    def __onDetected(self):
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
    def __init__(self, text, parent=None, **kwargs):
        super(TransformChildNode, self).__init__( **kwargs)
        if parent:
            parent.appendChild(self)

        self.textRect = TextRect(text, parent=self)
        self.size = self.textRect.size

        self.inputNode = avg.RectNode(size=(self.size.x, self.size.y/2), 
                fillopacity=0.5, fillcolor="808080", strokewidth=0, parent=self)
        self.recognizer = gesture.TransformRecognizer(
                eventNode=self.inputNode,
                coordSysNode=self,
                detectedHandler=self.__onDetected,
                moveHandler=self.__onMove,
                friction=0.05
                )

    def __onDetected(self):
        moveNodeToTop(self)

    def __onMove(self, transform):
        transform.moveNode(self)
        moveNodeOnScreen(self)
        self.textRect.size = self.size
        self.inputNode.size = (self.size.x, self.size.y/2)


class DragNode(TextRect):
    def __init__(self, text, friction=-1, **kwargs):
        super(DragNode, self).__init__(text, **kwargs)

        self.recognizer = gesture.DragRecognizer(
                eventNode=self,
                detectedHandler=self.__onDetected,
                moveHandler=self.__onMove,
                upHandler=self.__onMove,
                endHandler=self.__onEnd,
                friction=friction
                )

    def __onDetected(self):
        self.__dragStartPos = self.pos
        moveNodeToTop(self)

    def __onMove(self, offset):
        self.pos = self.__dragStartPos + offset
        moveNodeOnScreen(self)

    def __onEnd(self):
        pass


class ConstrainedDragNode(TextRect):
    def __init__(self, text, friction=-1, **kwargs):
        super(ConstrainedDragNode, self).__init__(text, **kwargs)

        self.recognizer = gesture.DragRecognizer(
                eventNode=self,
                detectedHandler=self.__onDetected,
                moveHandler=self.__onHorizMove,
                upHandler=self.__onHorizMove,
                direction=gesture.DragRecognizer.HORIZONTAL,
                friction=0.05
                )

        self.recognizer2 = gesture.DragRecognizer(
                eventNode=self,
                detectedHandler=self.__onDetected,
                moveHandler=self.__onVertMove,
                upHandler=self.__onVertMove,
                direction=gesture.DragRecognizer.VERTICAL,
                friction=0.05
                )

    def __onDetected(self):
        self.__dragStartPos = self.pos
        moveNodeToTop(self)

    def __onHorizMove(self, offset):
        self.pos = self.__dragStartPos + (offset.x, 0)
        moveNodeOnScreen(self)

    def __onVertMove(self, offset):
        self.pos = self.__dragStartPos + (0, offset.y)
        moveNodeOnScreen(self)


class TapNode(TextRect):
    def __init__(self, text, isDoubleTap, **kwargs):
        super(TapNode, self).__init__(text, **kwargs)

        if isDoubleTap:
            self.recognizer = gesture.DoubletapRecognizer(node=self, 
                    possibleHandler=self.__onPossible, detectedHandler=self.__onDetected,
                    failHandler=self.__onFail)
        else:
            self.recognizer = gesture.TapRecognizer(node=self, 
                    possibleHandler=self.__onPossible, detectedHandler=self.__onDetected,
                    failHandler=self.__onFail)

    def __onPossible(self):
        self.rect.fillcolor = "FFFFFF"
        self.words.color = "000000"

    def __onDetected(self):
        self.rect.fillcolor = "000000"
        self.words.color = "FFFFFF"
        self.rect.color = "00FF00"

    def __onFail(self):
        self.rect.fillcolor = "000000"
        self.words.color = "FFFFFF"
        self.rect.color = "FF0000"


class SwipeNode(TextRect):
    def __init__(self, text, numContacts, **kwargs):
        super(SwipeNode, self).__init__(text, **kwargs)

        self.recognizer = gesture.SwipeRecognizer(node=self, minDist=25, 
                numContacts=numContacts, direction=gesture.SwipeRecognizer.RIGHT,
                possibleHandler=self.__onPossible, detectedHandler=self.__onDetected, 
                failHandler=self.__onFail)

    def __onPossible(self):
        self.rect.fillcolor = "FFFFFF"
        self.words.color = "000000"

    def __onDetected(self):
        self.rect.fillcolor = "000000"
        self.words.color = "FFFFFF"
        self.rect.color = "00FF00"

    def __onFail(self):
        self.rect.fillcolor = "000000"
        self.words.color = "FFFFFF"
        self.rect.color = "FF0000"


class HoldNode(TextRect):
    def __init__(self, text, **kwargs):
        super(HoldNode, self).__init__(text, **kwargs)

        self.recognizer = gesture.HoldRecognizer(node=self, 
                possibleHandler=self.__onPossible, detectedHandler=self.__onDetected, 
                failHandler=self.__onFail, stopHandler=self.__onStop)

    def __onPossible(self):
        self.rect.fillcolor = "FFFFFF"
        self.rect.color = "FFFFFF"
        self.words.color = "000000"

    def __onDetected(self):
        self.rect.fillcolor = "000000"
        self.words.color = "FFFFFF"
        self.rect.color = "00FF00"

    def __onFail(self):
        self.rect.fillcolor = "000000"
        self.words.color = "FFFFFF"
        self.rect.color = "FF0000"

    def __onStop(self):
        self.rect.fillcolor = "000000"
        self.rect.color = "FFFFFF"
        self.words.color = "FFFFFF"


class GestureDemoApp(libavg.AVGApp):
    multitouch = True

    def init(self):
        TransformNode(text="TransformRecognizer",
                ignoreRotation=False, ignoreScale=False,
                pos=(20,20), parent=self._parentNode)

        TransformNode(text="TransformRecognizer<br/>ignoreRotation",
                ignoreRotation=True, ignoreScale=False,
                pos=(20,70), parent=self._parentNode)

        TransformNode(text="TransformRecognizer<br/>ignoreScale",
                ignoreRotation=False, ignoreScale=True,
                pos=(20,120), parent=self._parentNode)

        TransformNode(text="TransformRecognizer<br/>friction",
                ignoreRotation=False, ignoreScale=False,
                pos=(20,170), friction=0.02, parent=self._parentNode)

        TransformChildNode(text="TransformRecognizer<br/>child dragger",
                pos=(20,220), parent=self._parentNode)

        DragNode(text="DragRecognizer", pos=(200,20), parent=self._parentNode)

        DragNode(text="DragRecognizer<br/>friction", pos=(200,70), friction=0.01,
                parent=self._parentNode)

        ConstrainedDragNode(text="DragRecognizer<br/>constrained", pos=(200,120), 
                friction=0.01, parent=self._parentNode)

        TapNode(text="TapRecognizer", pos=(380,20), isDoubleTap=False,
                parent=self._parentNode)

        TapNode(text="DoubletapRecognizer", pos=(380,70), isDoubleTap=True,
                parent=self._parentNode)

        HoldNode(text="HoldRecognizer", pos=(380,120), parent=self._parentNode)

        SwipeNode(text="SwipeRecognizer<br/>(Right)", pos=(380,170), 
                numContacts=1, parent=self._parentNode)

        SwipeNode(text="SwipeRecognizer<br/>(Right, 2 fingers)", pos=(380,220), 
                numContacts=2, parent=self._parentNode)


if __name__ == '__main__':
    GestureDemoApp.start(resolution=RESOLUTION)
