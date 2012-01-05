#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import avg

player = avg.Player.get()

canvas = player.createMainCanvas(size=(160,120))
rootNode = canvas.getRootNode()
divNode = avg.DivNode(pos=(120,30), pivot=(0,0), angle=1.570, parent=rootNode)
avg.ImageNode(pos=(40,30), size=(40,30), href="rgb24-64x64.png", parent=divNode)
player.play()



