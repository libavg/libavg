#!/usr/bin/python
# -*- coding: utf-8 -*-

from libavg import avg

import unittest
import sys, syslog

class CameraTestCase(unittest.TestCase):
    def __init__(self, testFuncName, bpp):
        self.__bpp = bpp;
        self.__testFuncName = testFuncName
        unittest.TestCase.__init__(self, testFuncName)
        print "-------- ", self.__testFuncName, " --------"
    def setUp(self):
        Player.setResolution(0, 0, 0, self.__bpp)
    def test(self):
        def setWhitebalance():
            self.__camera.whitebalance = 24407
        def resetWhitebalance():
            self.__camera.whitebalance = -1
        def changeBrightness():
            self.brightness += 10
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
        self.__camera.play()
        Player.getElementByID("camera1").play()
#        self.brightness = 0
#        Player.setInterval(200, changeBrightness)
#        Player.setTimeout(200, setWhitebalance)
#        Player.setTimeout(300, resetWhitebalance)
#        Player.setInterval(3000, stopPlayback)
        Player.setInterval(200, setStrobe)
        Player.play()

def playerTestSuite(bpp):
    suite = unittest.TestSuite()
    suite.addTest(CameraTestCase("test", bpp))
    return suite

Player = avg.Player()
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

