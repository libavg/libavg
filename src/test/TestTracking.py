#!/usr/bin/python
# -*- coding: utf-8 -*-

import avg

import unittest
import sys, syslog

class TrackingTestCase(unittest.TestCase):
    def __init__(self, testFuncName):
        self.__testFuncName = testFuncName
        unittest.TestCase.__init__(self, testFuncName)
        print "-------- ", self.__testFuncName, " --------"
    def onFrame(self):
        Bitmap = self.__tracker.getImage(avg.IMG_CAMERA)
        Player.getElementByID("camera").setBitmap(Bitmap)
        Bitmap = self.__tracker.getImage(avg.IMG_HISTORY)
        Player.getElementByID("history").setBitmap(Bitmap)
    def test(self):
        Player.loadFile("tracking.avg")
        Player.setFramerate(60)
        self.__tracker = Player.addTracker("", 60, "640x480_MONO8")
        Player.setInterval(1, self.onFrame)
        Player.setResolution(0, 640, 480, 24)
        Player.play()

def playerTestSuite():
    suite = unittest.TestSuite()
    suite.addTest(TrackingTestCase("test"))
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
runner.run(playerTestSuite())
