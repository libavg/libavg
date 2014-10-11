#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import app, avg

class CaptionNode(avg.WordsNode):
    def __init__(self, pos, text, parent):
        pos = avg.Point2D(pos) + (32,70)
        super(CaptionNode, self).__init__(pos=pos, text=text, alignment="center",
                fontsize=11, aagamma=2.0, width=64)
        self.registerInstance(self, parent)

class RenderingDiv(app.MainDiv):
    def onInit(self):
        self.toggleTouchVisualization()
       
        avg.WordsNode(pos=(10,5), text="Masking", fontsize=13, aagamma=2.0, parent=self)
        avg.ImageNode(pos=(10,30), href="rgb24-64x64.png", parent=self)
        CaptionNode(pos=(10,30), text="Plain Image", parent=self)
        avg.ImageNode(pos=(90,30), href="rgb24-64x64.png", maskhref="mask.png", 
                parent=self)
        CaptionNode(pos=(90,30), text="Masked Image", parent=self)
  
        avg.WordsNode(pos=(10,155), text="Blend Modes", fontsize=13, aagamma=2.0,
                parent=self)
        for i, blendmode in enumerate(("blend", "add", "min", "max")):
            pos=avg.Point2D(10,180)+i*avg.Point2D(80,0)
            avg.ImageNode(pos=pos, href="mask.png", parent=self)
            avg.ImageNode(pos=pos, href="rgb24-64x64.png", blendmode=blendmode,
                    parent=self)
            CaptionNode(pos=pos, text=blendmode, parent=self)


app.App().run(RenderingDiv(), app_resolution='400x300')

