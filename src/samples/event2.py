#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import avg

def onDiv(Event):
    Node = Player.getElementByID("ClashText")
    Node.color = "FF8000"

def onWords(Event):
    Node = Player.getElementByID("ClashText")
    Node.color = "00FF00"

Player = avg.Player.get()

Player.loadFile("event2.avg")
Player.play()

