#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import avg, player

canvas = player.createMainCanvas(size=(160,120))
rootNode = canvas.getRootNode()
avg.ImageNode(pos=(80,30), size=(40,30), href="rgb24-64x64.png", angle=1.570, pivot=(0,0),
        parent=rootNode)
player.play()

