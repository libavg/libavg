#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import *

def startAnim():
    animObj.start()

canvas = player.createMainCanvas(size=(640,480))
rootNode = canvas.getRootNode()
node = avg.RectNode(pos=(10,10), size=(100,100), fillcolor="FF0000", fillopacity=1.0,
        parent=rootNode)

animObj = ParallelAnim(
    [LinearAnim(node, "pos", 2000, (0,0), (200,10)),
     LinearAnim(node, "fillcolor", 2000, "00FFFF", "FF0000")])
player.setTimeout(0, startAnim)

player.play()

