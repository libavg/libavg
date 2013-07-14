#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import avg, app
import sys

class VideoPlayer(app.MainDiv):
    def init(self):
        self.videoNode = avg.VideoNode(
                href=sys.argv[1],
                parent=self)
        self.videoNode.play()
        
app.App().run(VideoPlayer(), app_resolution='1920x1080', app_window_size='720x450')
