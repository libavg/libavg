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


class SpeedDiv(app.MainDiv):
    def onArgvParserCreated(self, parser):
        usage = '%prog [options]\n' \
                'Checks libavg performance by creating lots of nodes. ' \
                'Displays a frame time graph and executes for 20 secs.'
        parser.set_usage(usage)

        parser.add_option('--use-fx', '-f', dest='useFX',
                action='store_true', default=False,
                help='display everything using a NullFX to test FX overhead')
        parser.add_option('--video', '-i', dest='video',
                action='store_true', default=False,
                help='show videos instead of images')
        parser.add_option('--audio', '-a', dest='audio',
                action='store_true', default=False,
                help='when showing videos, use videos with an audio channel')
        parser.add_option('--create-nodes', '-c', dest='create',
                action='store_true', default=False,
                help='destroy and recreate all nodes every 400 ms')
        parser.add_option('--move', '-m', dest='move',
                action='store_true', default=False,
                help='move nodes every frame')
        parser.add_option('--blur', '-b', dest='blur',
                action='store_true', default=False,
                help='apply a BlurFXNode to the nodes')
        parser.add_option('--color', '-o', dest='color',
                action='store_true', default=False,
                help='apply gamma to the nodes, causing the color correction shader to activate')
        parser.add_option('--vsync', '-s', dest='vsync',
                action='store_true', default=False,
                help='sync output to vertical refresh')
        parser.add_option('--num-objs', '-n', dest='numObjs',
                type='int', default=-1,
                help='number of objects to create [Default: 200 images or 40 videos]')
        parser.add_option('--profile', '-p', dest='profile',
                action='store_true', default=False,
                help='enable profiling output, note that profiling makes things slower')

    def onArgvParsed(self, options, args, parser):
        self.__optUseFX = options.useFX
        self.__optVideo = options.video
        self.__optAudio = options.audio
        self.__optCreate = options.create
        self.__optMove = options.move
        self.__optBlur = options.blur
        self.__optColor = options.color
        self.__optVsync = options.vsync
        self.__optNumObjs = options.numObjs
        if self.__optNumObjs < 1:
            if self.__optVideo:
                self.__optNumObjs = 40
            else:
                self.__optNumObjs = 200 

        log = avg.logger
        log.configureCategory(log.Category.CONFIG, log.Severity.DBG)
        if options.profile:
            log.configureCategory(log.Category.PROFILE, log.Severity.DBG)

    def onInit(self):
        if not self.__optVsync:
            player.setFramerate(1000)

        self.mediadir = utils.getMediaDir(None, 'data')
        self.__createNodes()
        app.instance.debugPanel.toggleWidget(app.debugpanel.FrametimeGraphWidget)
        if self.__optCreate:
            player.setInterval(400, self.__createNodes)
        # Ignore the first frame for the 20 sec-limit so long startup times don't
        # break things.
        player.setTimeout(0, lambda: player.setTimeout(20000, player.stop))

    def onFrame(self, dt):
         if self.__optMove:
            self.__moveNodes()

    def __createNodes(self):
        self.__nodes = []
        for i in xrange(self.__optNumObjs):
            pos = (random.randrange(800-64), random.randrange(600-64))
            if self.__optVideo:
                if self.__optAudio:
                    fname = "mpeg1-48x48-sound.avi"
                else:
                    fname = "mpeg1-48x48.mpg"
                node = avg.VideoNode(pos=pos, href=fname, loop=True, parent=self)
                node.play()
            else:
                node = avg.ImageNode(pos=pos, href="rgb24alpha-64x64.png", parent=self)
            if self.__optUseFX:
                node.setEffect(avg.NullFXNode())
            if self.__optBlur:
                node.setEffect(avg.BlurFXNode(10))
            if self.__optColor:
                node.gamma = (1.1, 1.1, 1.1)
            self.__nodes.append(node)
        if self.__optCreate:
            player.setTimeout(300, self.__deleteNodes)

    def __deleteNodes(self):
        for node in self.__nodes:
            node.unlink(True)
        self.__nodes = []

    def __moveNodes(self):
        for node in self.__nodes:
            node.pos = (random.randrange(800-64), random.randrange(600-64))


if __name__ == '__main__':
    app.App().run(SpeedDiv(), app_resolution='800x600')

