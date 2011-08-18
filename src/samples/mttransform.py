#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import avg, ui
import libavg

BASE_SIZE=avg.Point2D(256,256)

class TransformApp(libavg.AVGApp):
    multitouch = True

    def init(self):
        self.image = avg.ImageNode(href="rgb24-64x64.png", pos=(100,100),
                size=BASE_SIZE, parent=self._parentNode)
        self.transformer = ui.TransformRecognizer(
                node=self.image, 
                startHandler=self.__onStart,
                moveHandler=self.__onMove,
                upHandler=self.__onUp,
                ignoreScale=True)

    def __onStart(self):
        self.baseTransform = ui.Mat3x3.fromNode(self.image)

    def __onMove(self, transform):
        totalTransform = transform.applyMat(self.baseTransform)
        totalTransform.setNodeTransform(self.image)

    def __onUp(self, transform):
        pass


if __name__ == '__main__':
    TransformApp.start(resolution=(800,600))
