#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# libavg - Media Playback Engine.
# Copyright (C) 2003-2011 Ulrich von Zadow
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
#

import sys, os
import time
import optparse
from libavg import avg, AVGApp

g_Player = avg.Player.get()

class VideoPlayer(AVGApp):

    def __init__(self, parentNode):
        if options.fullscreen:
            g_Player.setResolution(True, 1920, 1200, 0)
        AVGApp.__init__(self, parentNode)

    def init(self):
        self.node = avg.VideoNode(href=args[0], loop=True, 
                accelerated=not(options.disableAccel))
        self.node.play()
        if self.node.hasAlpha():
            self.__makeAlphaBackground()
        self._parentNode.appendChild(self.node)
        avg.WordsNode(parent=self._parentNode, id="curframe", pos=(10, 10), 
                font="arial", fontsize=10)
        avg.WordsNode(parent=self._parentNode, id="curtime", pos=(10, 22), 
                font="arial", fontsize=10)
        avg.WordsNode(parent=self._parentNode, id="framesqueued", pos=(10, 34), 
                font="arial", fontsize=10)

        g_Player.setOnFrameHandler(self.onFrame)
    
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

    def onFrame(self):
        curFrame = self.node.getCurFrame()
        numFrames = self.node.getNumFrames()
        g_Player.getElementByID("curframe").text = "Frame: %i/%i"%(curFrame, numFrames)
        curVideoTime = self.node.getCurTime()
        g_Player.getElementByID("curtime").text = "Time: "+str(curVideoTime/1000.0)
        framesQueued = self.node.getNumFramesQueued()
        g_Player.getElementByID("framesqueued").text = "Frames queued: "+str(framesQueued)

    def __makeAlphaBackground(self):
        SQUARESIZE=40
        size = self.node.getMediaSize()
        avg.RectNode(parent=self._parentNode, size=self.node.getMediaSize(), 
                strokewidth=0, fillcolor="FFFFFF", fillopacity=1)
        for y in xrange(0, int(size.y)/SQUARESIZE):
            for x in xrange(0, int(size.x)/(SQUARESIZE*2)):
                pos = avg.Point2D(x*SQUARESIZE*2, y*SQUARESIZE)
                if y%2==1:
                    pos += (SQUARESIZE, 0)
                avg.RectNode(parent=self._parentNode, pos=pos,
                        size=(SQUARESIZE, SQUARESIZE), strokewidth=0, fillcolor="C0C0C0",
                        fillopacity=1)

parser = optparse.OptionParser("Usage: %prog <filename> [options]")
parser.add_option("-d", "--disable-accel", dest="disableAccel", action="store_true",
        default=False, help="disable vdpau acceleration")
parser.add_option("-f", "--fullscreen", dest="fullscreen", action="store_true",
        default=False)
(options, args) = parser.parse_args()

if len(args) == 0:
    parser.print_help()
    sys.exit(1)

argsNode = avg.VideoNode(href=args[0], loop=True, accelerated=False)
argsNode.pause()
VideoPlayer.start(resolution=argsNode.getMediaSize())

