#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import avg

offset = None

def onMouseDown(Event):
    global offset
    Node = Event.node
    offset = Node.getRelPos((Event.x, Event.y))
    Node.setEventCapture()

def onMouseMove(Event):
    global offset
    Node = Event.node
    if offset != None:
        Node.x = Event.x-offset[0]
        Node.y = Event.y-offset[1]

def onMouseUp(Event):
    global offset
    Node = Event.node
    if offset != None:
        Node.releaseEventCapture()
        offset = None;

Player = avg.Player.get()

Player.loadFile("drag.avg")
Player.play()

