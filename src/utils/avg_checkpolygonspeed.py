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
# Original author of this file is Robert Parcus <betoparcus@gmail.com>
#

from libavg import *

import optparse
import random
import math
import time

g_Player = avg.Player.get()

R = 40.0
g_Trigger = True

def parseCmdLine():
    parser = optparse.OptionParser(usage=
"""%prog [option]. 
Checks libavg performance by creating lots of polygon nodes. Displays a frame time graph and executes for 20 secs.""")
    parser.add_option('--hole-polygon', '-y', dest='hole', action='store_true', default=False,
            help='Equipped polygon with one hole. Attention the number of points in a polygon will dublicated.')
    parser.add_option('--create-nodes', '-c', dest='createNodes', action='store_true',
            default=False, 
            help='Destroy and recreate all nodes every 400 ms.')
    parser.add_option('--move', '-m', dest='move', action='store_true',
            default=False, 
            help='Move nodes every frame.')
    parser.add_option('--vsync', '-s', dest='vsync', action='store_true',
            default=False, 
            help='Sync output to vertical refresh.')
    parser.add_option('--num-objs', '-n', dest='numObjs', type='int', default=-1,
            help='Number of objects to create. Default is 200 images or 40 videos.')
    parser.add_option('--num-points', '-x', dest='numPoints', type='int', default=-1,
            help='Number of points in each polygon. Default is 10. Only even Numbers.')
    parser.add_option('--profile', '-p', dest='profile', action='store_true',
            default=False,
            help='Enable profiling output. Note that profiling makes things slower.')

    (options, args) = parser.parse_args()

    return options


class SpeedApp(AVGApp):
    def init(self):
        self._parentNode.mediadir = utils.getMediaDir(None, "data")
        tstart = time.time()
        self.__createNodes()
        print 'Buildtime needed: %f' % (time.time()-tstart)
        self._starter.showFrameRate()
        if options.createNodes:
            g_Player.setInterval(400, self.__createNodes)
        # Ignore the first frame for the 20 sec-limit so long startup times don't
        # break things.
        g_Player.setTimeout(0, lambda: g_Player.setTimeout(20000, g_Player.stop))
        if options.move:
            g_Player.setOnFrameHandler(self.__moveNodes)

    def __createNodes(self):
        self.__nodes = []
        for i in xrange(options.numObjs):
            pos = (random.randrange(800-64), random.randrange(600-64))
            polyPos = self.__calPolyCords(pos, R)
            holes = []
            if options.hole:
                holes = (self.__calPolyCords(pos, R/2), )
            node = avg.PolygonNode(parent=self._parentNode, pos=polyPos, fillopacity=1,
                    holes=holes)
            self.__nodes.append(node)
        if options.createNodes:
            g_Player.setTimeout(300, self.__deleteNodes)

    def __deleteNodes(self):
        for node in self.__nodes:
            node.unlink(True)
        self.__nodes = []

    def __moveNodes(self):
        global g_Trigger
        for node in self.__nodes:            
            newPos = []
            if g_Trigger:
                newPos = [(i[0]+1, i[1]+1) for i in node.pos]
            else:
                newPos = [(i[0]-1, i[1]-1) for i in node.pos]
            node.pos = newPos
        g_Trigger = not g_Trigger
        

    def __calPolyCords(self, offset, r):
        r2 = r/2
        alpha = math.radians(360.0 / (options.numPoints/2))
        beta = alpha/2
        result = []
        for i in xrange(options.numPoints/2):
            result.append( (r*math.cos(i*alpha) + offset[0], r*math.sin(i*alpha) + offset[1]) )
            result.append( (r2*math.cos(i*alpha+beta) + offset[0], r2*math.sin(i*alpha+beta) + offset[1]) )
        return result        

options = parseCmdLine()
if not(options.vsync):
    g_Player.setFramerate(1000)
if options.numObjs == -1:
    options.numObjs = 40 

if options.numPoints < 10:
    options.numPoints = 10
elif options.numPoints % 2 != 0:
    options.numPoints -= 1

log = avg.Logger.get()
if options.profile:
    log.setCategories(log.PROFILE | log.CONFIG | log.WARNING | log.ERROR)
else:
    log.setCategories(log.CONFIG | log.WARNING | log.ERROR)

SpeedApp.start(resolution=(800,600))

