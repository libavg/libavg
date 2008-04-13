#!/usr/bin/python
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

CREATE_BASELINE_IMAGES = False
BASELINE_DIR = "baseline"
RESULT_DIR = "resultimages"

ourSaveDifferences = True
g_CustomOGLOptions = False

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
            self.__Player.setOGLOptions(g_UsePOW2Textures, g_YCbCrMode, g__UsePixelBuffers, 1)
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
        self.actions[self.curFrame]()
#        print (self.curFrame)
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
                NumPixels = self.__Player.getTestHelper().getNumDifferentPixels(Bmp, 
                        BaselineBmp)
                if (NumPixels > 20):
                    if ourSaveDifferences:
                        Bmp.save(RESULT_DIR+"/"+fileName+".png")
                        BaselineBmp.save(RESULT_DIR+"/"+fileName+"_baseline.png")
                        Bmp.subtract(BaselineBmp)
                        Bmp.save(RESULT_DIR+"/"+fileName+"_diff.png")
                    self.Log.trace(self.Log.WARNING, "Image compare: "+str(NumPixels)+
                            " bright pixels.")
                    if warn:
                        self.Log.trace(self.Log.WARNING, "Image "+fileName
                                +" differs from original.")
                    else:
                        self.assert_(False)
            except RuntimeError:
                Bmp.save(RESULT_DIR+"/"+fileName+".png")
                self.Log.trace(self.Log.WARNING, "Could not load image "+fileName+".png")
                self.assert_(False)

def setOGLOptions(UsePOW2Textures, YCbCrMode, UsePixelBuffers):
    global g_CustomOGLOptions
    global g_UsePOW2Textures
    global g_YCbCrMode
    global g_UsePixelBuffers
    g_CustomOGLOptions = True
    g_UsePOW2Textures = UsePOW2Textures
    g_YCbCrMode = YCbCrMode
    g_UsePixelBuffers = UsePixelBuffers

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

