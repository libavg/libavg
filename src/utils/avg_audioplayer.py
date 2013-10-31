#!/usr/bin/env python
# -*- coding: utf-8 -*-

# libavg - Media Playback Engine.
# Copyright (C) 2003-2013 Ulrich von Zadow
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

import sys
from libavg import avg, app, player


class AudioPlayerDiv(app.MainDiv):
    def onArgvParserCreated(self, parser):
        parser.set_usage("%prog <filename>")

    def onArgvParsed(self, options, args, parser):
        if len(args) != 1:
            parser.print_help()
            sys.exit(1)
        self._audioFName = args[0]

    def onInit(self):
        self._node = avg.SoundNode(parent=self, href=self._audioFName)
        self._node.play()
        self._words = avg.WordsNode(parent=self, pos=(10, 22), fontsize=10)

    def onFrame(self):
        curTime = self._node.getCurTime()
        self._words.text = "Time: "+str(curTime/1000.0)


if __name__ == "__main__":
    app.App().run(AudioPlayerDiv(), app_resolution="320x200")

