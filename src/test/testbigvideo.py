#!/usr/bin/python
# -*- coding: utf-8 -*-
import unittest

import sys, syslog
import avg

class BigVideoTestCase(unittest.TestCase):
    def test(self):
        def onFrame():
            Player.getElementByID("video").seekToFrame(self.__curFrame)
            self.__curFrame -= 3
            if self.__curFrame < 2:
                self.__curFrame = 200
        self.__curFrame = 200 
        Player.loadFile("videofmt.avg")
        Player.getElementByID("video").play()
        Player.setFramerate(100)
        Player.setInterval(10, onFrame)
        Player.play()

Player = avg.Player()
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

