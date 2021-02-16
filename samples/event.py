#!/usr/bin/env python
# -*- coding: utf-8 -*-

# libavg - Media Playback Engine.
# Copyright (C) 2014-2021 Ulrich von Zadow
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
