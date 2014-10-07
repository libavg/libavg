#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import app, avg
from PIL import Image

# Demonstrates interoperability with pillow (https://pillow.readthedocs.org/index.html)

class MyMainDiv(app.MainDiv):
    def onInit(self):
        self.toggleTouchVisualization()
        bmp = avg.Bitmap("rgb24-64x64.png")
        pixels = bmp.getPixels()
        image = Image.frombytes("RGBA", (64,64), pixels)
        # Need to swap red and blue.
        b,g,r,a = image.split()
        image = Image.merge("RGBA", (r,g,b,a))

        image.save("foo.jpg")

    def onExit(self):
        pass

    def onFrame(self):
        pass

app.App().run(MyMainDiv())


