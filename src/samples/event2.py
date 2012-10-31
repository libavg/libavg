#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import avg, player

def onDiv(event):
    words.color = "FF8000"

def onWords(event):
    words.color = "00FF00"

canvas = player.createMainCanvas(size=(640,480))
rootNode = canvas.getRootNode()
words = avg.WordsNode(pos=(10,10), text="Should I stay or should I go?", 
        parent=rootNode)
div = avg.DivNode(pos=(100,0), size=(80,200), parent=rootNode)
words.subscribe(words.CURSOR_MOTION, onWords)
div.subscribe(div.CURSOR_MOTION, onDiv)

player.play()

