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

from libavg import avg, utils
from testcase import *

class FXTestCase(AVGTestCase):
    def __init__(self, testFuncName):
        AVGTestCase.__init__(self, testFuncName)

    def testImageNullFX(self):
        def activateFX():
            for node in self.nodes[0]:
                node.setEffect(avg.NullFXNode())

        def newNode():
            self.newNode = avg.ImageNode(parent=root, href="rgb24-32x32.png", pos=(64,0))
            self.newNode.setEffect(avg.NullFXNode())

        def newFX():
            self.newNode.setEffect(avg.NullFXNode())

        def addBgNode():
            node = avg.RectNode(pos=(0,0), size=(64,96), fillopacity=1, opacity=0,
                    fillcolor="FFFFFF")
            root.insertChild(node, 0)

        # Initial setup is 3x2 images: 
        # rows: no alpha, alpha, alpha & opacity 0.6
        # cols: no FX, FX
        # The two cols should look the same.
        root = self.loadEmptyScene()
        self.nodes = []
        for fx in (False, True):
            curNodes = []
            self.nodes.append(curNodes)
            def configureNode(node, fx):
                curNodes.append(node)
                if fx:
                    node.x = 32
                    node.setEffect(avg.NullFXNode())

            node = avg.ImageNode(parent=root, href="rgb24-32x32.png", pos=(0,0))
            configureNode(node, fx)
            node = avg.ImageNode(parent=root, href="rgb24alpha-32x32.png", pos=(0,32))
            configureNode(node, fx)
            node = avg.ImageNode(parent=root, href="rgb24alpha-32x32.png", pos=(0,64),
                opacity=0.6)
            configureNode(node, fx)

        self.start((
                 lambda: self.compareImage("testImageNullFX1", False),
                 addBgNode,
                 lambda: self.compareImage("testImageNullFX2", False),
                 activateFX,
                 lambda: self.compareImage("testImageNullFX2", False),
                 newNode,
                 lambda: self.compareImage("testImageNullFX3", False),
                 newFX,
                 lambda: self.compareImage("testImageNullFX3", False),
                 lambda: utils.initFXCache(10),
                ))

    def testVideoNullFX(self):
        root = self.loadEmptyScene()
        Player.setFakeFPS(25)
        node = avg.VideoNode(parent=root, href="../video/testfiles/mjpeg-48x48.avi",
                threaded=False)
        node.setEffect(avg.NullFXNode())
        node.play()
        self.start((lambda: self.compareImage("testVideoNullFX", False),))

    def testWordsNullFX(self):
        root = self.loadEmptyScene()
        node = avg.WordsNode(parent=root, text="testtext", font="Bitstream Vera Sans")
        node.setEffect(avg.NullFXNode())
        node = avg.WordsNode(parent=root, text="testtext", pos=(0,20),
                font="Bitstream Vera Sans")
        self.start((
                 lambda: self.compareImage("testWordsNullFX", True),
                ))

    def testCanvasNullFX(self):
        def setOpacity():
            node.opacity=0.6

        root = self.loadEmptyScene()
        self.__createOffscreenCanvas()
        node = avg.ImageNode(parent=root, href="canvas:offscreen")
        node.setEffect(avg.NullFXNode())
        self.start((
                 lambda: self.compareImage("testCanvasNullFX1", False),
                 setOpacity,
                 lambda: self.compareImage("testCanvasNullFX2", False),
                ))

    def testNodeInCanvasNullFX(self):
        root = self.loadEmptyScene()
        canvas = self.__createOffscreenCanvas()
        avg.ImageNode(parent=root, href="canvas:offscreen")
        node = canvas.getElementByID("test")
        node.setEffect(avg.NullFXNode())
        rect = avg.RectNode(size=(100,100), strokewidth=0, fillcolor="FF0000",
                fillopacity=1)
        canvas.getRootNode().insertChild(rect, 0)
        
        self.start((
                 lambda: self.compareImage("testNodeInCanvasNullFX1", False),
                ))

    def testRenderPipeline(self):
        print
        for useSrcCanvas in (False, True):
            for useDestCanvas in (False, True):
                for useFX in (False, True):
                    for useColorConv in (False, True):
                        print "  ", useSrcCanvas, useDestCanvas, useFX, useColorConv
                        root = self.loadEmptyScene()
                        if useSrcCanvas:
                            srcCanvas = Player.createCanvas(id="src", size=(160,120))
                            avg.ImageNode(href="rgb24alpha-64x64.png", 
                                    parent=srcCanvas.getRootNode())
                            srcImg = avg.ImageNode(href="canvas:src")
                        else:
                            srcImg = avg.ImageNode(href="rgb24alpha-64x64.png")
                        if useFX:
                            srcImg.setEffect(avg.NullFXNode())
                        if useColorConv:
                            srcImg.contrast = (1.01, 1.0, 1.0)
                        if useDestCanvas:
                            destCanvas = Player.createCanvas(id="dest", size=(160,120))
                            destCanvas.getRootNode().appendChild(srcImg)
                            destImg = avg.ImageNode(href="canvas:dest", parent=root)
                        else:
                            root.appendChild(srcImg)
                        self.start((
                                lambda: self.compareImage("testRenderPipeline", False),
                                ))

    def testBlurFX(self):
        
        def setRadius(radius):
            self.effect.radius = radius
        
        def removeFX():
            self.node.setEffect(None)

        def reAddFX():
            self.node.setEffect(self.effect)

        def addNewFX():
            effect = avg.BlurFXNode()
            effect.radius = 8
            self.node.setEffect(effect)

        root = self.loadEmptyScene()
        self.node = avg.ImageNode(parent=root, pos=(10,10), href="rgb24-64x64.png")
        self.effect = avg.BlurFXNode()
        self.node.setEffect(self.effect)
        self.start((
                 lambda: self.compareImage("testBlurFX1", False),
                 lambda: setRadius(8),
                 lambda: self.compareImage("testBlurFX2", False),
                 removeFX,
                 lambda: self.compareImage("testBlurFX3", False),
                 reAddFX,
                 lambda: self.compareImage("testBlurFX2", False),
                 removeFX,
                 addNewFX,
                 lambda: self.compareImage("testBlurFX2", False),
                ))

    def testHueSatFX(self):

        def resetFX():
            self.effect = avg.HueSatFXNode()
            self.node.setEffect(self.effect)

        def setParam(param, value):
            assert(hasattr(self.effect, param))
            setattr(self.effect, param, value)

        root = self.loadEmptyScene()
        self.node = avg.ImageNode(parent=root, pos=(10,10), href="rgb24alpha-64x64.png")
        resetFX()
        self.start((
                lambda: self.compareImage("testHueSatFX1", False),
                lambda: setParam('saturation', -50),
                lambda: self.compareImage("testHueSatFX2", False),
                lambda: setParam('saturation', -100),
                lambda: self.compareImage("testHueSatFX3", False),
                lambda: setParam('saturation', -150),
                lambda: self.compareImage("testHueSatFX3", False),
                resetFX,
                lambda: setParam('hue', 180),
                lambda: self.compareImage("testHueSatFX4", False),
                lambda: setParam('hue', -180),
                lambda: self.compareImage("testHueSatFX4", False),
        ))

    def testInvertFX(self):

        def resetFX():
            self.effect = avg.InvertFXNode()
            self.node.setEffect(self.effect)

        def redAlphaScene():
            self.redRect = avg.RectNode(parent=self.root, pos=(5, 5), fillcolor='FF0000',
                    fillopacity=1, opacity=0, size=(72, 72))
            self.node = avg.ImageNode(parent=self.root, pos=(10,10),
                    href="../graphics/testfiles/rgb24alpha-64x64.png")
            resetFX()

        self.root = self.loadEmptyScene()
        self.node = avg.ImageNode(parent=self.root, pos=(10,10),
                href="../graphics/testfiles/hsl.png")
        resetFX()
        self.start((
                lambda: self.compareImage("testInvertFX1", False),
                redAlphaScene,
                lambda: self.compareImage("testInvertFX2", False),
        ))

    def testShadowFX(self):
        
        def setParams(offset, radius, opacity, color):
            effect.offset = offset
            effect.radius = radius
            effect.opacity = opacity
            effect.color =  color

        root = self.loadEmptyScene()
        rect = avg.RectNode(parent=root, pos=(9.5,9.5), color="0000FF")
        node = avg.ImageNode(parent=root, pos=(10,10), href="shadow.png")
        rect.size = node.size + (1, 1)
        effect = avg.ShadowFXNode()
        node.setEffect(effect)
        self.start((
                 lambda: self.compareImage("testShadowFX1", False),
                 lambda: setParams((0,0), 3, 2, "00FFFF"),
                 lambda: self.compareImage("testShadowFX2", False),
                 lambda: setParams((2,2), 0.1, 1, "FFFFFF"),
                 lambda: self.compareImage("testShadowFX3", False),
                 lambda: setParams((-2,-2), 0.1, 1, "FFFFFF"),
                 lambda: self.compareImage("testShadowFX4", False),
                 lambda: setParams((-2,-2), 3, 1, "FFFFFF"),
                 lambda: self.compareImage("testShadowFX5", False),
                 lambda: setParams((0,0), 0, 1, "FFFFFF"),
                 lambda: self.compareImage("testShadowFX6", False),
                ))

    def testWordsShadowFX(self):
        root = self.loadEmptyScene()
        node = avg.WordsNode(parent=root, pos=(10,10), text="testtext", 
                font="Bitstream Vera Sans")
        effect = avg.ShadowFXNode()
        effect.setParams((0,0), 1.5, 1.5, "FF0000")
        node.setEffect(effect)
        self.start((
                 lambda: self.compareImage("testWordsShadowFX1", True),
                 lambda: effect.setParams((2,2), 2, 2, "00FFFF"),
                 lambda: self.compareImage("testWordsShadowFX2", True),
                ))

    def testGamma(self):
        def setGamma(val):
            node.gamma = val

        root = self.loadEmptyScene()
        node = avg.ImageNode(parent=root, href="colorramp.png", gamma=(0.5,0.5,0.5))
        self.assertEqual(node.gamma, (0.5,0.5,0.5))
        self.start((
                 lambda: self.compareImage("testGamma1", False),
                 lambda: setGamma((1.5,2.0,2.5)),
                 lambda: self.assertEqual(node.gamma, (1.5,2.0,2.5)),
                 lambda: self.compareImage("testGamma2", False),
                ))

    def testIntensity(self):
        def setIntensity(val):
            node.intensity = val

        def showVideo():
            node.unlink(True)
            self.videoNode = avg.VideoNode(parent=root, size=(96,96), threaded=False, 
                    href="../video/testfiles/mpeg1-48x48.mpg", intensity=(0.5,0.5,0.5))
            self.videoNode.play()

        def showText():
            self.videoNode.unlink(True)
            textNode = avg.WordsNode(parent=root, fontsize=24, font="Bitstream Vera Sans",
                    intensity=(0.5,0.5,0.5), text="Half-brightness text.",
                    width=140)

        root = self.loadEmptyScene()
        node = avg.ImageNode(parent=root, href="colorramp.png", intensity=(0.5,0.5,0.5))
        self.assertEqual(node.intensity, (0.5,0.5,0.5))
        Player.setFakeFPS(10)
        self.start((
                 lambda: self.compareImage("testIntensity1", False),
                 lambda: setIntensity((1.5,2.0,2.5)),
                 lambda: self.assertEqual(node.intensity, (1.5,2.0,2.5)),
                 lambda: self.compareImage("testIntensity2", False),
                 showVideo,
                 lambda: self.compareImage("testIntensity3", False),
                 showText,
                 lambda: self.compareImage("testIntensity4", False),
                ))
        Player.setFakeFPS(-1)
        self.videoNode = None

    def testContrast(self):
        def setContrast(val):
            node.contrast = val

        def showVideo():
            node.unlink(True)
            videoNode = avg.VideoNode(parent=root, size=(96,96), threaded=False, 
                    href="../video/testfiles/mpeg1-48x48.mpg", contrast=(0.5,0.5,0.5))
            videoNode.play()

        root = self.loadEmptyScene()
        node = avg.ImageNode(parent=root, href="colorramp.png", contrast=(0.5,0.5,0.5))
        self.assertEqual(node.contrast, (0.5,0.5,0.5))
        Player.setFakeFPS(10)
        self.start((
                 lambda: self.compareImage("testContrast1", False),
                 lambda: setContrast((1.5,2.0,2.5)),
                 lambda: self.assertEqual(node.contrast, (1.5,2.0,2.5)),
                 lambda: self.compareImage("testContrast2", False),
                 showVideo,
                 lambda: self.compareImage("testContrast3", False),
                ))
        Player.setFakeFPS(-1)

    def testFXUpdate(self):
        # This tests if the FX render-on-demand functionality doesn't forget updates.
        def changeTexture():
            node.href = "colorramp.png"

        def addMaskTex():
            node.maskhref = "mask.png"

        def changeMaskTex():
            node.maskhref = "mask2.png"

        def changeMaskPos():
            node.maskpos = (10, 10)

        def changeFX():
            effect.radius = 2

        def addVideo():
            node.unlink(True)
            videoNode = avg.VideoNode(parent=root, threaded=False, size=(96,96),
                    href="../video/testfiles/mpeg1-48x48.mpg")
            effect = avg.BlurFXNode()
            effect.radius = 0
            videoNode.setEffect(effect)
            videoNode.play()

        root = self.loadEmptyScene()
        node = avg.ImageNode(parent=root, href="rgb24alpha-64x64.png")
        effect = avg.BlurFXNode()
        effect.radius = 0
        node.setEffect(effect)
        Player.setFakeFPS(25)
        self.start((
                 changeTexture,
                 lambda: self.compareImage("testFXUpdateTex", False),
                 addMaskTex,
                 lambda: self.compareImage("testFXUpdateMaskTex1", False),
                 changeMaskTex,
                 lambda: self.compareImage("testFXUpdateMaskTex2", False),
                 changeMaskPos,
                 lambda: self.compareImage("testFXUpdateMaskPos", False),
                 changeFX,
                 lambda: self.compareImage("testFXUpdateFX", False),
                 addVideo,
                 None,
                 lambda: self.compareImage("testFXUpdateVideo", False),
                ))

    def __createOffscreenCanvas(self):
        canvas = Player.createCanvas(id="offscreen", size=(160,120))
        root = canvas.getRootNode()
        avg.ImageNode(href="rgb24-32x32.png", parent=root)
        avg.ImageNode(id="test", pos=(32,0), href="rgb24alpha-32x32.png", parent=root)
        return canvas

def areFXSupported():
    sceneString = """<avg id="avg" width="160" height="120"/>"""
    Player.loadString(sceneString)
    root = Player.getRootNode()
    # XXX: The second of the following two lines prevent an opengl error in
    # testImageNullFX on the Mac (Snow Leopard) for some reason. 
    node = avg.ImageNode(href="rgb24-65x65.png", parent=root)
    node = avg.ImageNode(href="rgb24-65x65.png", parent=root)
    node.setEffect(avg.BlurFXNode())
    Player.setTimeout(0, Player.stop)
    try:
        Player.play() 
        return True
    except RuntimeError:
        return False


def fxTestSuite(tests):
    if areFXSupported():
        availableTests = [
                "testImageNullFX",
                "testVideoNullFX",
                "testWordsNullFX",
                "testCanvasNullFX",
                "testNodeInCanvasNullFX",
                "testRenderPipeline",
                "testBlurFX",
                "testHueSatFX",
                "testInvertFX",
                "testShadowFX",
                "testWordsShadowFX",
                "testGamma",
                "testIntensity",
                "testContrast",
                "testFXUpdate",
            ]
    else:
        availableTests = []
    return createAVGTestSuite(availableTests, FXTestCase, tests)

Player = avg.Player.get()
