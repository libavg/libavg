#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import avg, player

def onMouseOver(event):
    global node
    node.color = "FF8000"

def onMouseOut(event):
    global node
    node.color = "FFFFFF"

canvas = player.createMainCanvas(size=(640,480))
rootNode = canvas.getRootNode()
node = avg.WordsNode(pos=(10,10), text="Hello World", parent=rootNode)
node.subscribe(node.CURSOR_OVER, onMouseOver)
node.subscribe(node.CURSOR_OUT, onMouseOut)
player.play()

