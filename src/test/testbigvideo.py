#!/usr/bin/python
# -*- coding: utf-8 -*-
# libavg - Media Playback Engine.
# Copyright (C) 2003-2008 Ulrich von Zadow
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

import unittest

import sys, syslog
import avg

class BigVideoTestCase(unittest.TestCase):
    def test(self):
        def init():
            Player.getElementByID("video").play()
        def onFrame():
            Player.getElementByID("video").seekToFrame(self.__curFrame)
            self.__curFrame -= 3
            if self.__curFrame < 2:
                self.__curFrame = 200
        self.__curFrame = 200 
        Player.loadFile("videofmt.avg")
        Player.setFramerate(10)
        Player.setTimeout(10, init)
#        Player.setInterval(10, onFrame)
        Player.play()

Player = avg.Player.get()
Log = avg.Logger.get()
Log.setCategories(Log.APP |
            Log.WARNING | 
            Log.PROFILE |
#            Log.PROFILE_LATEFRAMES |
            Log.CONFIG |
            Log.MEMORY | 
#           Log.BLTS    |
            Log.EVENTS
            )
runner = unittest.TextTestRunner()
runner.run(BigVideoTestCase("test"))

