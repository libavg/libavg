#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import avg

def onMouseOver(event):
    node = player.getElementByID("ClashText")
    node.color = "FF8000"

def onMouseOut(event):
    node = player.getElementByID("ClashText")
    node.color = "FFFFFF"

player = avg.Player.get()

player.loadFile("text.avg")
node = player.getElementByID("ClashText")
node.setEventHandler(avg.CURSOROVER, avg.MOUSE, onMouseOver)
node.setEventHandler(avg.CURSOROUT, avg.MOUSE, onMouseOut)
player.play()

