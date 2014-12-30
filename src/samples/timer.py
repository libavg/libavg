#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import avg, app, player


class MainDiv(app.MainDiv):
    def onInit(self):
        self.node = avg.WordsNode(pos=(10,10), text="Hello World", parent=self)

        player.setTimeout(1000, self.moveText)

    def moveText(self):
        self.node.x = 200

app.App().run(MainDiv())
