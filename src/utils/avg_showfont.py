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

import sys
from libavg import avg
import time

Player = avg.Player.get()

Player.loadString("""
<?xml version="1.0"?>
<!DOCTYPE avg SYSTEM "../../doc/avg.dtd">
<avg width="640" height="480">
</avg>
""")
if len(sys.argv) ==1:
    print "Available fonts: "
    fontList = avg.WordsNode.getFontFamilies()
    print fontList
    print
    print "Usage: showfont.py <fontname> [<text>]"
    print
    print "  Shows all available variants of a font. If <text> is given, displays the"
    print "  text. If <fontname> is not given, dumps a list of all fonts available."
    sys.exit(1)
else:
    fontname=sys.argv[1]
    if len(sys.argv) > 2:
        displayText=sys.argv[2]
    else:
        displayText=""
variants = avg.WordsNode.getFontVariants(fontname)
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
              "fontsize": 24,
              "x": 10,
              "y": y
            })
    rootNode.appendChild(node)
    y += 50
Player.setVBlankFramerate(1)
Player.play()


