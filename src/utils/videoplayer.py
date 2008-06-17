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


Log = avg.Logger.get();
Log.setCategories(Log.APP |
          Log.WARNING | 
          Log.PROFILE |
#          Log.PROFILE_LATEFRAMES |
          Log.CONFIG |
          Log.EVENTS)

Player = avg.Player()

Player.loadString("""
<?xml version="1.0"?>
<!DOCTYPE avg SYSTEM "../../doc/avg.dtd">
<avg width="1280" height="720">
  <video id="video" x="0" y="0" opacity="1" blendmode="blend" fps="25" threaded="true"/>
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
Player.setVBlankFramerate(1)
Player.play()

