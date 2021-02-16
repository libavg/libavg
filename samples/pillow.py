#!/usr/bin/env python
# -*- coding: utf-8 -*-

# libavg - Media Playback Engine.
# Copyright (C) 2014-2021 Ulrich von Zadow
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

# Demonstrates interoperability with pillow (https://pillow.readthedocs.io)

from libavg import app, avg
from PIL import Image


class MyMainDiv(app.MainDiv):
    def onInit(self):
        self.toggleTouchVisualization()
        srcbmp = avg.Bitmap("rgb24-64x64.png")
        pixels = srcbmp.getPixels(False)
        image = Image.frombytes("RGBA", (64,64), pixels)
        # Need to swap red and blue.
        b,g,r,a = image.split()
        image = Image.merge("RGBA", (r,g,b,a))

        image.save("foo.png")

        destbmp = avg.Bitmap((64,64), avg.B8G8R8A8, "")
        destbmp.setPixels(image.tobytes())
        node = avg.ImageNode(parent=self)
        node.setBitmap(destbmp)

    def onExit(self):
        pass

    def onFrame(self):
        pass

app.App().run(MyMainDiv())


