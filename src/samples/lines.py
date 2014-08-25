#!/usr/bin/env python
# -*- coding: utf-8 -*-

import libavg
from libavg import app

class LinesDiv(app.MainDiv):
    def onInit(self):
        self.toggleTouchVisualization()
        self.__createLines(0)
        self.__createLines(250, "dottedline.png", 0, 20)

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
        pass

app.App().run(LinesDiv())

