#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import * 
import sys

g_player = avg.Player.get()

class VideoApp(AVGApp):
    def init(self):
        self.videoNode = VideoNode(
                href=sys.argv[1],
                parent=self._parentNode)
        self.videoNode.play()
        
VideoApp.start(resolution=(1920, 1080), debugWindowSize=(720, 450))
