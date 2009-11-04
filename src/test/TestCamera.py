#!/usr/bin/env python
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

from libavg import avg

import unittest
import sys

class CameraTestCase(unittest.TestCase):
    def __init__(self, testFuncName, bpp):
        self.__bpp = bpp;
        self.__testFuncName = testFuncName
        unittest.TestCase.__init__(self, testFuncName)
        print "-------- ", self.__testFuncName, " --------"
    def setUp(self):
        Player.setResolution(0, 0, 0, self.__bpp)
    def test(self):
        def getWhitebalance():
            self.__wb = (self.__camera.getWhitebalanceU(), 
                    self.__camera.getWhitebalanceV())
            print self.__wb
        def setWhitebalance():
            self.__camera.setWhitebalance(0, 0);
        def resetWhitebalance():
            self.__camera.setWhitebalance(self.__wb[0], self.__wb[1])
        def changeBrightness():
            self.brightness += 10
#            print self.__camera.brightness
            self.__camera.brightness = self.brightness
        def stopPlayback():
            self.__camera.stop()
            Player.setTimeout(500, self.__camera.play)
        def setStrobe():
#            print self.__camera.strobeduration
            self.__camera.strobeduration += 10
        self.curFrame = 200
        Player.loadFile("camera.avg")
        Player.setFramerate(60)
        self.__camera = Player.getElementByID("camera")
        self.__camera.strobeduration = -1
        self.__camera.play()
        Player.getElementByID("camera1").play()
        self.brightness = 0
        Player.setInterval(200, changeBrightness)

        Player.setTimeout(200, getWhitebalance)
        Player.setTimeout(1200, setWhitebalance)
        Player.setTimeout(2200, resetWhitebalance)

        Player.setTimeout(300, stopPlayback)
#        Player.setInterval(200, setStrobe)
        Player.play()

def playerTestSuite(bpp):
    suite = unittest.TestSuite()
    suite.addTest(CameraTestCase("test", bpp))
    return suite

Player = avg.Player.get()
Log = avg.Logger.get()
Log.setCategories(Log.APP |
          Log.WARNING | 
          Log.PROFILE |
#          Log.PROFILE_LATEFRAMES |
          Log.CONFIG |
          Log.MEMORY  |
#          Log.BLTS    |
          Log.EVENTS)

runner = unittest.TextTestRunner()

if len(sys.argv) != 3:
    bpp = 24
else:
    bpp = int(sys.argv[1])
runner.run(playerTestSuite(bpp))

