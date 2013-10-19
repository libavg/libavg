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
# Original author of this file is Robert Parcus <betoparcus@gmail.com>

from libavg import *

import random
import math
import time

R = 40.0
g_Trigger = True


class SpeedDiv(app.MainDiv):
    def onArgvParserCreated(self, parser):
        usage = '%prog [options]\n' \
                'Checks libavg performance by creating lots of polygon nodes. ' \
                'Displays a frame time graph and executes for 20 secs.'
        parser.set_usage(usage)

        parser.add_option('--hole-polygon', '-y', dest='hole',
                action='store_true', default=False,
                help='generate polygons with holes')
        parser.add_option('--create-nodes', '-c', dest='create',
                action='store_true', default=False,
                help='destroy and recreate all nodes every 400 ms')
        parser.add_option('--move', '-m', dest='move',
                action='store_true', default=False,
                help='move nodes every frame')
        parser.add_option('--vsync', '-s', dest='vsync',
                action='store_true', default=False,
                help='sync output to vertical refresh')
        parser.add_option('--num-objs', '-n', dest='numObjs',
                type='int', default=40,
                help='number of polygons to create [Default: 40]')
        parser.add_option('--num-points', '-x', dest='numPoints',
                type='int', default=10,
                help='number of points in each polygon [Default: 10]')
        parser.add_option('--profile', '-p', dest='profile',
                action='store_true', default=False,
                help='enable profiling output, note that profiling makes things slower')

    def onArgvParsed(self, options, args, parser):
        self.__optHole = options.hole
        self.__optCreate = options.create
        self.__optMove = options.move
        self.__optVsync = options.vsync
        self.__optNumObjs = options.numObjs
        if self.__optNumObjs < 1:
            self.__optNumObjs = 40
        self.__optNumPoints = options.numPoints
        if self.__optNumPoints < 10:
            self.__optNumPoints = 10
        elif self.__optNumPoints % 2 != 0:
            self.__optNumPoints -= 1

        log = avg.logger
        log.configureCategory(log.Category.CONFIG, log.Severity.DBG)
        if options.profile:
            log.configureCategory(log.Category.PROFILE, log.Severity.DBG)

    def onInit(self):
        if not self.__optVsync:
            player.setFramerate(1000)

        tstart = time.time()
        self.__createNodes()
        print 'Time to create nodes: %f' % (time.time()-tstart)
        app.instance.debugPanel.toggleWidget(app.debugpanel.FrametimeGraphWidget)
        if self.__optCreate:
            player.setInterval(400, self.__createNodes)
        # Ignore the first frame for the 20 sec-limit so long startup times don't
        # break things.
        player.setTimeout(0, lambda: player.setTimeout(20000, player.stop))

    def onFrame(self):
         if self.__optMove:
            self.__moveNodes()

    def __createNodes(self):
        self.__nodes = []
        for i in xrange(self.__optNumObjs):
            pos = (random.randrange(800-64), random.randrange(600-64))
            polyPos = self.__calPolyCords(pos, R)
            holes = []
            if self.__optHole:
                holes = (self.__calPolyCords(pos, R/2), )
            node = avg.PolygonNode(parent=self, pos=polyPos, fillopacity=1, holes=holes)
            self.__nodes.append(node)
        if self.__optCreate:
            player.setTimeout(300, self.__deleteNodes)

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
        alpha = math.radians(360.0 / (self.__optNumPoints/2))
        beta = alpha/2
        result = []
        for i in xrange(self.__optNumPoints/2):
            result.append((r*math.cos(i*alpha) + offset[0],
                    r*math.sin(i*alpha) + offset[1]))
            result.append((r2*math.cos(i*alpha+beta) + offset[0],
                    r2*math.sin(i*alpha+beta) + offset[1]))
        return result


if __name__ == '__main__':
    app.App().run(SpeedDiv(), app_resolution='800x600')

