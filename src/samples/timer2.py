#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import avg

def moveText():
    Node = Player.getElementByID("ClashText")
    if Node.x < 200:
        Node.x += 1

Player = avg.Player.get()

Player.loadFile("text.avg")
Player.setOnFrameHandler(moveText)

Player.setVBlankFramerate(1)
Player.play()

