#!/usr/bin/env python
# -*- coding: utf-8 -*-

# libavg - Media Playback Engine.
# Copyright (C) 2003-2014 Ulrich von Zadow
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

from libavg import *

import random
import time

g_Trigger = True


class SpeedDiv(app.MainDiv):
    def onArgvParserCreated(self, parser):
        usage = '%prog [options]\n' \
                'Checks libavg performance by creating lots of polygon nodes. ' \
                'Displays a frame time graph and executes for 20 secs.'
        parser.set_usage(usage)

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
        parser.add_option('--radius', '-r', dest='radius',
                type='int', default=10,
                help='number of points in each polygon [Default: 10]')
        parser.add_option('--profile', '-p', dest='profile',
                action='store_true', default=False,
                help='enable profiling output, note that profiling makes things slower')

    def onArgvParsed(self, options, args, parser):
        self.__optCreate = options.create
        self.__optMove = options.move
        self.__optVsync = options.vsync
        self.__optNumObjs = options.numObjs
        if self.__optNumObjs < 1:
            self.__optNumObjs = 40
        self.__optRadius = options.radius

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
            node = avg.CircleNode(parent=self, pos=pos, r=self.__optRadius, fillopacity=0)
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
            if g_Trigger:
                node.pos += (1,1)
            else:
                node.pos -= (1,1)
        g_Trigger = not g_Trigger


if __name__ == '__main__':
    app.App().run(SpeedDiv(), app_resolution='800x600')

