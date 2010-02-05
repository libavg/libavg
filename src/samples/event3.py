#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import avg

def clickFunc(Event):
    Node = Player.getElementByID("mouseBlock")
    Node.fillcolor = "FF0000"
 
def keyboardFunc(Event):
    if Event.keystring == 'x':
        Node = Player.getElementByID("mouseBlock")
        Node.fillcolor = "00FF00"
         
Player = avg.Player.get()

Player.loadString('''
<?xml version="1.0"?>
<avg width="640" height="480">
  <rect pos="(10,10)" size="(250,250)" fillopacity="1" id="mouseBlock"/>
</avg>
''')

mouseBlock = Player.getElementByID("mouseBlock")
mouseBlock.setEventHandler(avg.CURSORDOWN, avg.MOUSE, clickFunc)

rootNode = Player.getRootNode()
rootNode.setEventHandler(avg.KEYDOWN, avg.NONE, keyboardFunc)

Player.play()

