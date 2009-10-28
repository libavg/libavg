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
from libavg import avg
import time

def resize():
    node = Player.getElementByID("video")
    sizeFactor = 1280.0/node.width
    node.width = 1280
    node.height *= sizeFactor

def dumpInfo():
    # Should display more info.
    node = Player.getElementByID("video")
    print "Audio track: ", node.hasAudio()

def onFrame():
    node = Player.getElementByID("video")
    curFrame = node.getCurFrame()
    numFrames = node.getNumFrames()
    Player.getElementByID("curframe").text = "Frame: %i/%i"%(curFrame, numFrames)
    curVideoTime = node.getCurTime()
    Player.getElementByID("curtime").text = "Time: "+str(curVideoTime/1000.0)
    framesQueued = node.getNumFramesQueued()
    Player.getElementByID("framesqueued").text = "Frames queued: "+str(framesQueued)

def onKey(event):
    node = Player.getElementByID("video")
    if event.keystring == "right":
        node.seekToTime(node.getCurTime()+10000)
    elif event.keystring == "up":
        node.volume += 0.1
        print "Volume: ", node.volume
    elif event.keystring == "down":
        node.volume -= 0.1
        print "Volume: ", node.volume
    else:
        print event.keystring

Log = avg.Logger.get();
Log.setCategories(Log.APP |
          Log.WARNING | 
          Log.PROFILE |
          Log.MEMORY |
          Log.CONFIG |
          0)

Player = avg.Player.get()

Player.loadString("""
<?xml version="1.0"?>
<!DOCTYPE avg SYSTEM "../../doc/avg.dtd">
<avg width="1280" height="720" onkeyup="onKey">
  <video id="video" x="0" y="0" threaded="true"/>
  <words id="curframe" x="10" y="10" font="arial" fontsize="10"/> 
  <words id="curtime" x="10" y="22" font="arial" fontsize="10"/> 
  <words id="framesqueued" x="10" y="34" font="arial" fontsize="10"/> 
</avg>
""")
node = Player.getElementByID("video")
if len(sys.argv) ==1:
    print "Usage: videoplayer.py <filename>"
    sys.exit(1)
else:
    node.href=sys.argv[1]
node.play()
Player.setTimeout(10, resize)
Player.setTimeout(10, dumpInfo)
Player.setOnFrameHandler(onFrame)
Player.setVBlankFramerate(1)
Player.play()

