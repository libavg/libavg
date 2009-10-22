#!/usr/bin/python
# -*- coding: utf-8 -*-

import sys
from libavg import avg
import time

def resize():
    node = Player.getElementByID("video")
    sizeFactor = 1280.0/node.width
    node.width = 1280
    node.height *= sizeFactor

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
    else:
        print event.keystring

Log = avg.Logger.get();
Log.setCategories(Log.APP |
          Log.WARNING | 
          Log.PROFILE |
          Log.MEMORY |
          Log.CONFIG |
          Log.EVENTS)

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
Player.setOnFrameHandler(onFrame)
Player.setVBlankFramerate(1)
Player.play()

