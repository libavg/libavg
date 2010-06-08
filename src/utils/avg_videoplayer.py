#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# libavg - Media Playback Engine.
# Copyright (C) 2003-2008 Ulrich von Zadow
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

import sys
from libavg import avg, AVGApp
import time

g_player = avg.Player.get()

def onFrame():
    curFrame = node.getCurFrame()
    numFrames = node.getNumFrames()
    g_player.getElementByID("curframe").text = "Frame: %i/%i"%(curFrame, numFrames)
    curVideoTime = node.getCurTime()
    g_player.getElementByID("curtime").text = "Time: "+str(curVideoTime/1000.0)
    framesQueued = node.getNumFramesQueued()
    g_player.getElementByID("framesqueued").text = "Frames queued: "+str(framesQueued)

def onKey(event):
    node = g_player.getElementByID("video")
    if event.keystring == "right":
        node.seekToTime(node.getCurTime()+10000)
    elif event.keystring == "up":
        node.volume += 0.1
        print "Volume: ", node.volume
    elif event.keystring == "down":
        node.volume -= 0.1
        print "Volume: ", node.volume

class VideoPlayer(AVGApp):
    def init(self):
        global node
        self._parentNode.appendChild(node)
        avg.WordsNode(parent=self._parentNode, id="curframe", pos=(10, 10), 
                font="arial", fontsize=10)
        avg.WordsNode(parent=self._parentNode, id="curtime", pos=(10, 22), 
                font="arial", fontsize=10)
        avg.WordsNode(parent=self._parentNode, id="framesqueued", pos=(10, 34), 
                font="arial", fontsize=10)

        g_player.setOnFrameHandler(onFrame)
    
if len(sys.argv) ==1:
    print "Usage: videoplayer.py <filename>"
    sys.exit(1)

node = avg.VideoNode(href=sys.argv[1], loop=True)
node.play()
VideoPlayer.start(resolution=node.getMediaSize())

