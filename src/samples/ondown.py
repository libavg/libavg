#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import app, avg

class MainDiv(app.MainDiv):
    def onInit(self):
        self.node = avg.WordsNode(pos=(10,10), text="Hello World", parent=self)
        self.node.subscribe(avg.Node.CURSOR_DOWN, self.onDown)

    def onDown(self, event):
        self.node.x = 200

app.App().run(MainDiv())
