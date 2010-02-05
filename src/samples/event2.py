#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import avg

def onDiv(event):
    node = player.getElementByID("ClashText")
    node.color = "FF8000"

def onWords(event):
    node = player.getElementByID("ClashText")
    node.color = "00FF00"

player = avg.Player.get()

player.loadFile("event2.avg")
player.play()

