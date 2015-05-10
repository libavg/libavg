#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import *

def startAnim():
    animObj.start()

canvas = player.createMainCanvas(size=(640,480))
rootNode = canvas.getRootNode()
startColor = avg.Color("0000FF")
endColor = avg.Color("FFFF00")
for i in xrange(11):
    color = avg.Color.mix(startColor, endColor, 1-(i/10.))
    avg.RectNode(pos=(i*20,0), size=(20,20), fillcolor=color, fillopacity=1.0,
            parent=rootNode)

player.play()

