#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import avg, player

canvas = player.createMainCanvas(size=(160,120))
rootNode = canvas.getRootNode()
videoNode = avg.VideoNode(href="mpeg1-48x48-sound.avi", pos=(10,10), 
        parent=rootNode)
videoNode.play()
player.play()
