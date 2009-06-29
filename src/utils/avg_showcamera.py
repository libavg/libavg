#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
from libavg import avg
import time

def printUsage():
    print
    print "Usage: avg_showcamera.py [<driver> [<device> [<unit> [<width> <height> <pixelformat> <framerate> [<fw800>]]]]]"
    print "Valid values for driver: firewire, v4l, directshow."
    print "Valid values for pixelformat: I8, I16, YUV411, YUV422, RGB, BGR, BAYER8"
    print

def checkCamera():
    if not(camNode.isAvailable()):
        Log.trace(Log.APP, "Could not open camera")
        printUsage()
        exit(1)

Log = avg.Logger.get();
Log.setCategories(Log.APP |
          Log.WARNING | 
          Log.PROFILE |
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

driver = "firewire"
device = ""
unit = -1 
width = 640
height = 480
pixelFormat = "RGB"
framerate = 15
fw800 = False

if len(sys.argv) == 1:
    printUsage()
    avg.Camera.dumpCameras()
    exit(1)

if len(sys.argv) > 1:
    driver = sys.argv[1]
if len(sys.argv) > 2:
    device = sys.argv[2]
if len(sys.argv) > 3:
    unit = int(sys.argv[3])
if len(sys.argv) > 7:
    width = int(sys.argv[4])
    height = int(sys.argv[5])
    pixelFormat = sys.argv[6]
    framerate = float(sys.argv[7])
if len(sys.argv) == 9:
    fw800str = sys.argv[8].upper()
    if fw800str == "FALSE":
        fw800 = False
    else:
        fw800 = True

Log.trace(Log.APP, "Creating camera, driver="+driver+", device="+device+
        ", unit="+str(unit)+", width="+str(width)+", height="+str(height)+
        ", pixelformat="+pixelFormat+", framerate="+str(framerate)+", fw800="+str(fw800))

camNode = Player.createNode("camera", 
        {"driver": driver, "device": device, "unit": unit, "fw800": fw800,
         "capturewidth": width, "captureheight": height, "pixelformat": pixelFormat, 
         "framerate": framerate, "width": 1280, "height":960})
Player.getRootNode().appendChild(camNode)
camNode.play()
Player.setFramerate(framerate)
Player.setTimeout(100, checkCamera)
Player.play()

