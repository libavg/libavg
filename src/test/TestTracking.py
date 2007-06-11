#!/usr/bin/python
# -*- coding: utf-8 -*-

from libavg import avg

import unittest
import sys, syslog

global ourTrackingTestCase

def onKeyUp():
    Event = Player.getCurEvent()
    ourTrackingTestCase.onKeyUp(Event)

class TrackingTestCase(unittest.TestCase):
    def __init__(self, testFuncName):
        self.__testFuncName = testFuncName
        unittest.TestCase.__init__(self, testFuncName)
        print "-------- ", self.__testFuncName, " --------"
        global ourTrackingTestCase
        ourTrackingTestCase = self
    def onKeyUp(self, Event):
        if Event.keystring == "a":
            self.__tracker.threshold += 1
            print "Threshold: ", self.__tracker.threshold
        elif Event.keystring == "y":
            self.__tracker.threshold -= 1
            print "Threshold: ", self.__tracker.threshold
        elif Event.keystring == "s":
            self.__tracker.brightness += 5
            print "Brightness: ", self.__tracker.brightness
        elif Event.keystring == "x":
            self.__tracker.brightness -= 5
            print "Brightness: ", self.__tracker.brightness
        elif Event.keystring == "d":
            self.__tracker.exposure += 1
            print "Exposure: ", self.__tracker.exposure
        elif Event.keystring == "c":
            self.__tracker.exposure -= 1
            print "Exposure: ", self.__tracker.exposure
        elif Event.keystring == "f":
            self.__tracker.shutter += 5
            print "Shutter: ", self.__tracker.shutter
        elif Event.keystring == "v":
            self.__tracker.shutter -= 5
            print "Shutter: ", self.__tracker.shutter
        elif Event.keystring == "g":
            self.__tracker.gain += 5
            print "Gain: ", self.__tracker.gain
        elif Event.keystring == "b":
            self.__tracker.gain -= 5
            print "Gain: ", self.__tracker.gain
        elif Event.keystring == "j":
            self.__tracker.trapezoid -= 0.01
            print "Trapezoid: ", self.__tracker.trapezoid
        elif Event.keystring == "m":
            self.__tracker.trapezoid += 0.01
            print "Trapezoid: ", self.__tracker.trapezoid
        elif Event.keystring == "h":
            self.__tracker.resetHistory()
            print "History reset"
        elif Event.keystring == "w":
            self.__tracker.saveConfig()
            print ("Tracker configuration saved.")
        elif Event.keystring == "e":
            self.__saveIndex += 1
            self.__tracker.getImage(avg.IMG_NOHISTORY).save("img"+str(self.__saveIndex)+".png")
            print ("Image saved.")
        elif Event.keystring == "r":
            self.__tracker.debug = True
            print ("Debug image generation enabled.")
    def updateBitmap(self, ImgName, ID):
        Bitmap = self.__tracker.getImage(ID)
        Node = Player.getElementByID(ImgName)
        Node.setBitmap(Bitmap)
        Node.width=Bitmap.getSize()[0]/2
        Node.height=Bitmap.getSize()[1]/2
    def onFrame(self):
        self.updateBitmap("camera", avg.IMG_CAMERA);
        self.updateBitmap("nohistory", avg.IMG_NOHISTORY);
        self.updateBitmap("histogram", avg.IMG_HISTOGRAM);
        self.updateBitmap("highpass", avg.IMG_HIGHPASS);
        self.updateBitmap("fingers", avg.IMG_FINGERS);
    def test(self):
        Player.loadFile("tracking.avg")
        Player.setFramerate(60)
#        Player.getTestHelper().useFakeCamera(True)
        self.__tracker = Player.addTracker("/dev/video1394/0", 30, "640x480_MONO8")
        Player.setInterval(1, self.onFrame)
#        Player.setResolution(0, 640, 480, 24)
        self.__saveIndex = 0
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
          Log.MEMORY
#          Log.EVENTS2 |
#          Log.BLTS    |
#          Log.EVENTS
          )

runner = unittest.TextTestRunner()
runner.run(playerTestSuite())
