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

parser = OptionParser(usage="%prog filename(s) [options]")
parser.add_option("-x", "--xml", dest = "xml", action = "store_true",
        help = "Output in XML format")
parser.add_option("-c", "--csv", dest = "csv", action = "store_true",
        help = "Output in csv format")
parser.add_option("-r", "--recursion", dest = "recursion", action = "store_true",
        help = "Recurse into subdirectories")
options, args = parser.parse_args()

def sortByName(a, b):
    if a < b:
        return -1
    else:
        return 1

class OutputHandler(object):
    
    def __init__(self, args):
        self._node = avg.VideoNode()
        self.__getFileNames(args)

    def __getFileNames(self, args):
        self._fileNameList = []
        filePaths = []
        for arg in args:
            if arg == ".":
                filePaths.extend(os.listdir(os.curdir))
                if options.recursion:
                    for folder in filePaths:
                        if os.path.isdir(folder):
                            filePaths.extend(self.__getFilesInFolder(folder))
            elif arg == ".." or os.path.isdir(arg):
                filePaths.extend(self.__getFilesInFolder(arg))
            else:
                if os.path.isfile(arg):
                    filePaths.append(arg)

        for file in filePaths:
            try:
                if os.path.isfile(file):
                    self._node.href = str(file)
                    self._node.play()
                    self._fileNameList.append(self._node.href)   
            except RuntimeError, err:
                sys.stderr.write(str(err) + "\n")
                self._node = avg.VideoNode()
        self._fileNameList.sort(cmp=sortByName)

    def __getFilesInFolder(self, folder):
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
                folderFiles.extend(self.__getFilesInFolder(testFile))
        return folderFiles


class ConsoleOutputHandler(OutputHandler):
    
    def __init__(self, args):
        super(ConsoleOutputHandler, self).__init__(args)
        if self._fileNameList == []:
            print "No valid video files found."
            printHelp()

    def output(self):
        if len(self._fileNameList) == 1:
            self.__outputSingleFile(self._fileNameList[0])
        else:
            self.__outputTable()

    def __outputSingleFile(self, filename):
        self._node.href = filename
        print "File: " + self._node.href
        print ("Duration: " + str(self._node.getDuration()/1000.) + " s (" 
                + str(self._node.getNumFrames()) + " frames)")
        print "Bitrate: " + str(self._node.getBitrate()) + " b/s"
        print "Video stream: " 
        print "  Codec: " + self._node.getVideoCodec()
        print "  Size: " + str(self._node.getMediaSize())
        print "  Pixel format: " + self._node.getStreamPixelFormat()
        print "  FPS: " + str(self._node.fps)
        print "  Duration: " + str(self._node.getVideoDuration()/1000.) + " s"
        if self._node.hasAudio():
            print "Audio stream: " 
            print "  Codec: " + self._node.getAudioCodec()
            print "  Sample rate: " + str(self._node.getAudioSampleRate()) + " Hz"
            print "  Channels: " + str(self._node.getNumAudioChannels())
            print "  Duration: " + str(self._node.getAudioDuration()/1000.) + " s"

    def __outputTable(self):
        self.__filenameLen = 8
        self.__videoCodecLen = 5
        self.__videoFormatLen = 6
        self.__audioCodecLen = 5

        for filename in self._fileNameList:
            self._node.href = filename
            self._node.play()
            self.__filenameLen = max(self.__filenameLen, len(filename))
            curLen = len(self._node.getVideoCodec())
            self.__videoCodecLen = max(self.__videoCodecLen, curLen)
            curLen = len(self._node.getStreamPixelFormat())
            self.__videoFormatLen = max(self.__videoFormatLen, curLen)
            if self._node.hasAudio():
                curLen = len(self._node.getAudioCodec())
                self.__audioCodecLen = max(self.__audioCodecLen, curLen)
        
        self.__outputTableHeader()

        for filename in self._fileNameList:
            self._node.href = filename
            self.__outputTableLine(self._node)

    def __outputTableHeader(self):
        vFile = "Filename".ljust(self.__filenameLen+1)
        vDuration = "Duration".ljust(9)
        vVideoCodec = "Codec".ljust(self.__videoCodecLen +1)
        vVideoSize = "Size".ljust(13)
        vPixel = "Pixels".ljust(self.__videoFormatLen+1)
        vFPS = "FPS".ljust(6)
        vAudioCodec = "Codec".ljust(self.__audioCodecLen+1)
        vSampleRate = "Rate".ljust(6)
        vChannels = "Channels".ljust(8)
       
        videoPropWidth = self.__videoFormatLen+self.__videoCodecLen+30
        print ("| ".rjust(self.__filenameLen + 3) +
                "Video properties".center(videoPropWidth) +
                "| " +
                "Audio properties".center((self.__audioCodecLen+17)))
        print (vFile + "| " + vDuration + vVideoCodec +
                vVideoSize + vPixel + vFPS + "| " + vAudioCodec +
                vSampleRate + vChannels )
        print ("| ".rjust(self.__filenameLen+3) +
                "|".rjust(videoPropWidth+1))

    def __outputTableLine(self, node):
        vFile = node.href.ljust(self.__filenameLen + 1)
        vDuration = str(node.getDuration()/1000.).ljust(9)
        vVideoCodec = str(node.getVideoCodec()).ljust(self.__videoCodecLen + 1)
        vVideoSize = str(node.getMediaSize()).ljust(13)
        vPixel = str(node.getStreamPixelFormat()).ljust(self.__videoFormatLen + 1)
        if node.fps%1 < 0.0000001:
            vFPS = str(int(node.fps)).ljust(6)
        else:
            vFPS = str(round(node.fps, 2)).ljust(6)

        if node.hasAudio():   
            vAudioCodec = str(node.getAudioCodec()).ljust(self.__audioCodecLen + 1)
            vSampleRate = str(node.getAudioSampleRate()).ljust(6)
            vChannels = str(node.getNumAudioChannels()).ljust(8)
        else:
            vAudioCodec = "no audio"
            vSampleRate = ""
            vChannels = ""
            
        info = (vFile + "| " + vDuration + vVideoCodec + vVideoSize + vPixel +
                vFPS + "| " + vAudioCodec + vSampleRate + vChannels)
        print info

class XMLOutputHandler(OutputHandler):
    
    def __init__(self, args):
        super(XMLOutputHandler, self).__init__(args)
        self.__impl = minidom.getDOMImplementation()
        self.__doc = self.__impl.createDocument(None, "videodict", None)
        self.__rootElement = self.__doc.documentElement

    def output(self):
        for filename in self._fileNameList:
            self._node.href = str(filename)
            self.__appendXMLChild(self._node)
        print self.__doc.toprettyxml(indent="    ",encoding="utf-8")
        
    def __appendXMLChild(self, node):
        node.play()
        videoinfo = self.__doc.createElement("videoinfo")
        videoinfo.setAttribute("file", node.href)
        videoinfo.setAttribute("duration", str(node.getDuration()/1000.))
        videoinfo.setAttribute("bitrate", str(node.getBitrate()))
        self.__rootElement.appendChild(videoinfo)
        videoNode = self.__doc.createElement("video")
        videoNode.setAttribute("codec", node.getVideoCodec())
        videoNode.setAttribute("size", str(node.getMediaSize()))
        videoNode.setAttribute("pixelformat", node.getStreamPixelFormat())
        videoNode.setAttribute("fps", str(node.fps))
        videoinfo.appendChild(videoNode)
        if node.hasAudio():
            audioNode = self.__doc.createElement("audio")
            audioNode.setAttribute("codec", node.getAudioCodec())
            audioNode.setAttribute("samplerate", str(node.getAudioSampleRate()))
            audioNode.setAttribute("channels", str(node.getNumAudioChannels()))
            videoinfo.appendChild(audioNode)

class CSVOutputHandler(OutputHandler):
    
    def __init__(self, args):
        super(CSVOutputHandler, self).__init__(args)

    def output(self):
        print ("File\tDuration(sec)\tNumber of Frames\tBitrate(b/s)\tVideo Codec\t" +
                "Width\tHeight\tPixel format\tFPS\tAudio Codec\t" + 
                "Audio Sample rate(Hz)\t Audio channels")
        for filename in self._fileNameList:
            self._node.href = str(filename)
            self.__outputNode(self._node)

    def __outputNode(self, node):
        s = (str(node.href)+'\t'+
            str(node.getDuration()/1000.)+'\t' +
            str(node.getNumFrames())+"\t" + 
            str(node.getBitrate()) + '\t' +
            str(node.getVideoCodec()) + '\t' +
            str(node.getMediaSize()[0]) + '\t' +
            str(node.getMediaSize()[1]) + '\t' +
            str(node.getStreamPixelFormat()) + '\t' +
            str(node.fps) + '\t')
        if node.hasAudio():
            s += (
                str(node.getAudioCodec()) + '\t' +
                str(node.getAudioSampleRate()) + '\t' +
                str(node.getNumAudioChannels()))
        else:
            s += ' \t \t'
        print s
    
def printHelp():
    parser.print_help()
    sys.exit(1)

if len(sys.argv) == 1:
    printHelp()

if options.xml:
    outputHandler = XMLOutputHandler(args)
elif options.csv:
    outputHandler = CSVOutputHandler(args)
else:
    outputHandler = ConsoleOutputHandler(args)
outputHandler.output()
