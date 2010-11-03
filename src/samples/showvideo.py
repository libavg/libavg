#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import * 
import sys

g_player = avg.Player.get()

class VideoApp(AVGApp):
    def init(self):
        g_player.enableMultitouch()
        g_player.setFramerate(60)
        self.videoNode = VideoNode(
                href=sys.argv[1],
                parent=self._parentNode)
        self.videoNode.play()
        
VideoApp.start(resolution=(1440, 900), debugWindowSize=(720, 450))
