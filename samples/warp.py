#!/usr/bin/env python
# -*- coding: utf-8 -*-

# libavg - Media Playback Engine.
# Copyright (C) 2016-2021 Ulrich von Zadow
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
# Current versions can be found at www.libavg.de

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

