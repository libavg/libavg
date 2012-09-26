#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import * 
import os, sys

THUMBNAIL_WIDTH = 320
FADE_DURATION = 1000

class VideoChooserApp(AVGApp):
    def init(self):
        player.showCursor(True)
        dir = sys.argv[1]

        self.videoListNode = DivNode(parent=self._parentNode)
        self.videoNodes = []
        fileNames = os.listdir(dir)
        i = 0
        for fileName in fileNames:
            try:
                videoNode = VideoNode(
                        pos = (i*(THUMBNAIL_WIDTH+20), 0),
                        href = dir+"/"+fileName,
                        loop = True,
                        mipmap=True,
                        parent = self.videoListNode)
                videoNode.play()
                self.videoNodes.append(videoNode)

                size = videoNode.getMediaSize()
                height = (THUMBNAIL_WIDTH*size.y)/size.x
                videoNode.size = (THUMBNAIL_WIDTH, height)
                videoNode.subscribe(videoNode.CURSOR_DOWN,
                        lambda event, videoNode=videoNode: 
                                self.chooseVideo(event, videoNode))
                i += 1
            except RuntimeError:
                pass

        self._parentNode.subscribe(self._parentNode.CURSOR_MOTION, self.onMouseMove)
        self.bigVideoNode = None

    def onMouseMove(self, event):
        windowWidth = player.getRootNode().width
        ratio = event.x/float(windowWidth)
        self.videoListNode.x = -(ratio*(self.getTotalWidth()-windowWidth))

    def chooseVideo(self, event, videoNode):
        if self.bigVideoNode:
            self.removeBigVideo()
        destSize = videoNode.size*2
        destPos = Point2D(720, 550)-destSize/2
        absPos = videoNode.getAbsPos(Point2D(0,0))
        frame = videoNode.getCurFrame()
        self.bigVideoNode = VideoNode(href=videoNode.href, loop=True, sensitive=False,
                parent=self._parentNode)
        self.bigVideoNode.play()
        self.bigVideoNode.seekToFrame(frame)
        EaseInOutAnim(self.bigVideoNode, "pos", 1000, absPos, destPos, False,
                300, 300).start()
        EaseInOutAnim(self.bigVideoNode, "size", 1000, videoNode.size, destSize, False,
                300, 300).start()

    def removeBigVideo(self):
        oldVideoNode = self.bigVideoNode
        fadeOut(oldVideoNode, FADE_DURATION, lambda: oldVideoNode.unlink(True))

    def getTotalWidth(self):
        return (THUMBNAIL_WIDTH+20)*len(self.videoNodes)

VideoChooserApp.start(resolution=(1440, 900), debugWindowSize=(720, 450))

