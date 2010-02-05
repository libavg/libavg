#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import avg

def moveText():
    Node = Player.getElementByID("ClashText")
    if Node.x < 200:
        Node.x += 20

Player = avg.Player.get()

Player.loadFile("text.avg")
Player.setInterval(200, moveText)

Player.play()

