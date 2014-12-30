#!/usr/bin/env python
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

import libavg
from libavg import avg, player
from testcase import *
import AppTest

class MultiWindowTestCase(AVGTestCase):
    def __init__(self, testFuncName):
        AVGTestCase.__init__(self, testFuncName)

    def testMultiWindowBase(self):
        root = self.loadEmptyScene()
        avg.ImageNode(pos=(0,0), href="rgb24-64x64.png", parent=root)
        player.setWindowConfig("avgwindowconfig.xml")
        self.start(False,
                (lambda: self.compareImage("testMultiWindow1"),
                ))

    def testMultiWindowApp(self):
        app = AppTest.TestApp()
        app.CUSTOM_SETTINGS = {}
        app.settings.set('app_windowconfig', 'avgwindowconfig.xml')
        app.settings.set('app_window_size', '160x120')
        self.assertRaises(TypeError, lambda: app.testRun([]))
        libavg.app.instance = None

        app = AppTest.TestApp()
        app.CUSTOM_SETTINGS = {}
        app.settings.set('app_windowconfig', 'avgwindowconfig.xml')
        app.testRun([])

    def testMultiWindowCanvas(self):
        def deleteCanvas():
            img1.unlink(True)
            img2.unlink(True)
            player.deleteCanvas("canvas")

        def canvasScreenshot():
            bmp = canvas.screenshot()
            self.compareBitmapToFile(bmp, "testMultiWindowCanvas2")


        root = self.loadEmptyScene()
        player.setWindowConfig("avgwindowconfig.xml")
        canvas = player.createCanvas(id="canvas", size=(160,120))
        avg.ImageNode(pos=(0,0), href="media/rgb24-64x64.png", 
                parent=canvas.getRootNode())
        img1 = avg.ImageNode(pos=(0,0), href="canvas:canvas", parent=root)
        img2 = avg.ImageNode(pos=(80,0), href="canvas:canvas", parent=root)
        self.start(False,
                (lambda: self.compareImage("testMultiWindowCanvas1"),
                 canvasScreenshot,
                 deleteCanvas,
                ))

    def testMultiWindowManualCanvas(self):
        def renderCanvas():
            canvas.render()
            bmp = canvas.screenshot()
            self.compareBitmapToFile(bmp, "testMultiWindowManualCanvas")

        self.loadEmptyScene()
        player.setWindowConfig("avgwindowconfig.xml")
        canvas = player.createCanvas(id="canvas", size=(160,120), autorender=False)
        avg.ImageNode(pos=(0,0), href="media/rgb24-64x64.png", 
                parent=canvas.getRootNode())
        self.start(False,
                (renderCanvas,
                ))

    def testMultiWindowFX(self):
        def setHueSat(node):
            effect = avg.HueSatFXNode()
            effect.saturation = -200
            node.setEffect(effect)

        def setBlur(node):
            node.setEffect(avg.BlurFXNode(3))

        def startVideo():
            node.unlink(True)
            self.videoNode = avg.VideoNode(href="mpeg1-48x48.mov", size=(96,96), 
                    threaded=False, parent=root)
            self.videoNode.setEffect(avg.NullFXNode())
            self.videoNode.play()

        def addWords():
            self.videoNode.unlink(True)
            self.wordsNode = avg.WordsNode(text="WordsNode", color="FF0000", parent=root)
            self.wordsNode.setEffect(avg.NullFXNode())

        root = self.loadEmptyScene()
        player.setWindowConfig("avgwindowconfig.xml")
        node = avg.ImageNode(pos=(0,0), href="rgb24-64x64.png", parent=root)
        node.setEffect(avg.NullFXNode())
        player.setFakeFPS(25)
        self.start(False,
                (lambda: self.compareImage("testMultiWindowFX1"),
                 lambda: setHueSat(node),
                 lambda: self.compareImage("testMultiWindowFX2"),
                 lambda: setBlur(node),
                 lambda: self.compareImage("testMultiWindowFX3"),
                 startVideo,
                 lambda: self.compareImage("testMultiWindowFXVideo1"),
                 lambda: setHueSat(self.videoNode),
                 lambda: self.compareImage("testMultiWindowFXVideo2"),
                 addWords,
                 lambda: self.compareImage("testMultiWindowFXWords1"),
                 lambda: setHueSat(self.wordsNode),
                 lambda: self.compareImage("testMultiWindowFXWords2"),
                ))
        
        

def multiWindowTestSuite(tests):
    if sys.platform.startswith("linux"):
        if not player.isUsingGLES():
            availableTests = (
                    "testMultiWindowBase",
                    "testMultiWindowApp",
                    "testMultiWindowCanvas",
                    "testMultiWindowManualCanvas",
                    "testMultiWindowFX"
                    )
            return createAVGTestSuite(availableTests, MultiWindowTestCase, tests)
        else:
            sys.stderr.write("Skipping multi-window tests - only supported under Linux w/GLX")
            return unittest.TestSuite()
    else:
        sys.stderr.write("Skipping multi-window tests - only supported under Linux.")
        return unittest.TestSuite()
        
