#!/usr/bin/env python

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
