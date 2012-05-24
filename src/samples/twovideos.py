#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import *
import sys

FADE_DURATION = 2000

class HDVideoApp(AVGApp):
    def init(self):
        self.videoNodes = []
        for fileName in (sys.argv[1], sys.argv[2]):
            videoNode = VideoNode(
                    size=(1680, 1050), 
                    href=fileName,
                    opacity=0,
                    loop=True,
                    parent=self._parentNode)
            videoNode.play()
            self.videoNodes.append(videoNode)

        self.videoNodes[0].opacity = 1
        self.runningVideo = 0
        self.isFading = False

    def onKeyDown(self, event):
        if event.keystring == '1':
            if not(self.isFading):
                if self.runningVideo == 0:
                    fadeIn(self.videoNodes[1], FADE_DURATION)
                else:
                    fadeOut(self.videoNodes[1], FADE_DURATION)
                player.setTimeout(FADE_DURATION, self.fadeEnd)
                self.runningVideo = (self.runningVideo+1)%2
                self.isFading = True
            return True
        else:
            return False

    def fadeEnd(self):
        self.isFading = False
        
HDVideoApp.start(resolution=(1680, 1050), debugWindowSize=(720, 450))
