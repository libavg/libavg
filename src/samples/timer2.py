#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import avg, player

def moveText():
    global node
    if node.x < 200:
        node.x += 1

canvas = player.createMainCanvas(size=(640,480))
rootNode = canvas.getRootNode()
node = avg.WordsNode(pos=(10,10), text="Hello World", parent=rootNode)
player.subscribe(player.ON_FRAME, moveText)

player.play()

