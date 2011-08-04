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

from libavg import avg


def almostEqual(a,b):
    try:
        bOk = True
        for i in range(len(a)):
            if not(almostEqual(a[i], b[i])):
                bOk = False
        return bOk
    except:
        return math.fabs(a-b) < 0.000001


class AVGTestCase(unittest.TestCase):
    imageResultDirectory = "resultimages"
    baselineImageResultDirectory = "baseline"
    
    def __init__(self, testFuncName):
        unittest.TestCase.__init__(self, testFuncName)

        self.__player = avg.Player.get()
        self.__testFuncName = testFuncName
        self.__logger = avg.Logger.get()

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
    def cleanResultDir():
        dir = AVGTestCase.getImageResultDir()
        try:
            files = os.listdir(dir)
            for file in files:
                os.remove(dir+"/"+file)
        except OSError:
            try:
                os.mkdir(dir)
            except OSError:
                pass

    @staticmethod
    def setBaselineImageDirectory(name):
        AVGTestCase.baselineImageResultDirectory = name
    
    @staticmethod
    def getBaselineImageDir():
        return AVGTestCase.baselineImageResultDirectory
    
    def start(self, actions):
        self.__setupPlayer()
        self.__dumpTestFrames = (os.getenv("AVG_DUMP_TEST_FRAMES") != None)
        
        self.assert_(self.__player.isPlaying() == 0)
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
            baselineBmp = avg.Bitmap(AVGTestCase.getBaselineImageDir()+"/"+fileName+".png")
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
        self.__player.loadString(sceneString)
        return self.__player.getRootNode()

    def initDefaultImageScene(self):
        root = self.loadEmptyScene()
        avg.ImageNode(id="testtiles", pos=(0,30), size=(65,65), href="rgb24-65x65.png", 
                maxtilewidth=16, maxtileheight=32, parent=root)
        avg.ImageNode(id="test", pos=(64,30), href="rgb24-65x65.png", pivot=(0,0),
                angle=0.274, parent=root)
        avg.ImageNode(id="test1", pos=(129,30), href="rgb24-65x65.png", parent=root)

    def fakeClick(self, x, y):
        helper = self.__player.getTestHelper()
        helper.fakeMouseEvent(avg.CURSORDOWN, True, False, False, x, y, 1)
        helper.fakeMouseEvent(avg.CURSORUP, False, False, False, x, y, 1)

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


class NodeHandlerTester:
    def __init__(self, testCase, node):
        self.__testCase=testCase
        self.reset()
        self.__node = node
        self.setHandlers()

    def assertState(self, down, up, over, out, move):
        self.__testCase.assert_(down == self.__downCalled)
        self.__testCase.assert_(up == self.__upCalled)
        self.__testCase.assert_(over == self.__overCalled)
        self.__testCase.assert_(out == self.__outCalled)
        self.__testCase.assert_(move == self.__moveCalled)
        self.__testCase.assert_(not(self.__touchDownCalled))
        self.reset()

    def reset(self):
        self.__upCalled=False
        self.__downCalled=False
        self.__overCalled=False
        self.__outCalled=False
        self.__moveCalled=False
        self.__touchDownCalled=False

    def setHandlers(self):
        self.__node.setEventHandler(avg.CURSORDOWN, avg.MOUSE, self.__onDown) 
        self.__node.setEventHandler(avg.CURSORUP, avg.MOUSE, self.__onUp) 
        self.__node.setEventHandler(avg.CURSOROVER, avg.MOUSE, self.__onOver) 
        self.__node.setEventHandler(avg.CURSOROUT, avg.MOUSE, self.__onOut) 
        self.__node.setEventHandler(avg.CURSORMOTION, avg.MOUSE, self.__onMove) 
        self.__node.setEventHandler(avg.CURSORDOWN, avg.TOUCH, self.__onTouchDown) 

    def clearHandlers(self):
        self.__node.setEventHandler(avg.CURSORDOWN, avg.MOUSE, None) 
        self.__node.setEventHandler(avg.CURSORUP, avg.MOUSE, None) 
        self.__node.setEventHandler(avg.CURSOROVER, avg.MOUSE, None) 
        self.__node.setEventHandler(avg.CURSOROUT, avg.MOUSE, None) 
        self.__node.setEventHandler(avg.CURSORMOTION, avg.MOUSE, None) 
        self.__node.setEventHandler(avg.CURSORDOWN, avg.TOUCH, None) 

    def __onDown(self, Event):
        self.__testCase.assert_(Event.type == avg.CURSORDOWN)
        self.__downCalled = True
    
    def __onUp(self, Event):
        self.__testCase.assert_(Event.type == avg.CURSORUP)
        self.__upCalled = True

    def __onOver(self, Event):
        self.__testCase.assert_(Event.type == avg.CURSOROVER)
        self.__overCalled = True
    
    def __onOut(self, Event):
        self.__testCase.assert_(Event.type == avg.CURSOROUT)
        self.__outCalled = True
    
    def __onMove(self, Event):
        self.__testCase.assert_(Event.type == avg.CURSORMOTION)
        self.__moveCalled = True
    
    def __onTouchDown(self, Event):
        self.__touchDownCalled = True

