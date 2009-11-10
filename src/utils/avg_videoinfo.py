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
import sys, traceback
from libavg import avg
import time
from xml.dom import minidom
from xml import dom
import os


parser = OptionParser("usage: %prog <videofilename> [options]")
parser.add_option("-x", "--xml", dest = "xml", action="store_true",
        help = "Set to enable output as xml")
parser.add_option("-c", "--csv", dest = "csv", action="store_true",
        help = "Set to enable output as csv")
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
impl = minidom.getDOMImplementation()
doc = impl.createDocument(None, "videodict", None)
rootElement = doc.documentElement
CSV_video = ''
VideoList = []


def appendXMLChild(node):    
    node.play()
    videoinfo = doc.createElement("videoinfo")
    videoinfo.setAttribute("file", node.href)
    videoinfo.setAttribute("duration", str(node.getDuration()/1000.))
    videoinfo.setAttribute("bitrate", str(node.getBitrate()))
    rootElement.appendChild(videoinfo)
    videoNode = doc.createElement("video")
    videoNode.setAttribute("codec", node.getVideoCodec())
    videoNode.setAttribute("size", str(node.getMediaSize()))
    videoNode.setAttribute("pixelformat", node.getStreamPixelFormat())
    videoNode.setAttribute("fps", str(node.fps))
    videoinfo.appendChild(videoNode)
    if node.hasAudio():
        audioNode = doc.createElement("audio")
        audioNode.setAttribute("codec", node.getAudioCodec())
        audioNode.setAttribute("samplerate", str(node.getAudioSampleRate()))
        audioNode.setAttribute("channels", str(node.getNumAudioChannels()))
        videoinfo.appendChild(audioNode)

def singleVideoInfo(node):
    node.play()
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

    
def showInfo(node):    
    node.play()
    if node.hasAudio():
        print "%-24s %-12s %-15s %-11s %-11s %-14s %-10f %-10s %-12s %-15s" % (node.href[-20:], str(node.getDuration()/1000.), str(node.getBitrate()), node.getVideoCodec(), str(node.getMediaSize()), node.getStreamPixelFormat(), node.fps, node.getAudioCodec(), str(node.getAudioSampleRate()), str(node.getNumAudioChannels()))
    else: 
        print "%-24s %-12s %-15s %-11s %-11s %-14s %-10f %-10s %-12s %-15s" % (node.href[-20:], str(node.getDuration()/1000.), str(node.getBitrate()), node.getVideoCodec(), str(node.getMediaSize()), node.getStreamPixelFormat(), node.fps, "", "", "" )

  
def CSVtable(node):
    global CSV_video
    
    node.play()
    if CSV_video == '':
        CSV_head = ("File\tDuration\tBitrate\tVideoCodec\t" + \
                "VideoSize\tPixel format\tFPS\tAudioCodec\t" + \
                "Audio Sample rate\t Audio channels"+"\n")
        CSV_video = CSV_head   
    CSV_video += \
        str(node.href)+'\t'+\
        str(node.getDuration()/1000.) + " s (" + str(node.getNumFrames()) + \
            " frames)"+'\t'+\
        str(node.getBitrate()) + " b/s" + '\t' + \
        str(node.getVideoCodec()) + '\t' + \
        str(node.getMediaSize()) + " pixels" + '\t' + \
        str(node.getStreamPixelFormat()) + '\t' + \
        str(node.fps) + '\t'
    if node.hasAudio():
        CSV_video += \
            str(node.getAudioCodec()) + '\t' + \
            str(node.getAudioSampleRate()) + " Hz" + '\t' + \
            str(node.getNumAudioChannels()) + '\n'
    else:
        CSV_video += ' \t \t \n'
    
    
if len(sys.argv) ==1:
    parser.print_help()
    sys.exit(1)

elif os.path.isdir(sys.argv[1]):
    nameLength = 0
    for subdir, dirs, files in os.walk(sys.argv[1]):
        for file in files:
            try:
                node.href = os.path.join(subdir, file)
                node.play()
                VideoList.append(node.href)
                if len(node.href) > nameLength:
                    nameLength = len(node.href)
            except:
                sys.stderr.write("Error in getting Videoinfo: " + str(node.href) + "\n")
    
    for i in xrange(0, len(VideoList)):
        node.href = str(VideoList[i])
        if options.xml:
            appendXMLChild(node)
        elif options.csv:
            CSVtable(node)
        else:
            if i == 0:
                title = ("File","Duration[s]","Bitrate [b/s]","VideoCodec","VideoSize","Pixel format","FPS","AudioCodec","Sample rate", "Channels")
                print "%-24s %-12s %-15s %-11s %-11s %-14s %-10s %-10s %-12s %-15s" %title
                showInfo(node)
            else:    
                showInfo(node)
    if options.xml:
        print doc.toprettyxml(indent="    ",encoding="utf-8")
        
    elif options.csv:
        print CSV_video
    
    
elif os.path.isfile(sys.argv[1]):
    node.href = sys.argv[1]
    if options.xml:
        appendXMLChild(node)
        print doc.toprettyxml(indent="    ",encoding="utf-8")
    elif options.csv:
        CSVtable(node)
        print CSV_video
    else:
        singleVideoInfo(node)
        
