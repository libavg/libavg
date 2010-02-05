#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import avg

def clickFunc(event):
    node = player.getElementByID("mouseBlock")
    node.fillcolor = "FF0000"
 
def keyboardFunc(event):
    if event.keystring == 'x':
        node = player.getElementByID("mouseBlock")
        node.fillcolor = "00FF00"
         
player = avg.Player.get()

player.loadString('''
<?xml version="1.0"?>
<avg width="640" height="480">
  <rect pos="(10,10)" size="(250,250)" fillopacity="1" id="mouseBlock"/>
</avg>
''')

mouseBlock = player.getElementByID("mouseBlock")
mouseBlock.setEventHandler(avg.CURSORDOWN, avg.MOUSE, clickFunc)

rootNode = player.getRootNode()
rootNode.setEventHandler(avg.KEYDOWN, avg.NONE, keyboardFunc)

player.play()

