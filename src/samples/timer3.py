#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import avg

def moveText():
    node = player.getElementByID("ClashText")
    if node.x < 200:
        node.x += 20

player = avg.Player.get()

player.loadFile("text.avg")
player.setInterval(200, moveText)

player.play()

