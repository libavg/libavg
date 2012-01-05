#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import avg

player = avg.Player.get()

offscreenCanvas = player.createCanvas(id="londoncalling", size=(320,240))
avg.WordsNode(pos=(10,10), text="London Calling", font="Arial", 
        parent=offscreenCanvas.getRootNode())

mainCanvas = player.createMainCanvas(size=(640,480))
rootNode = mainCanvas.getRootNode()
avg.ImageNode(href="canvas:londoncalling", parent=rootNode)

player.play()
