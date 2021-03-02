#!/usr/bin/env python
# -*- coding: utf-8 -*-

# libavg - Media Playback Engine.
# Copyright (C) 2010-2021 Ulrich von Zadow
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


class HDVideo(app.MainDiv):
    def onArgvParserCreated(self, parser):
        parser.set_usage('%prog <video1> <video2>')
        parser.add_option('--duration', '-d', dest='duration',
                default=2000, type='int', help='Crossfade duration')

    def onArgvParsed(self, options, args, parser):
        if len(args) != 2:
            parser.print_help()
            sys.exit(1)

        self.__videos = args
        self.__duration = options.duration

    def onInit(self):
        self.videoNodes = []
        for fileName in (self.__videos[0], self.__videos[1]):
            videoNode = avg.VideoNode(
                    size=(1680, 1050),
                    href=fileName,
                    opacity=0,
                    loop=True,
                    parent=self)
            videoNode.play()
            self.videoNodes.append(videoNode)

        self.videoNodes[0].opacity = 1
        self.runningVideo = 0
        self.isFading = False

        app.keyboardmanager.bindKeyDown(keyname='1', handler=self.onButtonPressed,
                help='Crossfade between videos')

    def onButtonPressed(self):
        if not(self.isFading):
            if self.runningVideo == 0:
                avg.Anim.fadeIn(self.videoNodes[1], self.__duration)
            else:
                avg.Anim.fadeOut(self.videoNodes[1], self.__duration)
            player.setTimeout(self.__duration, self.fadeEnd)
            self.runningVideo = (self.runningVideo+1)%2
            self.isFading = True

    def fadeEnd(self):
        self.isFading = False
        
app.App().run(HDVideo(), app_resolution='1680x1050', app_window_size='720x450')
