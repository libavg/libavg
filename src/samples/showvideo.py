#!/usr/bin/env python
# -*- coding: utf-8 -*-

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
