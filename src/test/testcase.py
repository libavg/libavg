#!/usr/bin/python
# -*- coding: utf-8 -*-
# libavg - Media Playback Engine.
# Copyright (C) 2003-2011 Ulrich von Zadow
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

from libavg import avg, player

def almostEqual(a, b, epsilon):
    try:
        bOk = True
        for i in range(len(a)):
            if not(almostEqual(a[i], b[i], epsilon)):
                bOk = False
        return bOk
    except:
        return math.fabs(a-b) < epsilon

def flatten(l):
    ltype = type(l)
    l = list(l)
    i = 0
    while i < len(l):
        while isinstance(l[i], (list, tuple)):
            if not l[i]:
                l.pop(i)
                i -= 1
                break
            else:
                l[i:i + 1] = l[i]
        i += 1
    return ltype(l)

class AVGTestCase(unittest.TestCase):
    imageResultDirectory = "resultimages"
    baselineImageResultDirectory = "baseline"
    
    def __init__(self, testFuncName):
        unittest.TestCase.__init__(self, testFuncName)

        player.enableGLErrorChecks(True)
        self.__testFuncName = testFuncName
        self.__logger = avg.logger
        self.__skipped = False
        self.__warnOnImageDiff = False

    def __setupPlayer(self):
        player.setMultiSampleSamples(1)
        player.setResolution(0, 0, 0, 0)
    
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

    def start(self, warnOnImageDiff, actions):
        self.__setupPlayer()
        self.__dumpTestFrames = (os.getenv("AVG_DUMP_TEST_FRAMES") != None)
        self.__delaying = False
        self.__warnOnImageDiff = warnOnImageDiff
        
        self.assert_(player.isPlaying() == 0)
        self.actions = flatten(actions)
        self.curFrame = 0
        player.subscribe(player.ON_FRAME, self.__nextAction)
        player.setFramerate(10000)
        player.assumePixelsPerMM(1)
        player.play()
        self.assert_(player.isPlaying() == 0)

    def delay(self, time):
        def timeout():
            self.__delaying = False
        self.__delaying = True
        player.setTimeout(time, timeout)

    def compareImage(self, fileName):
        bmp = player.screenshot()
        self.compareBitmapToFile(bmp, fileName)

    def compareBitmapToFile(self, bmp, fileName):
        try:
            baselineBmp = avg.Bitmap(AVGTestCase.baselineImageResultDirectory + "/"
                    + fileName + ".png")
        except RuntimeError:
            bmp.save(AVGTestCase.getImageResultDir()+"/"+fileName+".png")
            self.__logger.trace(self.__logger.WARNING, 
                                "Could not load image "+fileName+".png")
            raise
        diffBmp = bmp.subtract(baselineBmp)
        average = diffBmp.getAvg()
        stdDev = diffBmp.getStdDev()
        if (average > 0.1 or stdDev > 0.5):
            if self._isCurrentDirWriteable():
                bmp.save(AVGTestCase.getImageResultDir() + "/" + fileName + ".png")
                baselineBmp.save(AVGTestCase.getImageResultDir() + "/" + fileName
                        + "_baseline.png")
                diffBmp.save(AVGTestCase.getImageResultDir() + "/" + fileName
                        + "_diff.png")
        if (average > 2 or stdDev > 6):
            msg = ("  "+fileName+
                    ": Difference image has avg=%(avg).2f, std dev=%(stddev).2f"%
                    {'avg':average, 'stddev':stdDev})
            if self.__warnOnImageDiff:
                sys.stderr.write("\n"+msg+"\n")
            else:
                self.fail(msg)

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

    def assertAlmostEqual(self, a, b, epsilon=0.00001):
        if not(almostEqual(a, b, epsilon)):
            msg = "almostEqual: " + str(a) + " != " + str(b)
            self.fail(msg)

    def loadEmptyScene(self, resolution=(160,120)):
        player.createMainCanvas(size=resolution)
        root = player.getRootNode()
        root.mediadir = "media"
        return root

    def initDefaultImageScene(self):
        root = self.loadEmptyScene()
        avg.ImageNode(id="testtiles", pos=(0,30), size=(65,65), href="rgb24-65x65.png", 
                maxtilewidth=16, maxtileheight=32, parent=root)
        avg.ImageNode(id="test", pos=(64,30), href="rgb24-65x65.png", pivot=(0,0),
                angle=0.274, parent=root)
        avg.ImageNode(id="test1", pos=(129,30), href="rgb24-65x65.png", parent=root)

    def fakeClick(self, x, y):
        helper = player.getTestHelper()
        helper.fakeMouseEvent(avg.Event.CURSOR_DOWN, True, False, False, x, y, 1)
        helper.fakeMouseEvent(avg.Event.CURSOR_UP, False, False, False, x, y, 1)

    def skip(self, message):
        self.__skipReason = str(message)
        sys.stderr.write("skipping: " + str(message) + " ... ")
        self.__skipped = True

    def skipped(self):
        return self.__skipped

    def skipReason(self):
        return self.__skipReason

    def _sendMouseEvent(self, type, x, y):
        helper = player.getTestHelper()
        if type == avg.Event.CURSOR_UP:
            button = False
        else:
            button = True
        helper.fakeMouseEvent(type, button, False, False, x, y, 1)

    def _sendTouchEvent(self, id, type, x, y):
        helper = player.getTestHelper()
        helper.fakeTouchEvent(id, type, avg.Event.TOUCH, avg.Point2D(x, y))
      
    def _sendTouchEvents(self, eventData):
        helper = player.getTestHelper()
        for (id, type, x, y) in eventData:
            helper.fakeTouchEvent(id, type, avg.Event.TOUCH, avg.Point2D(x, y))

    def _genMouseEventFrames(self, type, x, y, expectedEvents):
        return [
                 lambda: self._sendMouseEvent(type, x, y),
                 lambda: self.messageTester.assertState(expectedEvents),
                ]

    def _genTouchEventFrames(self, eventData, expectedEvents):
        return [
                 lambda: self._sendTouchEvents(eventData),
                 lambda: self.messageTester.assertState(expectedEvents),
                ]

    def _isCurrentDirWriteable(self):
        return bool(os.access('.', os.W_OK))
    
    def __nextAction(self):
        if not(self.__delaying):
            if self.__dumpTestFrames:
                self.__logger.trace(self.__logger.APP, "Frame "+str(self.curFrame))
            if len(self.actions) == self.curFrame:
                player.stop()
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
                sys.stderr.write(("No test named %s"%testName) + "\n")
                sys.exit(1)
    else:
        testNames = availableTests

    suite = unittest.TestSuite()
    for testName in testNames:
        suite.addTest(AVGTestCaseClass(testName))
    
    return suite


class NodeHandlerTester(object):
    def __init__(self, testCase, node):
        self.__testCase = testCase
        self.reset()
        self.__node = node
        self.__subscriberIDs = set()
        self.setHandlers()
        self.__messagesReceived = set()

    def assertState(self, expectedMessages):
        self.__testCase.assert_(self.isState(expectedMessages))
        self.reset()

    def isState(self, expectedMessages):
        expectedMessages = set(expectedMessages)
        if expectedMessages != self.__messagesReceived:
            sys.stderr.write("\nState expected: "+str(expectedMessages)+"\n")
            sys.stderr.write("Actual state: "+str(self.__messagesReceived)+"\n")
            return False
        else:
            return True

    def reset(self):
        self.__messagesReceived = set()

    def setHandlers(self):
        messageIDs = [avg.Node.CURSOR_DOWN, avg.Node.CURSOR_UP, avg.Node.CURSOR_OVER, 
                avg.Node.CURSOR_OUT, avg.Node.CURSOR_MOTION]
        for messageID in messageIDs:
            subscriberID = self.__node.subscribe(messageID, 
                    lambda event, messageID=messageID: 
                            self.setMessageReceived(messageID, event))
            self.__subscriberIDs.add(subscriberID)

    def clearHandlers(self):
        for subscriber in self.__subscriberIDs:
            self.__node.unsubscribe(subscriber)
        self.__subscriberIDs = set()

    def setMessageReceived(self, messageID, event):
        self.__messagesReceived.add(messageID)
    

class MessageTester(object):

    def __init__(self, publisher, messageIDs, testCase=None):
        for messageID in messageIDs:
            publisher.subscribe(messageID, 
                    lambda messageID=messageID: self.setMessageReceived(messageID))
        self.__messagesReceived = set()
        self.__testCase = testCase

    def assertState(self, expectedMessages):
        self.__testCase.assert_(self.isState(expectedMessages))
        self.reset()

    def isState(self, expectedMessages):
        expectedMessages = set(expectedMessages)
        if expectedMessages != self.__messagesReceived:
            sys.stderr.write("\nState expected: "+str(expectedMessages)+"\n")
            sys.stderr.write("Actual state: "+str(self.__messagesReceived)+"\n")
            return False
        else:
            return True

    def setMessageReceived(self, messageID):
        self.__messagesReceived.add(messageID)

    def reset(self):
        self.__messagesReceived = set()

