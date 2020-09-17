#!/usr/bin/env python
# -*- coding: utf-8 -*-

# libavg - Media Playback Engine.
# Copyright (C) 2010-2020 Ulrich von Zadow
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

from libavg import avg, app
import sys

class VideoPlayer(app.MainDiv):

    def onArgvParserCreated(self, parser):
        parser.set_usage("%prog [options] <video>")

    def onArgvParsed(self, options, args, parser):
        if len(args) != 1:
            parser.print_help()
            sys.exit(1)

        self.__dir=args[0]

    def onInit(self):
        self.videoNode = avg.VideoNode(
                href=self.__dir,
                size=(500, 500),
                parent=self)
        self.videoNode.play()
        
app.App().run(VideoPlayer(), app_resolution='1920x1080', app_window_size='720x450')
