#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import app, avg, widget

class MainDiv(app.MainDiv):
    def onInit(self):
        self.textNode = avg.WordsNode(pos=(10,10), text="", parent=self) 
        for i in range(0,10):
            pos = avg.Point2D(10,40) + ((i%3)*40, (i//3)*40)
            node = widget.TextButton(pos=pos, size=(30,30), text=str(i), parent=self)
            node.subscribe(widget.Button.CLICKED, lambda i=i: self.onDown(i))

    def onDown(self, i):
        self.textNode.text += str(i)

app.App().run(MainDiv())
