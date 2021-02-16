#!/usr/bin/env python
# -*- coding: utf-8 -*-

# libavg - Media Playback Engine.
# Copyright (C) 2015-2021 Ulrich von Zadow
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

from libavg import *

def startAnim():
    animObj.start()

def createColorBar(y, startColor, endColor):
    for i in xrange(11):
        color = avg.Color.mix(startColor, endColor, 1-(i/10.))
        avg.RectNode(pos=(i*20+1,y), size=(20,20), fillcolor=color, fillopacity=1.0,
                parent=rootNode)
    

canvas = player.createMainCanvas(size=(640,480))
rootNode = canvas.getRootNode()
createColorBar(1, avg.Color("00A0FF"), avg.Color("FFFF00"))
createColorBar(21, avg.Color("FFFFFF"), avg.Color("FF0000"))

player.play()

