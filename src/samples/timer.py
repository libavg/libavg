#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import avg

def moveText():
    Node = Player.getElementByID("ClashText")
    Node.x = 200

Player = avg.Player.get()

Player.loadFile("text.avg")
Player.setTimeout(1000, moveText)

Player.play()

