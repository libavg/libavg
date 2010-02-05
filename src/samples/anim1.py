#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import *

def startAnim():
    animObj.start()

player = avg.Player.get()
player.loadFile("text.avg")

node = player.getElementByID("ClashText")
animObj = LinearAnim(node, "x", 2000, 0, 200)
player.setTimeout(0, startAnim)

player.play()

