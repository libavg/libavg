#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import avg

player = avg.Player.get()

player.loadFile("text.avg")
node = player.getElementByID("ClashText")
print node.x
node.x = 200
player.play()
