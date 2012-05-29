#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import avg, player

canvas = player.createMainCanvas(size=(160,120))
rootNode = canvas.getRootNode()
divNode = avg.DivNode(pos=(40,30), parent=rootNode)
avg.ImageNode(pos=(40,30), size=(80,60), href="rgb24-64x64.png", parent=divNode)
player.play()
