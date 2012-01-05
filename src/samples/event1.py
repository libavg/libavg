#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import avg

def onMouseOver(event):
    global node
    node.color = "FF8000"

def onMouseOut(event):
    global node
    node.color = "FFFFFF"

player = avg.Player.get()

canvas = player.createMainCanvas(size=(640,480))
rootNode = canvas.getRootNode()
node = avg.WordsNode(pos=(10,10), font="arial", text="Hello World", parent=rootNode)
node.setEventHandler(avg.CURSOROVER, avg.MOUSE, onMouseOver)
node.setEventHandler(avg.CURSOROUT, avg.MOUSE, onMouseOut)
player.play()

