#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import *

def startAnim():
    animObj.start()

def createColorBar(y, startColor, endColor):
    for i in xrange(11):
        color = avg.Color.mix(startColor, endColor, 1-(i/10.))
        avg.RectNode(pos=(i*20+1,y), size=(20,20), fillcolor=color, fillopacity=1.0,
                parent=rootNode)
    

canvas = player.createMainCanvas(size=(640,480))
rootNode = canvas.getRootNode()
createColorBar(1, avg.Color("00A0FF"), avg.Color("FFFF00"))
createColorBar(21, avg.Color("FFFFFF"), avg.Color("FF0000"))

player.play()

