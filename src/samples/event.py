#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import app, avg

class MainDiv(app.MainDiv):
    def onInit(self):
        self.node = avg.WordsNode(pos=(10,10), 
                text="Should I stay or should I go?", parent=self)
        div = avg.DivNode(pos=(100,0), size=(80,200), parent=self)
        self.node.subscribe(avg.Node.CURSOR_MOTION, self.onWords)
        div.subscribe(div.CURSOR_MOTION, self.onDiv)

    def onDiv(self, event):
        print "div"
        self.node.color = "FF8000"

    def onWords(self, event):
        print "words"
        self.node.color = "00FF00"

app.App().run(MainDiv())
