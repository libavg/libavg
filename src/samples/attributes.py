#!/usr/bin/env python
# -*- coding: utf-8 -*-

from __future__ import print_function

from libavg import avg, player

canvas = player.createMainCanvas(size=(640,480))
rootNode = canvas.getRootNode()
node = avg.WordsNode(pos=(10,10), text="Hello World", parent=rootNode)
print(node.x)
node.x = 200
player.play()
