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
len_filename = 0

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
    print "  Duration: " + str(node.getVideoDuration()/1000.) + " s"
    if node.hasAudio():
        print "Audio stream: " 
        print "  Codec: " + node.getAudioCodec()
        print "  Sample rate: " + str(node.getAudioSampleRate()) + " Hz"
        print "  Number of channels: " + str(node.getNumAudioChannels())
        print "  Duration: " + str(node.getAudioDuration()/1000.) + " s"
  
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
    
def showInfo(node):    
    global len_filename
    node.play()
    if node.hasAudio():
        vFile = os.path.basename(node.href).ljust(len_filename+2)
        vDuration = str(node.getDuration()/1000.).ljust(12)
        vBitrate = str(node.getBitrate()).ljust(15) 
        vVideoCodec = str(node.getVideoCodec()).ljust(13)
        vVideoSize = str(node.getMediaSize()).ljust(11)
        vPixel = str(node.getStreamPixelFormat()).ljust(14)
        vFPS = str(round(node.fps, 2)).ljust(10)
        vAudioCodec = str(node.getAudioCodec()).ljust(12)
        vSampleRate = str(node.getAudioSampleRate()).ljust(13)
        vChannels = str(node.getNumAudioChannels()).ljust(8)
        
        info = vFile + vDuration + vVideoCodec + vVideoSize + vPixel + vFPS + vAudioCodec + vSampleRate + vChannels
        
        print info
    else:
        vFile = os.path.basename(node.href).ljust(len_filename+2)
        vDuration = str(node.getDuration()/1000.).ljust(12)
        vBitrate = str(node.getBitrate()).ljust(15) 
        vVideoCodec = str(node.getVideoCodec()).ljust(13)
        vVideoSize = str(node.getMediaSize()).ljust(11)
        vPixel = str(node.getStreamPixelFormat()).ljust(14)
        vFPS = str(round(node.fps,2)).ljust(10)
        vAudioCodec = "".ljust(12)
        vSampleRate = "".ljust(13)
        vChannels = "".ljust(8)
        
        info = vFile + vDuration + vVideoCodec + vVideoSize + vPixel + vFPS + vAudioCodec + vSampleRate + vChannels
        
        print info
        
    
            
if len(sys.argv) ==1:
    parser.print_help()
    sys.exit(1)

if len(args) == 1:
    node.href = sys.argv[1]
    if options.xml:
        appendXMLChild(node)
        print doc.toprettyxml(indent="    ",encoding="utf-8")
    elif options.csv:
        CSVtable(node)
        print CSV_video
    else:
        singleVideoInfo(node)
        
else:
    for file in args: 
        try:
            node.href = str(os.path.abspath(str(file)))    
            #node.href = str(os.path.join(os.path.normpath(os.getcwd() + "/" +str(file))))
            node.play()
            VideoList.append(node.href)   
        except:
            sys.stderr.write("Error in getting Videoinfo: " + str(node.href) + "\n")
   
    for i in xrange(0, len(VideoList)):
        curLen = len(os.path.basename(VideoList[i]))
        if len_filename < curLen:
            len_filename = curLen
            
    for i in xrange(0, len(VideoList)):
        node.href = VideoList[i]
        if options.xml:
            appendXMLChild(node)
        elif options.csv:
            CSVtable(node)
        else:
            if i == 0:
                vFile = "File".ljust(len_filename+2)
                vDuration = "Duration[s]".ljust(12)
                vBitrate = "Bitrate [b/s]".ljust(15) 
                vVideoCodec = "VideoCodec".ljust(13)
                vVideoSize = "VideoSize".ljust(11)
                vPixel = "Pixel format".ljust(14)
                vFPS = "FPS".ljust(10)
                vAudioCodec = "AudioCodec".ljust(12)
                vSampleRate = "Sample rate".ljust(13)
                vChannels = "Channels".ljust(8)
                
                title = vFile + vDuration + vVideoCodec + vVideoSize + vPixel + vFPS + vAudioCodec + vSampleRate + vChannels
                
                print title
                
                showInfo(node)
            else:    
                showInfo(node)
    if options.xml:
        print doc.toprettyxml(indent="    ",encoding="utf-8")
        
    elif options.csv:
        print CSV_video
    
    

        
