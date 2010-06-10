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
        node = avg.VideoNode(parent=root, href="../video/testfiles/mjpeg-48x48.avi")
        node.setEffect(avg.NullFXNode())
        node.play()
        self.start(None,
                (lambda: self.compareImage("testVideoNullFX", False),
                ))

    def testWordsNullFX(self):
        self.loadEmptyScene()
        root = Player.getRootNode()
        node = avg.WordsNode(parent=root, text="testtext")
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


def fxTestSuite(tests):
    availableTests = (
            "testImageNullFX",
            "testVideoNullFX",
            "testWordsNullFX",
            "testCanvasNullFX",
            "testColorFX",
            )
    return createAVGTestSuite(availableTests, FXTestCase, tests)

Player = avg.Player.get()
