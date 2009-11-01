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

Player = avg.Player.get()

Player.loadString("""
<?xml version="1.0"?>
<!DOCTYPE avg SYSTEM "../../doc/avg.dtd">
<avg width="1280" height="720">
  <video id="video" x="0" y="0" threaded="true"/>
</avg>
""")
node = Player.getElementByID("video")
if len(sys.argv) ==1:
    print "Usage: videoinfo.py <filename>"
    sys.exit(1)
else:
    node.href=sys.argv[1]
node.play()

print "File: " + node.href
print ("Duration: " + str(node.getDuration()/1000.) + " s (" + str(node.getNumFrames()) 
        + " frames)")
print "Bitrate: " + str(node.getBitrate()) + " b/s"
print "Video stream: " 
print "  Codec: " + node.getVideoCodec()
print "  Size: " + str(node.getMediaSize()) + " pixels"
print "  Pixel format: " + node.getStreamPixelFormat()
print "  FPS: " + str(node.fps)
if node.hasAudio():
    print "Audio stream: " 
    print "  Codec: " + node.getAudioCodec()
    print "  Sample rate: " + str(node.getAudioSampleRate()) + " Hz"
    print "  Number of channels: " + str(node.getNumAudioChannels())
