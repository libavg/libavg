#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import *

def startAnim():
    animObj.start()

player = avg.Player.get()
canvas = player.createMainCanvas(size=(640,480))
rootNode = canvas.getRootNode()
node = avg.WordsNode(pos=(10,10), font="arial", text="Hello World", parent=rootNode)

animObj = LinearAnim(node, "x", 2000, 0, 200)
player.setTimeout(0, startAnim)

player.play()

