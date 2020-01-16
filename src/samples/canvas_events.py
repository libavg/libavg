#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import app, avg, player

class MyMainDiv(app.MainDiv):
    def onInit(self):
        self.offscreenCanvas = player.createCanvas(id="foo", size=(640, 480),
                handleevents=True)
        self.img=avg.ImageNode(href="rgb24-64x64.png", pos=(160,120), size=(320,320),
                parent=self.offscreenCanvas.getRootNode())
        avg.ImageNode(href="canvas:foo", parent=self, pos=(160,120), size=(320,240))

        self.img.subscribe(self.img.CURSOR_DOWN, self.onDown)

    def onDown(self, event):
        print "down: ", event.pos

app.App().run(MyMainDiv())
