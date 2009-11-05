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

from optparse import OptionParser, OptionValueError
import sys
from libavg import avg
import time
from xml.dom import minidom
from xml import dom
import os


parser = OptionParser("usage: %prog <videofilename> [options]")
parser.add_option("-x", "--xml", dest = "xml", action="store_true",
        help = "Set to enable output as xml")
options, args = parser.parse_args()

Player = avg.Player.get()

Player.loadString("""
    <?xml version="1.0"?>
    <!DOCTYPE avg SYSTEM "../../doc/avg.dtd">
    <avg width="1280" height="720">
        <video id="video" x="0" y="0" threaded="true"/>
    </avg>
""")
node = Player.getElementByID("video")

def showInfo(node):    
    node.play()
    if options.xml:
        impl = minidom.getDOMImplementation()
        doc = impl.createDocument(None, "videoinfo", None)
        rootElement = doc.documentElement
        rootElement.setAttribute("file", node.href)
        rootElement.setAttribute("duration", str(node.getDuration()/1000.))
        rootElement.setAttribute("bitrate", str(node.getBitrate()))
        videoNode = doc.createElement("video")
        videoNode.setAttribute("codec", node.getVideoCodec())
        videoNode.setAttribute("size", str(node.getMediaSize()))
        videoNode.setAttribute("pixelformat", node.getStreamPixelFormat())
        videoNode.setAttribute("fps", str(node.fps))
        rootElement.appendChild(videoNode)
        if node.hasAudio():
            audioNode = doc.createElement("audio")
            audioNode.setAttribute("codec", node.getAudioCodec())
            audioNode.setAttribute("samplerate", str(node.getAudioSampleRate()))
            audioNode.setAttribute("channels", str(node.getNumAudioChannels()))
            rootElement.appendChild(audioNode)
        print doc.toprettyxml(indent="    ",encoding="utf-8")
    else:
        print "File: " + node.href
        print ("Duration: " + str(node.getDuration()/1000.) + " s (" 
                + str(node.getNumFrames()) + " frames)")
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
        print "\n\n"


if len(sys.argv) ==1:
    print "Usage: videoinfo.py <filename>"
    sys.exit(1)
elif os.path.isdir(sys.argv[1]):
    for subdir, dirs, files in os.walk(sys.argv[1]):
        for file in files:
            print subdir, file
            node.href = os.path.join(subdir, file)
            showInfo(node)
elif os.path.isfile(sys.argv[1]):
    node.href = sys.argv[1]
    showInfo(node)

        
