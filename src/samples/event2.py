#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import avg

def onDiv(event):
    words.color = "FF8000"

def onWords(event):
    words.color = "00FF00"

player = avg.Player.get()

player.loadFile("event2.avg")
words = player.getElementByID("ClashText")
words.setEventHandler(avg.CURSORMOTION, avg.MOUSE, onWords)
player.getElementByID("div1").setEventHandler(avg.CURSORMOTION, avg.MOUSE, onDiv)

player.play()

