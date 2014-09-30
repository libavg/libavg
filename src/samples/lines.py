#!/usr/bin/env python
# -*- coding: utf-8 -*-

import libavg
from libavg import app

import math

class LinesDiv(app.MainDiv):
    def onInit(self):
        self.toggleTouchVisualization()
        self.__createLines(0)
        self.__createLines(250, "dottedline.png", 0, 20)

        self.polyLine = libavg.PolyLineNode(texhref="dottedline.png", strokewidth=2,
                parent=self)
        self.growLine = True
        self.lineLen = 120

        libavg.PolyLineNode(pos=((10,100),(20,110),(30,100),(40,90),(50,100)),
                texhref="dottedline.png", texcoords=(0,5), parent=self) 

    def __createLines(self, xstart, texhref="", texcoord1=0, texcoord2=1):
        div = libavg.DivNode(pos=(xstart,0), parent=self)
        libavg.LineNode(pos1=(10,10.5), pos2=(200,10.5), strokewidth=1, 
                texhref=texhref, texcoord1=texcoord1, texcoord2=texcoord2, parent=div)
        libavg.LineNode(pos1=(10,30), pos2=(200,30), strokewidth=2, 
                texhref=texhref, texcoord1=texcoord1, texcoord2=texcoord2, parent=div)
        libavg.CurveNode(pos1=(10,50), pos2=(20,60), pos3=(190,60), pos4=(200,50),
                strokewidth=2, texhref=texhref, texcoord1=texcoord1, texcoord2=texcoord2,
                parent=div)



    def onExit(self):
        pass
    
    def onFrame(self):
        if self.growLine:
            self.lineLen += 1
            if self.lineLen > 300:
                self.growLine = False
        else:
            self.lineLen -= 1
            if self.lineLen < 100:
                self.growLine = True
        self.__calcPolyLine()

    def __calcPolyLine(self):
        pos = []
        texcoords = []
        for i in xrange(20):
            xpos = (10.0*i)/100*self.lineLen
            pos.append((xpos, 130+10*math.sin(i*2)))
            texcoords.append(xpos/10)
        self.polyLine.pos = pos
        self.polyLine.texcoords = texcoords

app.App().run(LinesDiv())

