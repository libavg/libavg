#!/usr/bin/env python
# -*- coding: utf-8 -*-
# libavg - Media Playback Engine.
# Copyright (C) 2003-2014 Ulrich von Zadow
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

from libavg import avg, utils, player
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

        def emptyImageFX():
            node = avg.ImageNode(parent=root, href="", pos=(64,0))
            node.setEffect(avg.NullFXNode())

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

        self.start(False,
                (lambda: self.compareImage("testImageNullFX1"),
                 addBgNode,
                 lambda: self.compareImage("testImageNullFX2"),
                 activateFX,
                 lambda: self.compareImage("testImageNullFX2"),
                 newNode,
                 lambda: self.compareImage("testImageNullFX3"),
                 newFX,
                 lambda: self.compareImage("testImageNullFX3"),
                 emptyImageFX,
                 lambda: utils.initFXCache(10),
                ))

    def testVideoNullFX(self):
        root = self.loadEmptyScene()
        player.setFakeFPS(25)
        node = avg.VideoNode(parent=root, href="mjpeg-48x48.avi",
                threaded=False)
        node.setEffect(avg.NullFXNode())
        node.play()
        self.start(False, (lambda: self.compareImage("testVideoNullFX"),))

    def testWordsNullFX(self):
        root = self.loadEmptyScene()
        node = avg.WordsNode(parent=root, text="testtext", font="Bitstream Vera Sans")
        node.setEffect(avg.NullFXNode())
        node = avg.WordsNode(parent=root, text="testtext", pos=(0,20),
                font="Bitstream Vera Sans")
        self.start(True,
                (lambda: self.compareImage("testWordsNullFX"),
                ))

    def testCanvasNullFX(self):
        def setOuterOpacity():
            node.opacity=0.6

        def setInnerOpacity():
            innerNode = canvas.getElementByID("test")
            innerNode.opacity = 0.0

        root = self.loadEmptyScene()
        canvas = self.__createOffscreenCanvas()
        node = avg.ImageNode(parent=root, href="canvas:offscreen")
        node.setEffect(avg.NullFXNode())
        self.start(False,
                (lambda: self.compareImage("testCanvasNullFX1"),
                 setOuterOpacity,
                 lambda: self.compareImage("testCanvasNullFX2"),
                 setInnerOpacity,
                 lambda: self.compareImage("testCanvasNullFX3"),
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
        
        self.start(False,
                (lambda: self.compareImage("testNodeInCanvasNullFX1"),
                ))

    def testRenderPipeline(self):
        sys.stderr.write("\n")
        for useSrcCanvas in (False, True):
            for useDestCanvas in (False, True):
                for useFX in (False, True):
                    for useColorConv in (False, True):
                        sys.stderr.write("  "+str(useSrcCanvas)+" "+str(useDestCanvas)+
                                " "+str(useFX)+" "+str(useColorConv)+"\n")
                        root = self.loadEmptyScene()
                        if useSrcCanvas:
                            srcCanvas = player.createCanvas(id="src", size=(160,120),
                                    mediadir="media")
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
                            destCanvas = player.createCanvas(id="dest",
                                    size=(160,120), mediadir="media")
                            destCanvas.getRootNode().appendChild(srcImg)
                            avg.ImageNode(href="canvas:dest", parent=root)
                        else:
                            root.appendChild(srcImg)
                        self.start(False,
                                (lambda: self.compareImage("testRenderPipeline"),
                                ))

    def testBlurFX(self):
       
        def setRadius(radius):
            self.effect.radius = radius
        
        def removeFX():
            self.node.setEffect(None)

        def reAddFX():
            self.node.setEffect(self.effect)

        def addNewFX():
            effect = avg.BlurFXNode(8)
            self.node.setEffect(effect)

        def addNewFXKWARGS():
            effect = avg.BlurFXNode(radius=8)
            self.node.setEffect(effect)

        root = self.loadEmptyScene()
        self.node = avg.ImageNode(parent=root, pos=(10,10), href="rgb24-64x64.png")
        self.effect = avg.BlurFXNode()
        self.start(False,
                (self.skipIfMinimalShader,
                 lambda: self.node.setEffect(self.effect),
                 lambda: self.compareImage("testBlurFX1"),
                 lambda: setRadius(8),
                 lambda: self.compareImage("testBlurFX2"),
                 removeFX,
                 lambda: self.compareImage("testBlurFX3"),
                 reAddFX,
                 lambda: self.compareImage("testBlurFX2"),
                 removeFX,
                 addNewFX,
                 lambda: self.compareImage("testBlurFX2"),
                 removeFX,
                 addNewFXKWARGS,
                 lambda: self.compareImage("testBlurFX2"),
                 lambda: setRadius(300),
                ))

    def testHueSatFX(self):

        def resetFX(**kwargs):
            self.effect = avg.HueSatFXNode(**kwargs)
            self.node.setEffect(self.effect)

        def setParam(param, value):
            assert(hasattr(self.effect, param))
            setattr(self.effect, param, value)

        root = self.loadEmptyScene()
        self.node = avg.ImageNode(parent=root, pos=(10,10), href="rgb24alpha-64x64.png")
        resetFX()
        self.start(False,
                (self.skipIfMinimalShader,
                 lambda: self.compareImage("testHueSatFX1"),
                 lambda: setParam('saturation', -50),
                 lambda: self.compareImage("testHueSatFX2"),
                 lambda: setParam('saturation', -100),
                 lambda: self.compareImage("testHueSatFX3"),
                 lambda: setParam('saturation', -150),
                 lambda: self.compareImage("testHueSatFX3"),
                 resetFX,
                 lambda: setParam('hue', 180),
                 lambda: self.compareImage("testHueSatFX4"),
                 lambda: setParam('hue', -180),
                 lambda: self.compareImage("testHueSatFX4"),
                 lambda: resetFX(saturation=-50),
                 lambda: self.compareImage("testHueSatFX2"),
                 lambda: resetFX(saturation=-150),
                 lambda: self.compareImage("testHueSatFX3"),
                 lambda: resetFX(hue=-180),
                 lambda: self.compareImage("testHueSatFX4"),
                ))

    def testInvertFX(self):

        def resetFX():
            self.effect = avg.InvertFXNode()
            self.node.setEffect(self.effect)

        def redAlphaScene():
            self.redRect = avg.RectNode(parent=self.root, pos=(5, 5), fillcolor='FF0000',
                    fillopacity=1, opacity=0, size=(72, 72))
            self.node = avg.ImageNode(parent=self.root, pos=(10,10),
                    href="rgb24alpha-64x64.png")
            resetFX()

        self.root = self.loadEmptyScene()
        self.node = avg.ImageNode(parent=self.root, pos=(10,10), href="hsl.png")
        resetFX()
        self.start(False,
                (self.skipIfMinimalShader,
                 lambda: self.compareImage("testInvertFX1"),
                 redAlphaScene,
                 lambda: self.compareImage("testInvertFX2"),
                ))

    def testShadowFX(self):
        
        def setParams(offset, radius, opacity, color):
            effect.offset = offset
            effect.radius = radius
            effect.opacity = opacity
            effect.color =  color

        def setEffect(**kwargs):
            effect = avg.ShadowFXNode(**kwargs)
            self.node.setEffect(effect)

        root = self.loadEmptyScene()
        rect = avg.RectNode(parent=root, pos=(9.5,9.5), color="0000FF")
        self.node = avg.ImageNode(parent=root, pos=(10,10), href="shadow.png")
        rect.size = self.node.size + (1, 1)
        effect = avg.ShadowFXNode((0,0), 1, 1, "FFFFFF")
        self.start(False,
                (self.skipIfMinimalShader,
                 lambda: self.node.setEffect(effect),
                 lambda: self.compareImage("testShadowFX1"),
                 lambda: setParams((0,0), 3, 2, "00FFFF"),
                 lambda: self.compareImage("testShadowFX2"),
                 lambda: setParams((2,2), 0.1, 1, "FFFFFF"),
                 lambda: self.compareImage("testShadowFX3"),
                 lambda: setParams((-2,-2), 0.1, 1, "FFFFFF"),
                 lambda: self.compareImage("testShadowFX4"),
                 lambda: setParams((-2,-2), 3, 1, "FFFFFF"),
                 lambda: self.compareImage("testShadowFX5"),
                 lambda: setParams((0,0), 0, 1, "FFFFFF"),
                 lambda: self.compareImage("testShadowFX6"),
                 lambda: setEffect(offset=(0,0), radius=3, opacity=2, color="00FFFF"),
                 lambda: self.compareImage("testShadowFX2"),
                 lambda: setEffect(offset=(2,2), radius=0.1, opacity=1, color="FFFFFF"),
                 lambda: self.compareImage("testShadowFX3"),
                 lambda: setEffect(offset=(-2,-2), radius=0.1, opacity=1, color="FFFFFF"),
                 lambda: self.compareImage("testShadowFX4"),
                 lambda: setEffect(offset=(-2,-2), radius=3, opacity=1, color="FFFFFF"),
                 lambda: self.compareImage("testShadowFX5"),
                 lambda: setEffect(offset=(0,0), radius=0, opacity=1, color="FFFFFF"),
                 lambda: self.compareImage("testShadowFX6"),
                ))

    def testWordsShadowFX(self):

        def setParams(offset, radius, opacity, color):
            effect.offset = offset
            effect.radius = radius
            effect.opacity = opacity
            effect.color =  color

        root = self.loadEmptyScene()
        node = avg.WordsNode(parent=root, pos=(10,10), text="testtext", 
                font="Bitstream Vera Sans")
        effect = avg.ShadowFXNode()
        setParams((0,0), 1.5, 1.5, "FF0000")
        self.start(True,
                (self.skipIfMinimalShader,
                 lambda: node.setEffect(effect),
                 lambda: self.compareImage("testWordsShadowFX1"),
                 lambda: setParams((2,2), 2, 2, "00FFFF"),
                 lambda: self.compareImage("testWordsShadowFX2"),
                ))

    def testGamma(self):
        def setGamma(val):
            node.gamma = val

        root = self.loadEmptyScene()
        node = avg.ImageNode(parent=root, href="colorramp.png", gamma=(0.5,0.5,0.5))
        self.assertEqual(node.gamma, (0.5,0.5,0.5))
        self.start(False,
                (lambda: self.compareImage("testGamma1"),
                 lambda: setGamma((1.5,2.0,2.5)),
                 lambda: self.assertEqual(node.gamma, (1.5,2.0,2.5)),
                 lambda: self.compareImage("testGamma2"),
                ))

    def testIntensity(self):
        def setIntensity(val):
            node.intensity = val

        def showVideo():
            node.unlink(True)
            self.videoNode = avg.VideoNode(parent=root, size=(96,96), threaded=False, 
                    href="mpeg1-48x48.mov", intensity=(0.5,0.5,0.5))
            self.videoNode.play()

        root = self.loadEmptyScene()
        node = avg.ImageNode(parent=root, href="colorramp.png", intensity=(0.5,0.5,0.5))
        self.assertEqual(node.intensity, (0.5,0.5,0.5))
        player.setFakeFPS(10)
        self.start(False,
                (lambda: self.compareImage("testIntensity1"),
                 lambda: setIntensity((1.5,2.0,2.5)),
                 lambda: self.assertEqual(node.intensity, (1.5,2.0,2.5)),
                 lambda: self.compareImage("testIntensity2"),
                 showVideo,
                 lambda: self.compareImage("testIntensity3"),
                ))
        player.setFakeFPS(-1)
        self.videoNode = None

    def testWordsIntensity(self):
        root = self.loadEmptyScene()
        avg.WordsNode(parent=root, fontsize=24, font="Bitstream Vera Sans",
                intensity=(0.5,0.5,0.5), text="brightness",
                width=140)
        self.start(True,
                (lambda: self.compareImage("testWordsIntensity"),
                ))


    def testContrast(self):
        def setContrast(val):
            node.contrast = val

        def showVideo():
            node.unlink(True)
            videoNode = avg.VideoNode(parent=root, size=(96,96), threaded=False, 
                    href="mpeg1-48x48.mov", contrast=(0.5,0.5,0.5))
            videoNode.play()

        root = self.loadEmptyScene()
        node = avg.ImageNode(parent=root, href="colorramp.png", contrast=(0.5,0.5,0.5))
        self.assertEqual(node.contrast, (0.5,0.5,0.5))
        player.setFakeFPS(10)
        self.start(False,
                (lambda: self.compareImage("testContrast1"),
                 lambda: setContrast((1.5,2.0,2.5)),
                 lambda: self.assertEqual(node.contrast, (1.5,2.0,2.5)),
                 lambda: self.compareImage("testContrast2"),
                 showVideo,
                 lambda: self.compareImage("testContrast3"),
                ))
        player.setFakeFPS(-1)

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
                    href="mpeg1-48x48.mov")
            effect = avg.BlurFXNode()
            effect.radius = 0
            videoNode.setEffect(effect)
            videoNode.play()

        root = self.loadEmptyScene()
        node = avg.ImageNode(parent=root, href="rgb24alpha-64x64.png")
        effect = avg.BlurFXNode()
        effect.radius = 0
        player.setFakeFPS(25)
        self.start(False,
                (self.skipIfMinimalShader,
                 lambda: node.setEffect(effect),
                 changeTexture,
                 lambda: self.compareImage("testFXUpdateTex"),
                 addMaskTex,
                 lambda: self.compareImage("testFXUpdateMaskTex1"),
                 changeMaskTex,
                 lambda: self.compareImage("testFXUpdateMaskTex2"),
                 changeMaskPos,
                 lambda: self.compareImage("testFXUpdateMaskPos"),
                 changeFX,
                 lambda: self.compareImage("testFXUpdateFX"),
                 addVideo,
                 None,
                 lambda: self.compareImage("testFXUpdateVideo"),
                ))

    def testChromaKeyFX(self):

        def setParams(htol, ltol, stol):
            effect.htolerance = htol
            effect.ltolerance = ltol
            effect.stolerance = stol

        root = self.loadEmptyScene()
        node = avg.ImageNode(parent=root, href="rgb24-64x64.png")
        effect = avg.ChromaKeyFXNode()
        setParams(0.01, 0.01, 0.01)
        self.start(False,
                (self.skipIfMinimalShader,
                 lambda: node.setEffect(effect),
                 lambda: self.compareImage("testChromaKeyFX1"),
                 lambda: setParams(0.2, 0.2, 0.2),
                 lambda: self.compareImage("testChromaKeyFX2"),
                 lambda: effect.__setattr__("color", "FF0000"),
                 lambda: self.compareImage("testChromaKeyFX3"),
                 lambda: effect.__setattr__("spillthreshold", 1),
                 lambda: self.compareImage("testChromaKeyFX4"),
                ))

    def __createOffscreenCanvas(self):
        canvas = player.createCanvas(id="offscreen", size=(160,120), mediadir="media")
        root = canvas.getRootNode()
        avg.ImageNode(href="rgb24-32x32.png", parent=root)
        avg.ImageNode(id="test", pos=(32,0), href="rgb24alpha-32x32.png", parent=root)
        return canvas


def fxTestSuite(tests):
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
            "testWordsIntensity",
            "testContrast",
            "testFXUpdate",
            "testChromaKeyFX",
        ]
    return createAVGTestSuite(availableTests, FXTestCase, tests)
