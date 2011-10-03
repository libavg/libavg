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

g_Player = avg.Player.get()

def parseCmdLine():
    parser = optparse.OptionParser(usage=
"""%prog [option]. 
Checks libavg performance by creating lots of nodes. Displays a frame time graph, executes for 20 secs and dumps profile statistics at the end of program execution.""")
    parser.add_option('--use-fx', '-f', dest='useFX', action='store_true', default=False, 
            help='Display everything using a NullFX to test FX overhead.')
    parser.add_option('--video', '-v', dest='video',  action='store_true', default=False, 
            help='Show videos instead of images.')
    parser.add_option('--create-nodes', '-c', dest='createNodes', action='store_true',
            default=False, 
            help='Destroy and recreate all nodes every 400 ms.')
    parser.add_option('--vsync', '-s', dest='vsync', action='store_true',
            default=False, 
            help='Sync output to vertical refresh.')
    parser.add_option('--num-objs', '-n', dest='numObjs', type='int', default=-1,
            help='Number of objects to create. Default is 200 images or 40 videos.')

    (options, args) = parser.parse_args()

    return options

class SpeedApp(AVGApp):
    def init(self):
        self.__createNodes()
        self._starter.showFrameRate()
        if options.createNodes:
            g_Player.setInterval(400, self.__createNodes)
    
    def __createNodes(self):
        self.__nodes = []
        for i in xrange(options.numObjs):
            pos = (random.randrange(800-64), random.randrange(600-64))
            if options.video:
                node = avg.VideoNode(pos=pos, href="../video/testfiles/mpeg1-48x48.mpg",
                        loop=True, parent=self._parentNode)
                node.play()
            else:
                node = avg.ImageNode(pos=pos, href="rgb24alpha-64x64.png",
                        parent=self._parentNode)
            if options.useFX:
                node.setEffect(avg.NullFXNode())
            self.__nodes.append(node)
        if options.createNodes:
            g_Player.setTimeout(300, self.__deleteNodes)

    def __deleteNodes(self):
        for node in self.__nodes:
            node.unlink(True)
        self.__nodes = []

options = parseCmdLine()
if not(options.vsync):
    g_Player.setFramerate(1000)
if options.numObjs == -1:
    if options.video:
        options.numObjs = 40
    else:
        options.numObjs = 200 
       
log = avg.Logger.get()
log.setCategories(log.PROFILE | log.CONFIG | log.WARNING | log.ERROR)
SpeedApp.start(resolution=(800,600))

