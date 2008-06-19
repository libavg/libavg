#!/usr/bin/python
# -*- coding: utf-8 -*-

import sys
from libavg import avg
import time

Log = avg.Logger.get();
Log.setCategories(Log.APP |
          Log.WARNING | 
#          Log.PROFILE |
#          Log.PROFILE_LATEFRAMES |
#          Log.CONFIG |
#          Log.EVENTS |
          0)

Player = avg.Player()

Player.loadString("""
<?xml version="1.0"?>
<!DOCTYPE avg SYSTEM "../../doc/avg.dtd">
<avg width="640" height="480">
</avg>
""")
if len(sys.argv) ==1:
    print "Usage: showfont.py <fontname> [<text>]"
    sys.exit(1)
else:
    fontname=sys.argv[1]
    if len(sys.argv) > 2:
        displayText=sys.argv[2]
    else:
        displayText=""
variants = avg.Words.getFontVariants(fontname)
print variants
rootNode = Player.getRootNode()
y = 10
for variant in variants:
    if displayText == "":
        text = fontname+": "+variant
    else:
        text = displayText
    node = Player.createNode("words", 
            { "text": text,
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


