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

import math
from libavg import avg, Point2D, RasterNode

g_Player = avg.Player.get()

class TrackerImageFlipper:
    def __init__(self):
        self.readConfig()

    def readConfig(self):
        global g_tracker
        print "reading tracker config"
        g_tracker = g_Player.getTracker()
        trackerAngle = float(g_tracker.getParam('/transform/angle/@value'))
        self.angle = round(trackerAngle/math.pi) * math.pi
        self.flipX = 0 > float(g_tracker.getParam('/transform/displayscale/@x'))
        self.flipY = 0 > float(g_tracker.getParam('/transform/displayscale/@y'))

    def transformPos(self, (x, y)):
        if self.flipX:
            x = 1 - x
        if self.flipY:
            y = 1 - y
        return (x, y)

    def flipNode(self, node):
        node.angle = self.angle
        grid = node.getOrigVertexCoords()
        grid = [ [ self.transformPos(pos) for pos in line ] for line in grid]
        node.setWarpedVertexCoords(grid)

    def loadTrackerImage(self, node, imageID):
        fingerBitmap = g_tracker.getImage(imageID)
        node.setBitmap(fingerBitmap)
        self.flipNode(node)

