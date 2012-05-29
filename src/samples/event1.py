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
node = avg.WordsNode(pos=(10,10), font="arial", text="Hello World", parent=rootNode)
node.connectEventHandler(avg.CURSOROVER, avg.MOUSE, node, onMouseOver)
node.connectEventHandler(avg.CURSOROUT, avg.MOUSE, node, onMouseOut)
player.play()

