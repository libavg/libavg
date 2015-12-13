#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import app, player

class MainDiv(app.MainDiv):
    def onInit(self):
        player.subscribe(player.KEY_DOWN, self.onKey)
        player.subscribe(player.KEY_UP, self.onKey)

    def onKey(self, event):
        print ("Key event: type=" + str(event.type) 
                + ", scancode=" + str(event.scancode) + ", keyname=" + event.keyname
                + ", text=" + event.text)

app.App().run(MainDiv())
