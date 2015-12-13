#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import app, avg

class MainDiv(app.MainDiv):
    def onInit(self):
        self.node = avg.WordsNode(pos=(10,10), 
                text="Should I stay or should I go?", parent=self)
        self.node.subscribe(avg.Node.MOUSE_WHEEL, self.onMouseWheel)

    def onMouseWheel(self, event):
        print event.motion.y
        self.node.pos = (10, 50)
#        self.node.fontsize += event.motion.y
#        print self.node.fontsize

app.App().run(MainDiv())
