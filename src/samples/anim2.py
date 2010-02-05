#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import *

def startAnim():
    animObj.start()

player = avg.Player.get()
player.loadFile("text.avg")

node = player.getElementByID("ClashText")
animObj = ParallelAnim(
    [LinearAnim(node, "x", 2000, 0, 200),
     LinearAnim(node, "y", 2000, 0, 10)])
player.setTimeout(0, startAnim)

player.play()

