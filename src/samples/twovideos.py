#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import libavg
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

        app.keyboardmanager.bindKeyDown('1', self.onButtonPressed,
                'Crossfade between videos')

    def onButtonPressed(self):
        if not(self.isFading):
            if self.runningVideo == 0:
                avg.fadeIn(self.videoNodes[1], self.__duration)
            else:
                avg.fadeOut(self.videoNodes[1], self.__duration)
            player.setTimeout(self.__duration, self.fadeEnd)
            self.runningVideo = (self.runningVideo+1)%2
            self.isFading = True

    def fadeEnd(self):
        self.isFading = False
        
app.App().run(HDVideo(), app_resolution='1680x1050', app_window_size='720x450')
