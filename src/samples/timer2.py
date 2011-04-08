#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import avg

def moveText():
    node = player.getElementByID("HelloText")
    if node.x < 200:
        node.x += 1

player = avg.Player.get()

player.loadFile("text.avg")
player.setOnFrameHandler(moveText)

player.setVBlankFramerate(1)
player.play()

