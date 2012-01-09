#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import avg, ui
import libavg

RESOLUTION = avg.Point2D(800, 600)

nodeList = []
nodesEnabled = True

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

def abortAll():
    for node in nodeList:
        node.recognizer.abort()

def switchNodesEnabled():
    global nodesEnabled
    nodesEnabled = not nodesEnabled
    for node in nodeList:
        node.recognizer.enable(nodesEnabled)


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

        self.recognizer = ui.TransformRecognizer(
                eventNode=self, 
                detectedHandler=self.__onDetected,
                moveHandler=self.__onMove,
                upHandler=self.__onUp,
                friction=friction
                )

    def __onDetected(self, event):
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


class DragNode(TextRect):
    def __init__(self, text, friction=-1, **kwargs):
        TextRect.__init__(self, text, **kwargs)

        self.recognizer = ui.DragRecognizer(
                eventNode=self,
                detectedHandler=self.__onDetected,
                moveHandler=self.__onMove,
                upHandler=self.__onMove,
                endHandler=self.__onEnd,
                friction=friction
                )

    def __onDetected(self, event):
        self.__dragStartPos = self.pos
        moveNodeToTop(self)

    def __onMove(self, event, offset):
        self.pos = self.__dragStartPos + offset
        moveNodeOnScreen(self)

    def __onEnd(self, event):
        pass

class HoldNode(TextRect):
    def __init__(self, text, **kwargs):
        TextRect.__init__(self, text, **kwargs)

        self.recognizer = ui.HoldRecognizer(node=self, possibleHandler=self.__onPossible,
                detectedHandler=self.__onDetected, failHandler=self.__onFail,
                stopHandler=self.__onStop)

    def __onPossible(self, event):
        self.rect.fillcolor = "FFFFFF"
        self.rect.color = "FFFFFF"
        self.words.color = "000000"

    def __onDetected(self, event):
        self.rect.fillcolor = "000000"
        self.words.color = "FFFFFF"
        self.rect.color = "00FF00"

    def __onFail(self, event):
        self.rect.fillcolor = "000000"
        self.words.color = "FFFFFF"
        self.rect.color = "FF0000"

    def __onStop(self, event):
        self.rect.fillcolor = "000000"
        self.rect.color = "FFFFFF"
        self.words.color = "FFFFFF"

class AbortButton(TextRect):
    def __init__(self, text, **kwargs):
        TextRect.__init__(self, text, **kwargs)

        self.recognizer = ui.TapRecognizer(node=self,
                    possibleHandler=self.__onPossible, detectedHandler=self.__onDetected,
                    failHandler=self.__onFail)

    def __onPossible(self, event):
        self.rect.fillcolor = "FFFFFF"
        self.words.color = "000000"

    def __onDetected(self, event):
        abortAll()
        self.rect.fillcolor = "000000"
        self.words.color = "FFFFFF"
        self.rect.color = "00FF00"

    def __onFail(self, event):
        self.rect.fillcolor = "000000"
        self.words.color = "FFFFFF"
        self.rect.color = "FF0000"

class EnableButton(TextRect):
    def __init__(self, text, **kwargs):
        TextRect.__init__(self, text, **kwargs)

        self.recognizer = ui.TapRecognizer(node=self,
                    possibleHandler=self.__onPossible, detectedHandler=self.__onDetected,
                    failHandler=self.__onFail)
        self.words.color = "FF0000"

    def changeText(self):
        if(nodesEnabled):
            self.words.text = "Disable all"
            self.words.color = "FF0000"
        else:
            self.words.text = "Enable all"
            self.words.color = "00FF00"

    def __onPossible(self, event):
        self.rect.fillcolor = "FFFFFF"

    def __onDetected(self, event):
        switchNodesEnabled()
        self.changeText()
        self.rect.fillcolor = "000000"
        self.rect.color = "00FF00"

    def __onFail(self, event):
        self.rect.fillcolor = "000000"
        self.rect.color = "FF0000"

class GestureDemoApp(libavg.AVGApp):

    def init(self):

        avg.WordsNode(text='''a - abort recognition <br/>
                d - switch enable/disable recognition <br/><br/>
                or use the buttons on the right side''',
                pos=(20, 510), parent=self._parentNode)

        nodeList.append(HoldNode(text="HoldRecognizer", pos=(20,20),
                parent=self._parentNode))

        nodeList.append(DragNode(text="DragRecognizer<br/>friction", pos=(200,20),
                friction=0.05, parent=self._parentNode))

        nodeList.append(TransformNode(text="TransformRecognizer",
                ignoreRotation=False, ignoreScale=False,
                pos=(380,20), parent=self._parentNode))

        self.abortButton = AbortButton(text="Abort all", pos = (630, 490),
                parent=self._parentNode)

        self.enableButton = EnableButton(text="Disable all", pos = (630, 540),
                parent=self._parentNode)

    def onKeyDown(self, event):
        if event.keystring == 'a':
            abortAll()
        if event.keystring == 'd':
            switchNodesEnabled()
            self.enableButton.changeText()
        else:
            libavg.AVGApp.onKeyDown(self, event)


if __name__ == '__main__':
    GestureDemoApp.start(resolution=RESOLUTION)
