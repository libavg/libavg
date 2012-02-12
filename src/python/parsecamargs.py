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

from libavg import avg

validPixFmt = list();
for formatItem in avg.getSupportedPixelFormats():
    validPixFmt.append(formatItem);
validDrivers = ('firewire', 'video4linux', 'directshow')

def addOptions(parser):
    parser.add_option("-t", "--driver", action="store", dest="driver", 
                  choices=validDrivers, 
                  help="camera drivers (one of: %s)" %', '.join(validDrivers))
    parser.add_option("-d", "--device", action = "store", dest = "device", default = "",
                      help = "camera device identifier (may be GUID or device path)")
    parser.add_option("-u", "--unit", action="store", dest="unit", default="-1",
              type="int", help="unit number")
    parser.add_option("-w", "--width", dest="width", default="640", type="int",
              help="capture width in pixels")
    parser.add_option("-e", "--height", dest="height", default="480", type="int",
              help="capture height in pixels")
    parser.add_option("-p", "--pixformat", dest="pixelFormat", default="R8G8B8",
                  choices=validPixFmt, 
                  help="camera frame pixel format (one of: %s)" %', '.join(validPixFmt))
    parser.add_option("-f", "--framerate", dest="framerate", default="15", type="float",
              help="capture frame rate")
    parser.add_option("-8", "--fw800", dest="fw800", action="store_true", default=False,
              help="set firewire bus speed to s800 (if applicable)") 
