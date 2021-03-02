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

from libavg import app, avg

class CaptionNode(avg.WordsNode):
    def __init__(self, pos, text, parent):
        pos = avg.Point2D(pos) + (32,70)
        super().__init__(pos=pos, text=text, alignment="center",
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

