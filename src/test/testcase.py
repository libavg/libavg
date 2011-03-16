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

import sys
import os
import math

import libavg


class AVGTestCase(unittest.TestCase):
    imageResultDirectory = ''
    baselineImageResultDirectory = ''
    
    def __init__(self, testFuncName):
        unittest.TestCase.__init__(self, testFuncName)

        self.__player = libavg.Player.get()
        self.__testFuncName = testFuncName
        self.__logger = libavg.Logger.get()

    def __setupPlayer(self):
        self.__player.setMultiSampleSamples(1)
        self.__player.setResolution(0, 0, 0, 0)
    
    @staticmethod
    def setImageResultDirectory(name):
        AVGTestCase.imageResultDirectory = name
    
    @staticmethod
    def getImageResultDir():
        return AVGTestCase.imageResultDirectory
    
    @staticmethod
    def setBaselineImageDirectory(name):
        AVGTestCase.baselineImageResultDirectory = name
    
    @staticmethod
    def getBaselineImageDir():
        return AVGTestCase.baselineImageResultDirectory
    
    def start(self, filename, actions):
        self.__setupPlayer()
        self.__dumpTestFrames = (os.getenv("AVG_DUMP_TEST_FRAMES") != None)
        
        self.assert_(self.__player.isPlaying() == 0)
        if filename != None:
            self.__player.loadFile(filename)
        self.actions = actions
        self.curFrame = 0
        self.__player.setOnFrameHandler(self.__nextAction)
        self.__player.setFramerate(10000)
        self.__player.play()
        self.assert_(self.__player.isPlaying() == 0)


    def compareImage(self, fileName, warn):
        bmp = self.__player.screenshot()
        self.compareBitmapToFile(bmp, fileName, warn)

    def compareBitmapToFile(self, bmp, fileName, warn):
        try:
            baselineBmp = libavg.Bitmap(AVGTestCase.getBaselineImageDir()+"/"+fileName+".png")
            diffBmp = bmp.subtract(baselineBmp)
            average = diffBmp.getAvg()
            stdDev = diffBmp.getStdDev()
            if (average > 0.1 or stdDev > 0.5):
                if self._isCurrentDirWriteable():
                    bmp.save(AVGTestCase.getImageResultDir()+"/"+fileName+".png")
                    baselineBmp.save(AVGTestCase.getImageResultDir()+"/"+fileName+"_baseline.png")
                    diffBmp.save(AVGTestCase.getImageResultDir()+"/"+fileName+"_diff.png")
            if (average > 2 or stdDev > 6):
                print ("  "+fileName+
                        ": Difference image has avg=%(avg).2f, std dev=%(stddev).2f"%
                        {'avg':average, 'stddev':stdDev})
                if not(warn):
                    self.assert_(False)
        except RuntimeError:
            bmp.save(AVGTestCase.getImageResultDir()+"/"+fileName+".png")
            self.__logger.trace(self.__logger.WARNING, 
                                "Could not load image "+fileName+".png")
            raise

    def areSimilarBmps(self, bmp1, bmp2, maxAvg, maxStdDev):
        diffBmp = bmp1.subtract(bmp2)
        avg = diffBmp.getAvg()
        stdDev = diffBmp.getStdDev()
        return avg <= maxAvg and stdDev <= maxStdDev

    def assertException(self, code):
        exceptionRaised = False
        try:
            code()
        except:
            exceptionRaised = True
        self.assert_(exceptionRaised)

    def loadEmptyScene(self, resolution = (160,120)):
        sceneString = """
        <avg id="avg" width="%d" height="%d">
        </avg>
        """ % (resolution[0], resolution[1])
        return self.__player.loadString(sceneString)

    def _isCurrentDirWriteable(self):
        return bool(os.access('.', os.W_OK))
    
    def __nextAction(self):
        if self.__dumpTestFrames:
            self.__logger.trace(self.__logger.APP, "Frame "+str(self.curFrame))
        if len(self.actions) == self.curFrame:
            self.__player.stop()
        else:
            action = self.actions[self.curFrame]
            if action != None:
                action()
        self.curFrame += 1
    

def createAVGTestSuite(availableTests, AVGTestCaseClass, testSubset):
    testNames = []
    if testSubset:
        for testName in testSubset:
            if testName in availableTests:
                testNames.append(testName)
            else:
                print "no test named %s" % testName
                sys.exit(1)
    else:
        testNames = availableTests

    suite = unittest.TestSuite()
    for testName in testNames:
        suite.addTest(AVGTestCaseClass(testName))
    
    return suite
