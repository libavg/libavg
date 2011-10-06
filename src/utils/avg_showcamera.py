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
from libavg import avg
from libavg import parsecamargs

def checkCamera():
    if not(camNode.isAvailable()):
        Log.trace(Log.APP, "Could not open camera")
        exit(1)


def onKey(event):
    def addWhitebalance(du = 0, dv = 0):
        camNode.setWhitebalance(camNode.getWhitebalanceU() + du, 
                                camNode.getWhitebalanceV() + dv)
        print "u:", camNode.getWhitebalanceU(), "v:", camNode.getWhitebalanceV()
    
    def addGain(gain):
        camNode.gain += gain
        print "gain:", camNode.gain
    
    def addShutter(shutter):
        camNode.shutter += shutter
        print "shutter:", camNode.shutter
        
    if event.keystring == "w":
        print "Setting Whitebalance"
        camNode.doOneShotWhitebalance()
    
    elif event.keystring == "1":
        addWhitebalance(du = -1)
        
    elif event.keystring == "2":
        addWhitebalance(du = 1)
        
    elif event.keystring == "3":
        addWhitebalance(dv = -1)
        
    elif event.keystring == "4":
        addWhitebalance(dv = 1)
        
    elif event.keystring == "s":
        print "Saving camera image to camimage.png" 
        camNode.getBitmap().save("camimage.png")
    
    elif event.keystring == "left":
       addShutter(shutter = -1)
        
    elif event.keystring == "right":
        addShutter(shutter = 1)
    
    elif event.keystring == "up":
        addGain(gain = 1)
    
    elif event.keystring == "down":
        addGain(gain = -1)
    
curFrame = 0

def updateFrameDisplay(node):
    global curFrame
    curFrame += 1
    node.text = "%(cam)d/%(player)d"%{"cam":camNode.framenum, "player":curFrame}

parser = optparse.OptionParser()
parsecamargs.addOptions(parser)
parser.add_option("-l", "--dump", dest="dump", action="store_true", default=False,
          help="dump a list of detected cameras")
parser.add_option("-s", "--noinfo", dest="noinfo", action="store_true", default=False,
          help="don't show any info overlayed on the screen")
parser.add_option("-r", "--resetbus", dest="resetbus", action="store_true", default=False,
          help="reset the firewire bus.")

(options, args) = parser.parse_args()

if options.driver is None and not options.dump and not options.resetbus:
    parser.print_help()
    print
    print "Keys available when image is being displayed:"
    print "  w: Execute whitebalance."
    print "  1/2: Decrease/Increase whitebalance u."
    print "  3/4: Decrease/Increase whitebalance v."
    print "  s: Take screenshot."
    print
    print "ERROR: at least '--driver', '--dump' or '--resetbus' options must be specified"
    exit()

optdict = {}
for attr in dir(options):
    if attr[0] != '_':
        optdict[attr] = eval("options.%s" %attr)

Log = avg.Logger.get()

Player = avg.Player.get()

Player.loadString("""
<?xml version="1.0"?>
<!DOCTYPE avg SYSTEM "../../doc/avg.dtd">
<avg width="%(width)d" height="%(height)d">
</avg>
""" %optdict)
Player.getRootNode().setEventHandler(avg.KEYDOWN, avg.NONE, onKey)

if options.dump:
    avg.CameraNode.dumpCameras()
    exit(0)

if options.resetbus:
    Log.trace(Log.APP, "Resetting firewire bus.")
    avg.CameraNode.resetFirewireBus()
    time.sleep(1)
    if not options.driver:
        exit(0)

Log.trace(Log.APP, "Creating camera:")
Log.trace(Log.APP, "driver=%(driver)s device=%(device)s" %optdict)
Log.trace(Log.APP, "width=%(width)d height=%(height)d pixelformat=%(pixelFormat)s" 
        %optdict)
Log.trace(Log.APP, "unit=%(unit)d framerate=%(framerate)d fw800=%(fw800)s" %optdict)
    
camNode = Player.createNode("camera", 
        {"driver": options.driver, "device": options.device, "unit": options.unit, 
         "fw800": options.fw800, 
         "capturewidth": options.width, "captureheight": options.height, 
         "pixelformat": options.pixelFormat, "framerate": options.framerate, 
         "width": options.width, "height": options.height})

Player.getRootNode().appendChild(camNode)

if not options.noinfo:
    infoText = "Driver=%(driver)s (dev=%(device)s unit=%(unit)d) %(width)dx%(height)d@%(framerate)f" %optdict
    avg.WordsNode(text=infoText, color="ff3333", pos=(5,5), fontsize=14,
            parent=Player.getRootNode())
    frameText = avg.WordsNode(color="ff3333", pos=(5,25), fontsize=14,
            parent=Player.getRootNode())
    Player.setOnFrameHandler(lambda:updateFrameDisplay(frameText))

camNode.play()
Player.setTimeout(100, checkCamera)
Player.play()

