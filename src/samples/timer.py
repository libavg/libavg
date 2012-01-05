#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import avg

def moveText():
    global node
    node.x = 200

player = avg.Player.get()

canvas = player.createMainCanvas(size=(640,480))
rootNode = canvas.getRootNode()
node = avg.WordsNode(pos=(10,10), font="arial", text="Hello World", parent=rootNode)
player.setTimeout(1000, moveText)

player.play()

