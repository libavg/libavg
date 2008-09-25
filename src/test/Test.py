#!/usr/bin/env python
# -*- coding: utf-8 -*-
import unittest

import sys, os, platform

# Import the correct version of libavg. Since it should be possible to
# run the tests without installing libavg, we add the location of the 
# uninstalled libavg to the path.
# TODO: This is a mess. 
sys.path += ['../wrapper/.libs', '../python']
if platform.system() == 'Darwin':
    sys.path += ['../..']     # Location of libavg in a mac installation. 

if platform.system() == 'Windows':
    from libavg import avg    # Under windows, there is no uninstalled version.
else:    
    import avg

SrcDir = os.getenv("srcdir",".")
os.chdir(SrcDir)

from LoggerTest import *
from AVGTest import *
from VectorTest import *
from WordsTest import *
from AVTest import *
from DynamicsTest import *
from PythonTest import *

def AVGTestSuite(bpp):
    suite = unittest.TestSuite()
    suite.addTest(LoggerTestCase())
    suite.addTest(playerTestSuite(bpp))
    suite.addTest(vectorTestSuite())
    suite.addTest(wordsTestSuite())
    suite.addTest(avTestSuite())
    suite.addTest(dynamicsTestSuite())
    suite.addTest(pythonTestSuite())
    return suite

def runConsoleTest():
    Player = avg.Player.get()
    Player.loadFile("video.avg")

def getBoolParam(paramIndex):
    param = sys.argv[paramIndex].upper()
    if param == "TRUE":
        return True
    elif param == "FALSE":
        return False
    else:
        print "Parameter "+paramIndex+" must be 'True' or 'False'"

if os.getenv("AVG_CONSOLE_TEST"):
    runConsoleTest()
else:
    if len(sys.argv) == 1:
        bpp = 24
        customOGLOptions = False
    elif len(sys.argv) == 2 or len(sys.argv) == 5:
        bpp = int(sys.argv[1])
        if (len(sys.argv) == 5):
            customOGLOptions = True
            UsePOW2Textures = getBoolParam(2)
            s = sys.argv[3]
            if s == "shader":
                YCbCrMode = avg.shader
            elif s == "apple":
                YCbCrMode = avg.apple
            elif s == "mesa":
                YCbCrMode = avg.mesa
            elif s == "none":
                YCbCrMode = avg.none
            else:
                print "Third parameter must be shader, apple, mesa or none"
                sys.exit(1)
            UsePixelBuffers = getBoolParam(4)
            setOGLOptions(UsePOW2Textures, YCbCrMode, UsePixelBuffers)
    else:
        print "Usage: Test.py [<bpp>"
        print "               [<UsePOW2Textures> <YCbCrMode> <UsePixelBuffers>]]"
        sys.exit(1)

    Player = avg.Player.get()
    Log = avg.Logger.get()
    Log.setCategories(
            Log.APP |
            Log.WARNING |
#            Log.PROFILE |
#            Log.PROFILE_LATEFRAMES |
#            Log.CONFIG  |
#            Log.MEMORY  |
#            Log.BLTS    |
#            Log.EVENTS  |
#            Log.EVENTS2 |
    0)

    runner = unittest.TextTestRunner()
    rmBrokenDir()
    rc = runner.run(AVGTestSuite(bpp))
    
    if rc.wasSuccessful():
        sys.exit(0)
    else:
        sys.exit(1)

