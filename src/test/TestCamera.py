#!/usr/bin/python
# -*- coding: utf-8 -*-
import unittest

import sys, syslog
# TODO: set this path via configure or something similar.
sys.path.append('/usr/local/lib/python2.3/site-packages/libavg')
sys.path.append('/usr/local/lib/python2.4/site-packages/libavg')
import avg

class VideoTestCase(unittest.TestCase):
    def __init__(self, testFuncName, engine, bpp):
        self.__engine = engine
        self.__bpp = bpp;
        self.__testFuncName = testFuncName
        unittest.TestCase.__init__(self, testFuncName)
        print "-------- ", self.__testFuncName, " --------"
    def setUp(self):
        Player.setDisplayEngine(self.__engine)
        Player.setResolution(0, 0, 0, self.__bpp)
    def test(self):
        self.curFrame = 200
        Player.loadFile("camera.avg")
        Player.setFramerate(15)
        Player.getElementByID("camera").play()
        Player.play()



def playerTestSuite(engine, bpp):
    suite = unittest.TestSuite()
    suite.addTest(VideoTestCase("test", engine, bpp))
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
    print "Usage: TestCamera.py <display engine> <bpp>"
else:
    if sys.argv[1] == "OGL":
        engine = avg.OGL
    elif sys.argv[1] == "DFB":
        engine = avg.DFB
    else:
        print "First parameter must be OGL or DFB"
    bpp = int(sys.argv[2])
    runner.run(playerTestSuite(engine, bpp))

