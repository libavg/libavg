#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import app, avg
from PIL import Image

# Demonstrates interoperability with pillow (https://pillow.readthedocs.io)

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


