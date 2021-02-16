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

import os, sys
from libavg import avg, app, player, Point2D


class VideoChooser(app.MainDiv):
    def onArgvParserCreated(self, parser):
        parser.set_usage("%prog <folder>")
        parser.add_option('--duration', '-d', dest='duration',
                default=2000, type='int', help='Fade duration')
        parser.add_option('--width', '-w', dest='thumbWidth',
                default=320, type='int', help='Thumbnail width')

    def onArgvParsed(self, options, args, parser):
        if len(args) != 1:
            parser.print_help()
            sys.exit(1)

        self.__folder = args[0]
        self.__duration = options.duration
        self.__thumbWidth = options.thumbWidth

    def onInit(self):
        player.showCursor(True)

        self.videoListNode = avg.DivNode(parent=self)
        self.videoNodes = []
        fileNames = os.listdir(self.__folder)
        i = 0
        for fileName in fileNames:
            try:
                videoNode = avg.VideoNode(
                        pos=(i*(self.__thumbWidth+20), 0),
                        href=self.__folder+'/'+fileName,
                        loop=True,
                        mipmap=True,
                        enablesound=False,
                        parent = self.videoListNode)
                videoNode.play()
                self.videoNodes.append(videoNode)

                size = videoNode.getMediaSize()
                height = (self.__thumbWidth*size.y)/size.x
                videoNode.size = (self.__thumbWidth, height)
                videoNode.subscribe(videoNode.CURSOR_DOWN,
                        lambda event, videoNode=videoNode: 
                                self.chooseVideo(event, videoNode))
                i += 1
            except RuntimeError:
                pass

        self.subscribe(self.CURSOR_MOTION, self.onMouseMove)
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
        self.bigVideoNode = avg.VideoNode(href=videoNode.href, loop=True, sensitive=False,
                parent=self)
        self.bigVideoNode.play()
        self.bigVideoNode.seekToFrame(frame)
        avg.EaseInOutAnim(self.bigVideoNode, "pos", 1000, absPos, destPos, False,
                300, 300).start()
        avg.EaseInOutAnim(self.bigVideoNode, "size", 1000, videoNode.size, destSize,
                False, 300, 300).start()

    def removeBigVideo(self):
        oldVideoNode = self.bigVideoNode
        avg.Anim.fadeOut(oldVideoNode, self.__duration, lambda: oldVideoNode.unlink(True))

    def getTotalWidth(self):
        return (self.__thumbWidth+20)*len(self.videoNodes)


app.App().run(VideoChooser(), app_resolution='1440x900', app_window_size='720x450')

