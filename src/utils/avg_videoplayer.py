#!/usr/bin/env python
# -*- coding: utf-8 -*-

# libavg - Media Playback Engine.
# Copyright (C) 2003-2013 Ulrich von Zadow
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
from libavg import avg, app, player, widget


class VideoPlayer(app.MainDiv):
    CONTROL_WIDTH=240

    def onArgvParserCreated(self, parser):
        parser.set_usage("%prog [options] <filename>")
        parser.add_option("-d", "--disable-accel", dest="disableAccel",
                action="store_true", default=False,
                help="disable vdpau acceleration")

    def onArgvParsed(self, options, args, parser):
        if len(args) != 1:
            parser.print_help()
            sys.exit(1)

        self.node = avg.VideoNode(href=args[0], accelerated=not(options.disableAccel))
        self.node.pause()

        mediaSize = self.node.getMediaSize()
        size = avg.Point2D(max(mediaSize.x, 320), max(mediaSize.y, 120))
        screenSize = player.getScreenResolution()
        size = avg.Point2D(min(size.x, screenSize.x), min(size.y, screenSize.y-80))
        self.settings.set("app_resolution", "%dx%d" %(size.x, size.y))

    def onInit(self):
        self.node.play()

        mediaSize = self.node.getMediaSize()
        canvasSize = self.size
        sizeRatio = min(mediaSize.x/canvasSize.x, mediaSize.y/canvasSize.y)
        self.node.size /= sizeRatio

        self.node.x = (self.width-self.node.width)/2
        self.node.y = (self.height-self.node.height)/2
        self.node.subscribe(avg.VideoNode.END_OF_FILE, self.onEOF)

        if self.node.hasAlpha():
            self.__makeAlphaBackground()
        self.appendChild(self.node)
        self.curFrameWords = avg.WordsNode(parent=self, pos=(10, 10), fontsize=10)
        self.framesQueuedWords = avg.WordsNode(parent=self, pos=(10, 22), fontsize=10)

        controlPos = ((self.width-VideoPlayer.CONTROL_WIDTH)/2, self.height-25)
        self.videoControl = widget.MediaControl(pos=controlPos,
                size=(VideoPlayer.CONTROL_WIDTH, 20),
                duration=self.node.getDuration(),
                parent=self)
        self.videoControl.play()
        self.videoControl.subscribe(widget.MediaControl.PLAY_CLICKED, self.onPlay)
        self.videoControl.subscribe(widget.MediaControl.PAUSE_CLICKED, self.onPause)
        self.videoControl.subscribe(widget.MediaControl.SEEK_PRESSED, self.onSeekStart)
        self.videoControl.subscribe(widget.MediaControl.SEEK_RELEASED, self.onSeekEnd)
        self.videoControl.subscribe(widget.MediaControl.SEEK_MOTION, self.onSeek)

        self.isSeeking = False
        self.isPaused = False

    def onKeyDown(self, event):
        curTime = self.node.getCurTime()
        if event.keystring == "right":
            self.node.seekToTime(curTime+10000)
        elif event.keystring == "left":
            if curTime > 10000:
                self.node.seekToTime(curTime-10000)
            else:
                self.node.seekToTime(0)
        return False

    def onFrame(self, dt):
        curFrame = self.node.getCurFrame()
        numFrames = self.node.getNumFrames()
        self.curFrameWords.text = "Frame: %i/%i"%(curFrame, numFrames)
        framesQueued = self.node.getNumFramesQueued()
        self.framesQueuedWords.text = "Frames queued: "+str(framesQueued)
        if not(self.isSeeking):
            self.videoControl.time = self.node.getCurTime()

    def onEOF(self):
        self.videoControl.pause()
        self.isPaused = True

    def onPlay(self):
        self.node.play()
        self.isPaused = False

    def onPause(self):
        self.node.pause()
        self.isPaused = True

    def onSeekStart(self):
        self.node.pause()
        self.isSeeking = True

    def onSeekEnd(self):
        if not(self.isPaused):
            self.node.play()
        self.isSeeking = False

    def onSeek(self, time):
        self.node.seekToTime(int(time))

    def __makeAlphaBackground(self):
        SQUARESIZE=40
        size = self.node.getMediaSize()
        avg.RectNode(parent=self, size=self.node.getMediaSize(),
                strokewidth=0, fillcolor="FFFFFF", fillopacity=1)
        for y in xrange(0, int(size.y)/SQUARESIZE):
            for x in xrange(0, int(size.x)/(SQUARESIZE*2)):
                pos = avg.Point2D(x*SQUARESIZE*2, y*SQUARESIZE)
                if y%2==1:
                    pos += (SQUARESIZE, 0)
                avg.RectNode(parent=self, pos=pos, size=(SQUARESIZE, SQUARESIZE),
                        strokewidth=0, fillcolor="C0C0C0", fillopacity=1)


if __name__ == "__main__":
    app.App().run(VideoPlayer())

