#!/usr/bin/env python
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

import math

from libavg import avg
from testcase import *

g_FXSupported = None

class FXTestCase(AVGTestCase):
    def __init__(self, testFuncName):
        AVGTestCase.__init__(self, testFuncName)

    def testImageNullFX(self):
        def activateFX():
            node2.setEffect(avg.NullFXNode())

        def newNode():
            node = avg.ImageNode(parent=root, href="rgb24-32x32.png", pos=(64,0))
            node.setEffect(avg.NullFXNode())

        def addBgNode():
            node = avg.RectNode(pos=(0,32), size=(64,32), fillopacity=1, opacity=0,
                    fillcolor="FFFFFF")
            root.insertChild(node, 0)

        self.loadEmptyScene()
        root = Player.getRootNode()
        node = avg.ImageNode(parent=root, href="rgb24-32x32.png")
        node.setEffect(avg.NullFXNode())
        node = avg.ImageNode(parent=root, href="rgb24alpha-32x32.png", pos=(0,32))
        node.setEffect(avg.NullFXNode())
        node = avg.ImageNode(parent=root, href="rgb24alpha-32x32.png", pos=(32,32),
                opacity=0.6)
        node.setEffect(avg.NullFXNode())
        node2 = avg.ImageNode(parent=root, href="rgb24-32x32.png", pos=(32,0))
        self.start(None,
                (lambda: self.compareImage("testImageNullFX1", False),
                 activateFX,
                 lambda: self.compareImage("testImageNullFX1", False),
                 newNode,
                 lambda: self.compareImage("testImageNullFX2", False),
                 addBgNode,
                 lambda: self.compareImage("testImageNullFX3", False),
                ))

    def testVideoNullFX(self):
        self.loadEmptyScene()
        root = Player.getRootNode()
        Player.setFakeFPS(25)
        node = avg.VideoNode(parent=root, href="../video/testfiles/mjpeg-48x48.avi",
                threaded=False)
        node.setEffect(avg.NullFXNode())
        node.play()
        self.start(None,
                (lambda: self.compareImage("testVideoNullFX", False),
                ))

    def testWordsNullFX(self):
        self.loadEmptyScene()
        root = Player.getRootNode()
        node = avg.WordsNode(parent=root, text="testtext", font="Bitstream Vera Sans")
        node.setEffect(avg.NullFXNode())
        self.start(None,
                (lambda: self.compareImage("testWordsNullFX", True),
                ))

    def testCanvasNullFX(self):
        def setOpacity():
            node.opacity=0.6

        Player.loadCanvasString("""
            <canvas id="offscreen" width="160" height="120">
                <image href="rgb24-32x32.png"/>
                <image pos="(32,0)" href="rgb24alpha-32x32.png"/>
            </canvas>""")
        self.loadEmptyScene()
        root = Player.getRootNode()
        node = avg.ImageNode(parent=root, href="canvas:offscreen")
        node.setEffect(avg.NullFXNode())
        self.start(None,
                (lambda: self.compareImage("testCanvasNullFX1", False),
                 setOpacity,
                 lambda: self.compareImage("testCanvasNullFX2", False),
                ))

    def testColorFX(self):
        def setAlphaImage():
            node.href="rgb24alpha-64x64.png"
            effect.setParams(1,2,1,1,1)

        if not(self._areFXSupported("testColorFX")):
            return
        self.loadEmptyScene()
        root = Player.getRootNode()
        node = avg.ImageNode(parent=root, href="colorramp.png")
        effect = avg.ColorFXNode()
        node.setEffect(effect)
        self.start(None,
                (lambda: self.compareImage("testColorFX1", False),
                 lambda: effect.setParams(1,2,1,1,1),
                 lambda: self.compareImage("testColorFX2", False),
                 lambda: effect.setParams(0.5,1,1,1,1),
                 lambda: self.compareImage("testColorFX3", False),
                 lambda: effect.setParams(1,1,0.3,1,1),
                 lambda: self.compareImage("testColorFX4", False),
                 lambda: effect.setParams(1,1,1,0.3,1),
                 lambda: self.compareImage("testColorFX5", False),
                 lambda: effect.setParams(1,1,1,1,0.3),
                 lambda: self.compareImage("testColorFX6", False),
                 setAlphaImage,
                 lambda: self.compareImage("testColorFX7", False),
                ))

    def testBlurFX(self):
        if not(self._areFXSupported("testBlurFX")):
            return
        self.loadEmptyScene()
        root = Player.getRootNode()
        node = avg.ImageNode(parent=root, href="rgb24-64x64.png")
        effect = avg.BlurFXNode()
        node.setEffect(effect)
        self.start(None,
                (lambda: self.compareImage("testBlurFX1", False),
                 lambda: effect.setParam(8),
                 lambda: self.compareImage("testBlurFX2", False),
                ))

    def testShadowFX(self):
        if not(self._areFXSupported("testShadowFX")):
            return
        self.loadEmptyScene()
        root = Player.getRootNode()
        node = avg.ImageNode(parent=root, href="shadow.png")
        effect = avg.ShadowFXNode()
        node.setEffect(effect)
        self.start(None,
                (lambda: self.compareImage("testShadowFX1", False),
                 lambda: effect.setParams((0,0), 3, 0.2, "00FFFF"),
                 lambda: self.compareImage("testShadowFX2", False),
                 lambda: effect.setParams((2,2), 2, 0.2, "FFFFFF"),
                 lambda: self.compareImage("testShadowFX3", False),
                ))

    def testGamma(self):
        def setGamma(val):
            node.gamma = val

        def testGetGamma(gamma):
            self.assert_(node.gamma == gamma)

        self.loadEmptyScene()
        root = Player.getRootNode()
        node = avg.ImageNode(parent=root, href="colorramp.png", gamma=(0.5,0.5,0.5))
        self.assert_(node.gamma == (0.5,0.5,0.5))
        self.start(None,
                (lambda: self.compareImage("testGamma1", False),
                 lambda: setGamma((1.5,2.0,2.5)),
                 lambda: testGetGamma((1.5,2.0,2.5)),
                 lambda: self.compareImage("testGamma2", False),
                ))

    def testBrightness(self):
        def setBrightness(val):
            node.brightness = val

        def testGetBrightness(brightness):
            self.assert_(node.brightness == brightness)

        self.loadEmptyScene()
        root = Player.getRootNode()
        node = avg.ImageNode(parent=root, href="colorramp.png", brightness=(0.5,0.5,0.5))
        self.assert_(node.brightness == (0.5,0.5,0.5))
        self.start(None,
                (lambda: self.compareImage("testBrightness1", False),
                 lambda: setBrightness((1.5,2.0,2.5)),
                 lambda: testGetBrightness((1.5,2.0,2.5)),
                 lambda: self.compareImage("testBrightness2", False),
                ))


    def testContrast(self):
        def setContrast(val):
            node.contrast = val

        def testGetContrast(contrast):
            self.assert_(node.contrast == contrast)

        self.loadEmptyScene()
        root = Player.getRootNode()
        node = avg.ImageNode(parent=root, href="colorramp.png", contrast=(0.5,0.5,0.5))
        self.assert_(node.contrast == (0.5,0.5,0.5))
        self.start(None,
                (lambda: self.compareImage("testContrast1", False),
                 lambda: setContrast((1.5,2.0,2.5)),
                 lambda: testGetContrast((1.5,2.0,2.5)),
                 lambda: self.compareImage("testContrast2", False),
                ))


    def _areFXSupported(self, testName):
        global g_FXSupported
        if g_FXSupported == None:
            self.loadEmptyScene()
            node = avg.ImageNode(href="rgb24-65x65.png")
            effect = avg.BlurFXNode()
            node.setEffect(effect)
            Player.getRootNode().appendChild(node)
            try:
                self.start(None, [])
                g_FXSupported = True
            except RuntimeError:
                g_FXSupported = False
                print ("Skipping "+testName
                        +" - no FX support with this graphics configuration.")
        return g_FXSupported


def fxTestSuite(tests):
    availableTests = (
            "testImageNullFX",
            "testVideoNullFX",
            "testWordsNullFX",
            "testCanvasNullFX",
            "testColorFX",
            "testBlurFX",
            "testShadowFX",
            "testGamma",
            "testBrightness",
            "testContrast",
            )
    return createAVGTestSuite(availableTests, FXTestCase, tests)

Player = avg.Player.get()
