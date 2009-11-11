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
    if node.hasAudio():
        print "Audio stream: " 
        print "  Codec: " + node.getAudioCodec()
        print "  Sample rate: " + str(node.getAudioSampleRate()) + " Hz"
        print "  Number of channels: " + str(node.getNumAudioChannels())
  
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
    
    info = "{File:%d}{Duration:12}{Bitrate:15}{VideoCodes:13}{VideoSize:11}{Pixel:14}{FPS:10}{AudioCodec:12}{SampleRate:13}{Channels:8}" % (len_filename+2)
    if node.hasAudio():
        print (info.format(
            File=str(os.path.basename(node.href)), 
            Duration=str(node.getDuration()/1000.), 
            Bitrate=str(node.getBitrate()), 
            VideoCodes=node.getVideoCodec(), 
            VideoSize=str(node.getMediaSize()), 
            Pixel=node.getStreamPixelFormat(), 
            FPS='%2.2f'%node.fps, 
            AudioCodec=node.getAudioCodec(), 
            SampleRate=str(node.getAudioSampleRate()), 
            Channels=str(node.getNumAudioChannels())))
    else: 
        print (info.format(
            File=str(os.path.basename(node.href)), 
            Duration=str(node.getDuration()/1000.), 
            Bitrate=str(node.getBitrate()), 
            VideoCodes=node.getVideoCodec(), 
            VideoSize=str(node.getMediaSize()), 
            Pixel=node.getStreamPixelFormat(), 
            FPS='%2.2f'%node.fps, 
            AudioCodec=' ', 
            SampleRate=' ', 
            Channels=' '))
        
    
            
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
            node.href = str(os.path.join(os.path.normpath(os.getcwd() + "/" +str(file))))
            node.play()
            VideoList.append(node.href)   
        except:
            sys.stderr.write("Error in getting Videoinfo: " + str(node.href) + "\n")
    
    for i in xrange(0, len(VideoList)):
        if len_filename < len(str(os.path.basename(str(VideoList[i])))):
            len_filename = len(str(os.path.basename(str(VideoList[i]))))
            
    for i in xrange(0, len(VideoList)):
        node.href = str(VideoList[i])
        if options.xml:
            appendXMLChild(node)
        elif options.csv:
            CSVtable(node)
        else:
            if i == 0:
                title = "{File:%d}{Duration:12}{Bitrate:15}{VideoCodes:13}{VideoSize:11}{Pixel:14}{FPS:10}{AudioCodec:12}{SampleRate:13}{Channels:8}" % (len_filename+2)
                print (title.format(
                    File="File",
                    Duration="Duration[s]",
                    Bitrate="Bitrate [b/s]",
                    VideoCodes="VideoCodec",
                    VideoSize="VideoSize",
                    Pixel="Pixel format", 
                    FPS="FPS", 
                    AudioCodec="AudioCodec", 
                    SampleRate="Sample rate", 
                    Channels="Channels"))
                print ""
                showInfo(node)
            else:    
                showInfo(node)
    if options.xml:
        print doc.toprettyxml(indent="    ",encoding="utf-8")
        
    elif options.csv:
        print CSV_video
    
    

        
