#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (C) 2009 Archimedes Solutions GmbH,
# Saarbrücker Str. 24b, Berlin, Germany
#
# This file contains proprietary source code and confidential
# information. Its contents may not be disclosed or distributed to
# third parties unless prior specific permission by Archimedes
# Solutions GmbH, Berlin, Germany is obtained in writing. This applies
# to copies made in any form and using any medium. It applies to
# partial as well as complete copies.

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

