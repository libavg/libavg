#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
from libavg import avg
import time

def printUsage():
    print
    print "Usage: avg_showcamera.py [<source> [<channel> [<width> <height> <pixelformat> <framerate>]]]"
    print "Valid values for source: firewire, v4l, directshow."
    print "Valid values for pixelformat: MONO8, MONO16, YUV411, YUV422, RGB, BGR, BY_GBRG"
    print

def checkCamera():
    if not(camNode.isAvailable()):
        Log.trace(Log.APP, "Could not open camera")
        printUsage()
        exit(1)

Log = avg.Logger.get();
Log.setCategories(Log.APP |
          Log.WARNING | 
#          Log.PROFILE |
#          Log.PROFILE_LATEFRAMES |
          Log.CONFIG |
#          Log.EVENTS |
          0)

Player = avg.Player.get()

Player.loadString("""
<?xml version="1.0"?>
<!DOCTYPE avg SYSTEM "../../doc/avg.dtd">
<avg width="1280" height="960">
</avg>
""")

source = "firewire"
device = ""
channel = ""
width = 640
height = 480
pixelFormat = "RGB"
framerate = 15

if len(sys.argv) > 1:
    source = sys.argv[1]
if len(sys.argv) > 2:
    channel = sys.argv[2]
if len(sys.argv) == 7:
    width = int(sys.argv[3])
    height = int(sys.argv[4])
    pixelFormat = sys.argv[5]
    framerate = float(sys.argv[6])

Log.trace(Log.APP, "Creating camera, source="+source+", channel="+channel+
        ", width="+str(width)+", height="+str(height)+", pixelformat="+pixelFormat+
        ", framerate="+str(framerate))

camNode = Player.createNode("camera", 
        {"source": source, "device": device, "channel": channel, "capturewidth": width, 
         "captureheight": height, "pixelformat": pixelFormat, "framerate": framerate,
         "width": 1280, "height":960})
Player.getRootNode().appendChild(camNode)
camNode.play()
Player.setVBlankFramerate(1)
Player.setTimeout(100, checkCamera)
Player.play()

