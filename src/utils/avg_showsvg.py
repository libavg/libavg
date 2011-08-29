#!/usr/bin/env python
# -*- coding: utf-8 -*-
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
import sys

from libavg import avg, AVGApp

g_player = avg.Player.get()
g_log = avg.Logger.get()

parser = optparse.OptionParser("Usage: %prog <svgFileName> <elementID> [options]")
parser.add_option("-s", "--size", action="store", type="float", dest="size", default=1,
        help="Specify a factor for the size of the element.")
parser.add_option("--save-image", action="store_true", dest="saveImage", default=False,
        help="Save the image rendered to a png file.")

(options, args) = parser.parse_args()
if len(args) < 2:
    parser.print_help()
    sys.exit(1)
svgFName = args[0]
svgID = args[1]

class ShowSVG(AVGApp):
    def init(self):
        self.svg = avg.SVG(svgFName, True)
        img = self.svg.createImageNode(svgID, {"parent":self._parentNode}, 
                options.size)
        if options.saveImage:
            bmp = self.svg.renderElement(svgID, options.size)
            bmp.save(svgID+".png")

if __name__ == '__main__':
    ShowSVG.start(resolution=(1024, 768))

