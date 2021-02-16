#!/usr/bin/env python
# -*- coding: utf-8 -*-

# libavg - Media Playback Engine.
# Copyright (C) 2015-2021 Ulrich von Zadow
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
# Current versions can be found at www.libavg.de

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
