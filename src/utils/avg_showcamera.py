#!/usr/bin/env python
# -*- coding: utf-8 -*-

import optparse
from libavg import avg

validPixFmt = ('I8', 'I16', 'YUV411', 'YUV422', 'RGB', 'BGR', 'BAYER8')
validDrivers = ('firewire', 'v4l', 'directshow')

def checkCamera():
    if not(camNode.isAvailable()):
        Log.trace(Log.APP, "Could not open camera")
        exit(1)


parser = optparse.OptionParser()
parser.add_option("-t", "--driver",
                  action="store", dest="driver", 
		  choices=validDrivers, help="camera drivers (one of: %s)" %', '.join(validDrivers))
parser.add_option("-d", "--device",
                  action="store", dest="device", default="",
                  help="camera device identifier (may be GUID or device path)")
parser.add_option("-u", "--unit", action="store", dest="unit", default="-1",
		  type="int", help="unit number")
parser.add_option("-w", "--width", dest="width", default="640", type="int",
		  help="capture width in pixels")
parser.add_option("-e", "--height", dest="height", default="480", type="int",
		  help="capture height in pixels")
parser.add_option("-p", "--pixformat", dest="pixelFormat", default="RGB",
	          choices=validPixFmt, help="pixel format (one of: %s)" %', '.join(validPixFmt))
parser.add_option("-f", "--framerate", dest="framerate", default="15", type="float",
		  help="capture frame rate")
parser.add_option("-8", "--fw800", dest="fw800", action="store_true", default=False,
		  help="set firewire bus speed to s800 (if applicable)")
parser.add_option("-l", "--dump", dest="dump", action="store_true", default=False,
		  help="dump a list of detected cameras")
parser.add_option("-s", "--noinfo", dest="noinfo", action="store_true", default=False,
		  help="Don't show any info overlayed on the screen")

(options, args) = parser.parse_args()

if options.driver is None:
    parser.print_help()
    print "\nERROR: at least '--driver' option must be specified"
    exit()

optdict = {}
for attr in dir(options):
    if attr[0] != '_':
	optdict[attr] = eval("options.%s" %attr)


Log = avg.Logger.get()
Log.setCategories(Log.APP |
          Log.WARNING | 
          Log.PROFILE |
#          Log.PROFILE_LATEFRAMES |
          Log.CONFIG |
#          Log.EVENTS |
          0)

Player = avg.Player.get()

Player.loadString("""
<?xml version="1.0"?>
<!DOCTYPE avg SYSTEM "../../doc/avg.dtd">
<avg width="%(width)d" height="%(height)d">
</avg>
""" %optdict)


if options.dump:
    avg.Camera.dumpCameras()
    exit(0)

Log.trace(Log.APP, "Creating camera:")
Log.trace(Log.APP, "driver=%(driver)s device=%(device)s" %optdict)
Log.trace(Log.APP, "width=%(width)d height=%(height)d pixelformat=%(pixelFormat)s" %optdict)
Log.trace(Log.APP, "unit=%(unit)d framerate=%(framerate)d fw800=%(fw800)s" %optdict)

camNode = Player.createNode("camera", 
        {"driver": options.driver, "device": options.device, "unit": options.unit, "fw800": options.fw800,
         "capturewidth": options.width, "captureheight": options.height, "pixelformat": options.pixelFormat, 
         "framerate": options.framerate, "width": options.width, "height": options.height})

Player.getRootNode().appendChild(camNode)

if not options.noinfo:
    infoText = "Driver=%(driver)s (dev=%(device)s unit=%(unit)d) %(width)dx%(height)d@%(framerate)f" %optdict
    infoNode = Player.createNode("words",
	{"text": infoText, "color": "ff3333", "pos": avg.Point2D(5,5), "fontsize": 14})

    Player.getRootNode().appendChild(infoNode)
    

camNode.play()
Player.setFramerate(options.framerate)
Player.setTimeout(100, checkCamera)
Player.play()

