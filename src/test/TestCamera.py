#!/usr/bin/python
# -*- coding: utf-8 -*-

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
#        self.brightness = 0
#        Player.setInterval(200, changeBrightness)

#        Player.setTimeout(200, getWhitebalance)
#        Player.setTimeout(1200, setWhitebalance)
#        Player.setTimeout(2200, resetWhitebalance)

#        Player.setInterval(3000, stopPlayback)
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

