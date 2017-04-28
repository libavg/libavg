#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import app, avg, player
import math

class WarpedImage(avg.ImageNode):
    def __init__(self, parent=None, **kwargs):
        kwargs["maxtilewidth"] = 4
        kwargs["maxtileheight"] = 4
        super(WarpedImage, self).__init__(**kwargs)
        self.registerInstance(self, parent)
        self.__a = 0
        self.__b = 1
        self.__calcVertexes()
        
    @property
    def angle(self):
        return self.__a

    @angle.setter
    def angle(self, a):
        self.__a = a
        self.__calcVertexes()

    @property
    def effect(self):
        return self.__b

    @effect.setter
    def effect(self, b):
        self.__b = b
        self.__calcVertexes()

    def __calcVertexes(self):
        grid = self.getOrigVertexCoords()
        grid = [ [ self.__calcVertex(pos) for pos in line ] for line in grid]
        self.setWarpedVertexCoords(grid)

    def __calcVertex(self, pos):
        x = pos.x - 0.5
        y = pos.y - 0.5
        localAngle = self.__a*math.exp(-(x*x+y*y)/(self.__b*self.__b))
        newPos = avg.Point2D(0,0)
        newPos.x = math.cos(localAngle)*x + math.sin(localAngle)*y
        newPos.y = -math.sin(localAngle)*x + math.cos(localAngle)*y
        return newPos + (0.5,0.5)


class MyMainDiv(app.MainDiv):
    def onInit(self):
        self.warpedImage = WarpedImage(href="rgb24-64x64.png", size=(256,256),
                parent=self)
        self.warpedImage.effect = 0.2


    def onExit(self):
        pass

    def onFrame(self):
        self.warpedImage.angle = math.sin(player.getFrameTime()/1000.)

app.App().run(MyMainDiv())

