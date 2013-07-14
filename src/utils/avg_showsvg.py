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
#

import sys

from libavg import avg, app


class ShowSVG(app.MainDiv):
    def onArgvParserCreated(self, parser):
        parser.set_usage("%prog [options] <svgFileName> <elementID>")

        parser.add_option("-s", "--size", dest="size",
                type="float", default=1.0,
                help="specify a factor for the size of the element [Default: 1.0]")
        parser.add_option("--save-image", dest="saveImage",
                action="store_true", default=False,
                help="save the image rendered to a png file")

    def onArgvParsed(self, options, args, parser):
        if len(args) != 2:
            parser.print_help()
            sys.exit(1)
        self._svgFName = args[0]
        self._svgID = args[1]
        self._size = options.size
        self._saveImage = options.saveImage

    def onInit(self):
        self.svg = avg.SVG(self._svgFName, True)
        img = self.svg.createImageNode(self._svgID, {"pos":(1,1), "parent":self},
                self._size)
        rect = avg.RectNode(fillcolor="808080", color="FFFFFF", fillopacity=1,
                pos=(0.5, 0.5), size=img.size+(1,1))
        self.insertChild(rect, 0)
        if self._saveImage:
            bmp = self.svg.renderElement(self._svgID, self._size)
            bmp.save(self._svgID+".png")


if __name__ == "__main__":
    app.App().run(ShowSVG(), app_resolution="1024x768")

