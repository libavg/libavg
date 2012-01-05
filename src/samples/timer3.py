#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import avg

def moveText():
    global node
    if node.x < 200:
        node.x += 20
    else:
        player.clearInterval(timer)

player = avg.Player.get()

canvas = player.createMainCanvas(size=(640,480))
rootNode = canvas.getRootNode()
node = avg.WordsNode(pos=(10,10), font="arial", text="Hello World", parent=rootNode)
timer = player.setInterval(200, moveText)

player.play()

