#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import avg

def moveText():
    node = player.getElementByID("ClashText")
    node.x = 200

player = avg.Player.get()

player.loadFile("text.avg")
player.setTimeout(1000, moveText)

player.play()

