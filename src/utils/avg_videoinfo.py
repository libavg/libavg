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

from optparse import OptionParser
import sys
from libavg import avg
from xml.dom import minidom
import os

parser = OptionParser(usage="%prog <videofilename(s) folder(s)> [options]")
parser.add_option("-x", "--xml", dest = "xml", action = "store_true",
        help = "Output in XML format")
parser.add_option("-c", "--csv", dest = "csv", action = "store_true",
        help = "Output in csv format")
parser.add_option("-r", "--recursion", dest = "recursion", action = "store_true",
        help = "Input will be recursiv interpreted")
options, args = parser.parse_args()

Player = avg.Player.get()

node = avg.VideoNode()
impl = minidom.getDOMImplementation()
doc = impl.createDocument(None, "videodict", None)
rootElement = doc.documentElement
CSV_video = ''
VideoList = []
len_filename = 8
len_videoCodec = 5
len_videoFormat = 6
len_audioCodec = 5

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
    videoNode.setAttribute("format", node.getStreamPixelFormat())
    videoNode.setAttribute("fps", str(node.fps))
    videoinfo.appendChild(videoNode)
    if node.hasAudio():
        audioNode = doc.createElement("audio")
        audioNode.setAttribute("codec", node.getAudioCodec())
        audioNode.setAttribute("samples", str(node.getAudioSampleRate()))
        audioNode.setAttribute("channels", str(node.getNumAudioChannels()))
        videoinfo.appendChild(audioNode)

def singleVideoInfo(node):
    print "File: " + node.href
    print ("Duration: " + str(node.getDuration()/1000.) + " s (" 
            + str(node.getNumFrames()) + " frames)")
    print "Bitrate: " + str(node.getBitrate()) + " b/s"
    print "Video stream: " 
    print "  Codec: " + node.getVideoCodec()
    print "  Size: " + str(node.getMediaSize()) + " pix"
    print "  Format: " + node.getStreamPixelFormat()
    print "  FPS: " + str(node.fps)
    print "  Duration: " + str(node.getVideoDuration()/1000.) + " s"
    if node.hasAudio():
        print "Audio stream: " 
        print "  Codec: " + node.getAudioCodec()
        print "  Samples: " + str(node.getAudioSampleRate()) + " Hz"
        print "  Channels: " + str(node.getNumAudioChannels())
        print "  Duration: " + str(node.getAudioDuration()/1000.) + " s"
  
def CSVtable(node):
    global CSV_video

    if CSV_video == '':
        CSV_head = ("File\tDuration\tBitrate\tVideoCodec\t" +
                "VideoSize\tPixel format\tFPS\tAudioCodec\t" + 
                "Audio Sample rate\t Audio channels"+"\n")
        CSV_video = CSV_head   
    CSV_video += (str(node.href)+'\t'+
        str(node.getDuration()/1000.) + " s (" + str(node.getNumFrames()) + 
            " frames)"+'\t'+
        str(node.getBitrate()) + " b/s" + '\t' +
        str(node.getVideoCodec()) + '\t' +
        str(node.getMediaSize()) + " pix" + '\t' +
        str(node.getStreamPixelFormat()) + '\t' +
        str(node.fps) + '\t')
    if node.hasAudio():
        CSV_video += (
            str(node.getAudioCodec()) + '\t' +
            str(node.getAudioSampleRate()) + " Hz" + '\t' +
            str(node.getNumAudioChannels()) + '\n')
    else:
        CSV_video += ' \t \t \n'
    
def showInfo(node):
    vFile = node.href.ljust(len_filename + 1)
    vDuration = str(node.getDuration()/1000.).ljust(12)
    vBitrate = str(node.getBitrate()).ljust(15) 
    vVideoCodec = str(node.getVideoCodec()).ljust(len_videoCodec + 1)
    vVideoSize = str(node.getMediaSize()).ljust(13)
    vPixel = str(node.getStreamPixelFormat()).ljust(len_videoFormat + 1)
    vFPS = str(round(node.fps, 2)).ljust(6)

    if node.hasAudio():   
        vAudioCodec = str(node.getAudioCodec()).ljust(len_audioCodec + 1)
        vSampleRate = str(node.getAudioSampleRate()).ljust(9)
        vChannels = str(node.getNumAudioChannels()).ljust(8)
    else:
        vAudioCodec = "no audio"
        vSampleRate = ""
        vChannels = ""
        
    info = (vFile + "| " + vDuration + vVideoCodec + vVideoSize + vPixel +
            vFPS + "| " + vAudioCodec + vSampleRate + vChannels)
    print info

def printHelp():
    parser.print_help()
    sys.exit(1)

def sortByName(a, b):
    if a < b:
        return -1
    else:
        return 1

def openFolder(folder):
    folderFiles = []
    newFiles = os.listdir(folder)
    for newfile in newFiles:
        if folder  == "..":
            testFile = "../"+newfile
        else: 
            testFile = str(folder)+"/"+newfile
        if os.path.isfile(testFile):
            folderFiles.append(testFile)
        if options.recursion and os.path.isdir(testFile):
            folderFiles.extend(openFolder(testFile))
    return folderFiles

def validPaths():
    filePaths = []
    for arg in args:
        if arg == ".":
            filePaths.extend(os.listdir(os.curdir))
            print os.listdir(os.curdir)
            if options.recursion:
                for folder in filePaths:
                    if os.path.isdir(folder):
                        filePaths.extend(openFolder(folder))
        elif arg == ".." or os.path.isdir(arg):
            filePaths.extend(openFolder(arg))
        else:
            if os.path.isfile(arg):
                filePaths.append(arg)

    for file in filePaths:
        try:
            if os.path.isfile(file):
                node.href = str(file)
                node.play()
                VideoList.append(node.href)   
        except:
            sys.stderr.write("\033[31mFile " + str(file) + " ignored: "
                    "Is no valid video file\n\033[m")
    VideoList.sort(cmp=sortByName)

if len(sys.argv) == 1:
    printHelp()

validPaths()

if len(VideoList) == 1:
    node.href = VideoList[0]
    if options.xml:
        appendXMLChild(node)
        print doc.toprettyxml(indent="    ",encoding="utf-8")
    elif options.csv:
        CSVtable(node)
        print CSV_video
    else:
        singleVideoInfo(node)
        
elif len(VideoList) > 0:
    for video in VideoList:
        node.href = str(video)
        curLen = len(video)
        if len_filename < curLen:
            len_filename = curLen
        curLen = len(node.getVideoCodec())
        if len_videoCodec < curLen:
            len_videoCodec = surLen
        curLen = len(node.getStreamPixelFormat())
        if len_videoFormat < curLen:
            len_videoFormat = curLen
        if node.hasAudio():
            curLen = len(node.getAudioCodec())
            if len_audioCodec < curLen:
                len_audioCodec = curLen
            
    for i in xrange(0, len(VideoList)):
        node.href = VideoList[i]
        if options.xml:
            appendXMLChild(node)
        elif options.csv:
            CSVtable(node)
        else:
            if i == 0:
                vFile = "Filename".ljust(len_filename+1)
                vDuration = "Duration[s]".ljust(12)
                vBitrate = "Bitrate [b/s]".ljust(15)
                vVideoCodec = "Codec".ljust(len_videoCodec +1)
                vVideoSize = "Size".ljust(13)
                vPixel = "Format".ljust(len_videoFormat+1)
                vFPS = "FPS".ljust(6)
                vAudioCodec = "Codec".ljust(len_audioCodec+1)
                vSampleRate = "Samples".ljust(9)
                vChannels = "Channels".ljust(8)
                
                headtitel = ("| ".rjust(len_filename + 3) +
                        "Video properties".rjust((len_videoFormat + len_videoCodec + 49)/ 2 ) +
                        "| ".rjust(17) +
                        "Audio properties".rjust((len_audioCodec + 18) / 2 + 8) )
                seperators = ("\n" + "| ".rjust(len_filename+3) +
                        "|".rjust((len_videoFormat + len_videoCodec + 49) / 2 + 16) )
                title = ("\n" + vFile + "| " + vDuration + vVideoCodec +
                        vVideoSize + vPixel + vFPS + "| " + vAudioCodec +
                        vSampleRate + vChannels )
                print headtitel,seperators,title,seperators
            showInfo(node)
    if options.xml:
        print doc.toprettyxml(indent="    ",encoding="utf-8")
        
    elif options.csv:
        print CSV_video
else:
    print "\033[31mNo valid video files found on this path(s).\033[m\n"
    printHelp()
