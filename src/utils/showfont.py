#!/usr/bin/python
# -*- coding: utf-8 -*-

import sys
from libavg import avg
import time

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
<avg width="640" height="480">
</avg>
""")
if len(sys.argv) ==1:
    print "Usage: showfont.py <fontname>"
    sys.exit(1)
else:
    fontname=sys.argv[1]
variants = avg.Words.getFontVariants(fontname)
print variants
rootNode = Player.getRootNode()
y = 10
for variant in variants:
    completeName = fontname+": "+variant
    node = Player.createNode("words", 
            { "text": completeName,
              "font": fontname,
              "variant": variant,
              "size": 24,
              "x": 10,
              "y": y
            })
    rootNode.appendChild(node)
    y += 50
Player.setVBlankFramerate(1)
Player.play()


