#!/usr/bin/python
# -*- coding: utf-8 -*-
import unittest

import sys, os, platform, math

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

CREATE_BASELINE_IMAGES = False
BASELINE_DIR = "baseline"
RESULT_DIR = "resultimages"

ourSaveDifferences = True
g_CustomOGLOptions = False
g_UsePOW2Textures = False
g_YCbCrMode = None
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


class AVGTestCase(unittest.TestCase):
    def __init__(self, testFuncName, bpp):
        self.__Player = avg.Player.get()
        self.__bpp = bpp
        self.__testFuncName = testFuncName
        self.Log = avg.Logger.get()
        unittest.TestCase.__init__(self, testFuncName)

    def setUpVideo(self):
        self.__Player.setResolution(0, 0, 0, self.__bpp)
        if g_CustomOGLOptions:
            self.__Player.setOGLOptions(g_UsePOW2Textures, g_YCbCrMode, 
                    g_UsePixelBuffers, 1)
        self.__Player.setMultiSampleSamples(1)

    def setUp(self):
        self.setUpVideo()
        print "-------- ", self.__testFuncName, " --------"

    def start(self, filename, actions):
        self.assert_(self.__Player.isPlaying() == 0)
        if filename != None:
            self.__Player.loadFile(filename)
        self.actions = actions
        self.curFrame = 0
        self.__Player.setOnFrameHandler(self.nextAction)
        self.__Player.setFramerate(100)
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
        global CREATE_BASELINE_IMAGES
        global ourSaveDifferences
        Bmp = self.__Player.screenshot()
        if CREATE_BASELINE_IMAGES:
            Bmp.save(BASELINE_DIR+"/"+fileName+".png")
        else:
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
                if (average > 2 or stdDev > 5):
                    print ("  "+fileName+
                            ": Difference image has avg=%(avg).2f, std dev=%(stddev).2f"%
                            {'avg':average, 'stddev':stdDev})
                    if not(warn):
                        self.assert_(False)
            except RuntimeError:
                Bmp.save(RESULT_DIR+"/"+fileName+".png")
                self.Log.trace(self.Log.WARNING, "Could not load image "+fileName+".png")
                self.assert_(False)

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


def setYCbCrMode (val):
    global g_CustomOGLOptions
    global g_YCbCrMode
    g_YCbCrMode = val

def setUsePOW2Textures (val):
    global g_CustomOGLOptions
    global g_UsePOW2Textures
    g_CustomOGLOptions = True
    g_UsePOW2Textures = val

def setUsePixelBuffers (val):
    global g_CustomOGLOptions
    global g_UsePixelBuffers
    g_CustomOGLOptions = True
    g_UsePixelBuffers = val


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

def AVGTestSuite (availableTests, TestCase, tests, extraargs=(), extrakwargs={}):
    suite = unittest.TestSuite()
    if tests:
        for testname in tests:
            if testname in availableTests:
                name = testname
            elif 'test'+testname in availableTests:
                name = 'test' + testname
            else:
                print "no test named %s" % testname
                sys.exit(1)
            testNames = (name,)
    else:
            testNames = availableTests
    for name in testNames:
        suite.addTest(TestCase(*([name,]+list(extraargs)), **extrakwargs ))
    return suite

def runStandaloneTest(suite):
    runner = unittest.TextTestRunner()
    rc = runner.run(suite(None))
    return 0 if rc.wasSuccessful() else 1


    pass

