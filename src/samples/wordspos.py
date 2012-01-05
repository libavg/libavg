#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import avg

player = avg.Player.get()

canvas = player.createMainCanvas(size=(160,120))
rootNode = canvas.getRootNode()
avg.WordsNode(pos=(10,10), width=70, text="<i>Left-justified paragraph</i>",
        parent=rootNode)
avg.WordsNode(pos=(150,10), width=70, alignment="right", text="Right-justified paragraph",
        parent=rootNode)
avg.WordsNode(pos=(80,80), width=70, alignment="center", text="Centered paragraph",
        parent=rootNode)
player.play()

