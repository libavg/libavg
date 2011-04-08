#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import avg

offset = None

def onMouseDown(event):
    global offset
    node = event.node
    offset = node.getRelPos((event.x, event.y))
    node.setEventCapture()

def onMouseMove(event):
    global offset
    node = event.node
    if offset != None:
        node.x = event.x-offset[0]
        node.y = event.y-offset[1]

def onMouseUp(event):
    global offset
    node = event.node
    if offset != None:
        node.releaseEventCapture()
        offset = None;

player = avg.Player.get()

player.loadFile("text.avg")
node = player.getElementByID("HelloText")
node.setEventHandler(avg.CURSORDOWN, avg.MOUSE, onMouseDown)
node.setEventHandler(avg.CURSORMOTION, avg.MOUSE, onMouseMove)
node.setEventHandler(avg.CURSORUP, avg.MOUSE, onMouseUp)

player.play()

