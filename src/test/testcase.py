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

BASELINE_DIR = "baseline"
RESULT_DIR = "resultimages"

ourSaveDifferences = True
g_CustomOGLOptions = False
g_UsePOW2Textures = False
g_UseShaders = True
g_UsePixelBuffers = True

def almostEqual(a,b):
    try:
        bOk = True
        for i in range(len(a)):
            if math.fabs(a[i]-b[i]) > 0.000001:
                bOk = False
        return bOk
    except:
        return math.fabs(a-b) < 0.000001

def setUpVideo(player):
    if g_CustomOGLOptions:
        player.setOGLOptions(g_UsePOW2Textures, g_UseShaders, g_UsePixelBuffers, 1)
    player.setMultiSampleSamples(1)


class AVGTestCase(unittest.TestCase):
    def __init__(self, testFuncName):
        self.__Player = avg.Player.get()
        self.__testFuncName = testFuncName
        self.Log = avg.Logger.get()
        unittest.TestCase.__init__(self, testFuncName)

    def setUp(self):
        self.__Player.setResolution(0, 0, 0, 0)
        setUpVideo(self.__Player)
        self.__Player.enableAudio(False)
        print "-------- ", self.__testFuncName, " --------"

    def start(self, filename, actions):
        self.assert_(self.__Player.isPlaying() == 0)
        if filename != None:
            self.__Player.loadFile(filename)
        self.actions = actions
        self.curFrame = 0
        self.__Player.setOnFrameHandler(self.nextAction)
        self.__Player.setFramerate(10000)
        self.__Player.play()
        self.assert_(self.__Player.isPlaying() == 0)

    def nextAction(self):
        if len(self.actions) == self.curFrame:
            self.__Player.stop()
        else:
            action = self.actions[self.curFrame]
#            print self.__Player.getFrameTime(), ": ", self.curFrame, repr(action)
            if action != None:
                action()
        self.curFrame += 1

    def compareImage(self, fileName, warn):
        global ourSaveDifferences
        Bmp = self.__Player.screenshot()
        try:
            BaselineBmp = avg.Bitmap(BASELINE_DIR+"/"+fileName+".png")
            DiffBmp = Bmp.subtract(BaselineBmp)
            average = DiffBmp.getAvg()
            stdDev = DiffBmp.getStdDev()
            if (average > 0.1 or stdDev > 0.5):
                if ourSaveDifferences:
                    Bmp.save(RESULT_DIR+"/"+fileName+".png")
                    BaselineBmp.save(RESULT_DIR+"/"+fileName+"_baseline.png")
                    DiffBmp.save(RESULT_DIR+"/"+fileName+"_diff.png")
            if (average > 2 or stdDev > 6):
                print ("  "+fileName+
                        ": Difference image has avg=%(avg).2f, std dev=%(stddev).2f"%
                        {'avg':average, 'stddev':stdDev})
                if not(warn):
                    self.assert_(False)
        except RuntimeError:
            Bmp.save(RESULT_DIR+"/"+fileName+".png")
            self.Log.trace(self.Log.WARNING, "Could not load image "+fileName+".png")
            raise

    def areSimilarBmps(self, bmp1, bmp2, maxAvg, maxStdDev):
        DiffBmp = bmp1.subtract(bmp2)
        avg = DiffBmp.getAvg()
        stdDev = DiffBmp.getStdDev()
        return avg <= maxAvg and stdDev <= maxStdDev

    def assertException(self, code):
        exceptionRaised = False
        try:
            code()
        except:
            exceptionRaised = True
        self.assert_(exceptionRaised)

    def _loadEmpty(self):
        self.__Player.loadString("""
        <?xml version="1.0"?>
        <avg id="avg" width="160" height="120">
        </avg>
        """)


def setUseShaders(val):
    global g_CustomOGLOptions
    global g_UseShaders
    g_UseShaders = val

def setUsePOW2Textures(val):
    global g_CustomOGLOptions
    global g_UsePOW2Textures
    g_CustomOGLOptions = True
    g_UsePOW2Textures = val

def setUsePixelBuffers(val):
    global g_CustomOGLOptions
    global g_UsePixelBuffers
    g_CustomOGLOptions = True
    g_UsePixelBuffers = val


def isDirWritable():
    return ourSaveDifferences

def rmBrokenDir():
    try:
        files = os.listdir(RESULT_DIR)
        for file in files:
            os.remove(RESULT_DIR+"/"+file)
    except OSError:
        try:
            os.mkdir(RESULT_DIR)
        except OSError:
            # This can happen on make distcheck (permission denied...)
            global ourSaveDifferences
            ourSaveDifferences = False

def AVGTestSuite (availableTests, AVGTestCaseClass, testSubset):
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
