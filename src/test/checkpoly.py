#!/usr/bin/env python
# -*- coding: utf-8 -*-

# libavg - Media Playback Engine.
# Copyright (C) 2015-2020 Ulrich von Zadow
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

from libavg import app, avg
from random import randint, random


class MainDiv(app.MainDiv):
    def onInit(self):
        self.poly = avg.PolygonNode(pos=self.__genPts(), fillopacity=0.5, parent=self)

    def onFrame(self):
        self.poly.pos = self.__genPts()

    def __genPts(self):
        def randPos():
            return avg.Point2D(randint(0,1400), randint(0,800))

        basePts = [randPos() for i in range(10)]
        pts = []
        for i in range(1000): 
            basePt = basePts[randint(0,9)]
            pts.append(basePt + (random(), random()))
        return pts

app.App().run(MainDiv(), app_resolution='1400, 800')
