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

import optparse
import time
from libavg import avg, player
from libavg import parsecamargs
from libavg import AVGApp

g_Log = avg.Logger.get()

usage = """%prog [options]

avg_showcamera.py shows the images captured by a camera attached to the
system. Its main use is to find out which parameters - device names,
image formats, framerates, etc. can be used with the camera(s)."""

parser = optparse.OptionParser(usage=usage)
parsecamargs.addOptions(parser)
parser.add_option("-l", "--list", dest="list", action="store_true", default=False,
          help="lists informations about detected cameras")
parser.add_option("-s", "--noinfo", dest="noinfo", action="store_true", default=False,
          help="don't show any info overlayed on the screen")
parser.add_option("-r", "--resetbus", dest="resetbus", action="store_true", default=False,
          help="reset the firewire bus.")

(g_options, g_args) = parser.parse_args()

if g_options.list:
    infoList = list()
    infoList = avg.CameraNode.getCamerasInfos()
    if (len(infoList) <= 0):
        print "No camera available!"
    for info in infoList:
        print ""
        print "##################",info.driver,"##################"
        print "Device ID:", info.deviceID
        print ""
        print "----------------- FORMATS ------------------"
        formatsList = list()
        formatsList = info.imageFormats
        for format in formatsList:
            print "++++"
            print "Pixelformat:", format.pixelFormat
            print "Resolution:", format.size
            print "Framerates: |",
            framerateList = list()
            framerateList = format.framerates
            for framerate in framerateList:
                print framerate, "|",
            print ""
        print ""
        print "----------------- CONTROLS -----------------"
        controlsList = list()
        controlsList = info.controls
        for control in controlsList:
            print "++++", control.controlName
            print "Min:" , control.min, "| Max:", control.max,
            print "| Default:", control.default
        print ""
    exit(0)

if g_options.resetbus:
    g_Log.trace(g_Log.APP, "Resetting firewire bus.")
    avg.CameraNode.resetFirewireBus()
    time.sleep(1)
    if not g_options.driver:
        exit(0)

if g_options.driver is None and not g_options.list and not g_options.resetbus:
    parser.print_help()
    print
    print "Keys available when image is being displayed:"
    print "  w: Execute whitebalance."
    print "  1/2: Decrease/Increase whitebalance u."
    print "  3/4: Decrease/Increase whitebalance v."
    print "  s: Take screenshot."
    print
    print "ERROR: at least '--driver', '--list' or '--resetbus' options must be specified"
    exit()

class ShowCamera(AVGApp):
    def init(self):
        self.curFrame = 0
        global g_options

        self.optdict = {}
        for attr in dir(g_options):
            if attr[0] != '_':
                self.optdict[attr] = eval("g_options.%s" %attr)

        g_Log.trace(g_Log.APP, "Creating camera:")
        g_Log.trace(g_Log.APP, "driver=%(driver)s device=%(device)s" %self.optdict)
        g_Log.trace(g_Log.APP,
                "width=%(width)d height=%(height)d pixelformat=%(pixelFormat)s" 
                %self.optdict)
        g_Log.trace(g_Log.APP, "unit=%(unit)d framerate=%(framerate)d fw800=%(fw800)s"
                %self.optdict)
           
        self.camNode = avg.CameraNode(driver = g_options.driver,
                device = g_options.device, unit = g_options.unit, fw800 = g_options.fw800,
                framerate = g_options.framerate, capturewidth = g_options.width,
                captureheight=g_options.height, pixelformat= g_options.pixelFormat)

        player.getRootNode().appendChild(self.camNode)

        if not g_options.noinfo:
            self.infoText = ("Driver=%(driver)s (dev=%(device)s unit=%(unit)d) %(width)dx%(height)d@%(framerate)f"
                    %self.optdict)
            avg.WordsNode(text=self.infoText, color="ff3333", pos=(5,5), fontsize=14, 
                    rawtextmode=True, parent=player.getRootNode())
            frameText = avg.WordsNode(color="ff3333", pos=(5,25), fontsize=14,
                    parent=player.getRootNode())
            player.setOnFrameHandler(lambda:self.updateFrameDisplay(frameText))


    def _enter(self):
        self.camNode.play()
        player.setTimeout(100, self.checkCamera)

    def _leave(self):
        self.camNode.stop()

    def checkCamera(self):
        if not(self.camNode.isAvailable()):
            g_Log.trace(g_Log.APP, "Could not open camera")
            exit(1)

    def onKeyDown(self,event):
        def addWhitebalance(du = 0, dv = 0):
            self.camNode.setWhitebalance(self.camNode.getWhitebalanceU() + du, 
                    self.camNode.getWhitebalanceV() + dv)
            print ("u:", self.camNode.getWhitebalanceU(), "v:",
                    self.camNode.getWhitebalanceV())
        
        def addGain(gain):
            self.camNode.gain += gain
            print "gain:", self.camNode.gain
        
        def addShutter(shutter):
            self.camNode.shutter += shutter
            print "shutter:", self.camNode.shutter
            
        if event.keystring == "w":
            print "Setting Whitebalance"
            self.camNode.doOneShotWhitebalance()
        
        elif event.keystring == "1":
            addWhitebalance(du = -1)
            
        elif event.keystring == "2":
            addWhitebalance(du = 1)
            
        elif event.keystring == "3":
            addWhitebalance(dv = -1)
            
        elif event.keystring == "4":
            addWhitebalance(dv = 1)
        
        elif event.keystring == "left":
           addShutter(shutter = -1)
            
        elif event.keystring == "right":
            addShutter(shutter = 1)
        
        elif event.keystring == "up":
            addGain(gain = 1)
        
        elif event.keystring == "down":
            addGain(gain = -1)
        else:
            AVGApp.onKeyDown(self, event)

    def updateFrameDisplay(self, node):
        self.curFrame += 1
        node.text = "%(cam)d/%(app)d"%{"cam":self.camNode.framenum, "app":self.curFrame}
    

ShowCamera.start(resolution=(g_options.width, g_options.height))

