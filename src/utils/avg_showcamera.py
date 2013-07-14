#!/usr/bin/env python
# -*- coding: utf-8 -*-

# libavg - Media Playback Engine.
# Copyright (C) 2003-2013 Ulrich von Zadow
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

import time
from libavg import avg, player
from libavg import parsecamargs
from libavg import app

usage = """%prog [options]

avg_showcamera.py shows the images captured by a camera attached to the
system. Its main use is to find out which parameters - device names,
image formats, framerates, etc. can be used with the camera(s)."""


class ShowCamera(app.MainDiv):
    def onArgvParserCreated(self, parser):
        parser.set_usage(usage)
        parsecamargs.addOptions(parser)
        parser.add_option("-l", "--list", dest="list",
                action="store_true", default=False,
                help="lists informations about detected cameras")
        parser.add_option("-s", "--noinfo", dest="noinfo",
                action="store_true", default=False,
                help="don't show any info overlayed on the screen")
        parser.add_option("-r", "--resetbus", dest="resetbus",
                action="store_true", default=False,
                help="reset the firewire bus")

    def onArgvParsed(self, options, args, parser):
        if options.list:
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

        if options.resetbus:
            avg.logger.info("Resetting firewire bus.")
            avg.CameraNode.resetFirewireBus()
            time.sleep(1)
            if not options.driver:
                exit(0)

        if options.driver is None and not options.list and not options.resetbus:
            parser.print_help()
            print
            print "ERROR: at least '--driver', '--list' or '--resetbus' options " \
                  "must be specified"
            exit(1)

        self.optdict = {}
        for attr in dir(options):
            if attr[0] != '_':
                self.optdict[attr] = eval("options.%s" %attr)

        self.settings.set("app_resolution", "%dx%d" %(options.width, options.height))

    def onInit(self):
        self.curFrame = 0

        avg.logger.info("Creating camera:")
        avg.logger.info("driver=%(driver)s device=%(device)s" %self.optdict)
        avg.logger.info( 
                "width=%(width)d height=%(height)d pixelformat=%(pixelFormat)s" 
                %self.optdict)
        avg.logger.info("unit=%(unit)d framerate=%(framerate)d fw800=%(fw800)s"
                %self.optdict)

        self.camNode = avg.CameraNode(driver=self.optdict["driver"],
                device=self.optdict["device"], unit=self.optdict["unit"],
                fw800=self.optdict["fw800"], framerate=self.optdict["framerate"],
                capturewidth=self.optdict["width"], captureheight=self.optdict["height"],
                pixelformat=self.optdict["pixelFormat"], parent=self)

        if not self.optdict["noinfo"]:
            self.infoText = ("Driver=%(driver)s (dev=%(device)s unit=%(unit)d) "
                    "%(width)dx%(height)d@%(framerate)f" %self.optdict)
            avg.WordsNode(text=self.infoText, color="ff3333", pos=(5,5), fontsize=14, 
                    rawtextmode=True, parent=self)
            self.frameText = avg.WordsNode(color="ff3333", pos=(5,25), fontsize=14,
                    parent=self)
        else:
            self.frameText = None

        self.setupKeys()

        self.camNode.play()
        player.setTimeout(100, self.checkCamera)

    def onFrame(self, dt):
        if self.frameText:
            self.curFrame += 1
            self.frameText.text = "%(cam)d/%(app)d" \
                    %{"cam":self.camNode.framenum, "app":self.curFrame}

    def checkCamera(self):
        if not(self.camNode.isAvailable()):
            avg.logger.error("Could not open camera")
            exit(1)

    def setupKeys(self):
        def setWhitebalance():
            print "Setting whitebalance"
            self.camNode.doOneShotWhitebalance()

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

        app.keyboardmanager.bindKeyDown(keystring="w",
                handler=setWhitebalance,
                help="Execute whitebalance")

        app.keyboardmanager.bindKeyDown(keystring="1",
                handler=lambda: addWhitebalance(du = -1),
                help="Decrease whitebalance u")
        app.keyboardmanager.bindKeyDown(keystring="2",
                handler=lambda: addWhitebalance(du = 1),
                help="Increase whitebalance u")
        app.keyboardmanager.bindKeyDown(keystring="3",
                handler=lambda: addWhitebalance(dv = -1),
                help="Decrease whitebalance v")
        app.keyboardmanager.bindKeyDown(keystring="4",
                handler=lambda: addWhitebalance(dv = 1),
                help="Increase whitebalance v")

        app.keyboardmanager.bindKeyDown(keystring="left",
                handler=lambda: addShutter(shutter = -1),
                help="Decrease shutter")
        app.keyboardmanager.bindKeyDown(keystring="right",
                handler=lambda: addShutter(shutter = 1),
                help="Increase shutter")

        app.keyboardmanager.bindKeyDown(keystring="up",
                handler=lambda: addGain(gain = 1),
                help="Increase gain")
        app.keyboardmanager.bindKeyDown(keystring="down",
                handler=lambda: addGain(gain = -1),
                help="Decrease gain")


if __name__ == "__main__":
    app.App().run(ShowCamera())

