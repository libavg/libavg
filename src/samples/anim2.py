#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import *

def startAnim():
    animObj.start()

canvas = player.createMainCanvas(size=(640,480))
rootNode = canvas.getRootNode()
node = avg.WordsNode(pos=(10,10), text="Hello World", parent=rootNode)

animObj = ParallelAnim(
    [LinearAnim(node, "x", 2000, 0, 200),
     LinearAnim(node, "y", 2000, 0, 10)])
player.setTimeout(0, startAnim)

player.play()

