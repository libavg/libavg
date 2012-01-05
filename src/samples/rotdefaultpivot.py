#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import avg

player = avg.Player.get()

canvas = player.createMainCanvas(size=(160,120))
rootNode = canvas.getRootNode()
avg.ImageNode(pos=(40,30), size=(80,60), href="rgb24-64x64.png", angle=1.570,
        parent=rootNode)
player.play()


