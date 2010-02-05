#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import avg

Player = avg.Player.get()

Player.loadFile("text.avg")
Node = Player.getElementByID("ClashText")
print Node.x
Node.x = 200
Player.play()
