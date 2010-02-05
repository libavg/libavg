#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import avg

def onMouseOver(Event):
    Node = Player.getElementByID("ClashText")
    Node.color = "FF8000"

def onMouseOut(Event):
    Node = Player.getElementByID("ClashText")
    Node.color = "FFFFFF"

Player = avg.Player.get()

Player.loadFile("event1.avg")
Player.play()

