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

from libavg import avg, player
from testcase import *
import gc

class OffscreenTestCase(AVGTestCase):
    def __init__(self, testFuncName):
        AVGTestCase.__init__(self, testFuncName)
    
    def testCanvasBasics(self):
        def createCanvas(isFirst, canvasName, x):
            canvas = self.__createOffscreenCanvas(canvasName, False)
            canvas.getElementByID("test1").x = x
            node = avg.ImageNode(parent=root, id="imagenode")
            node.href="canvas:"+canvasName
            if isFirst:
                self.assertEqual(canvas.getNumDependentCanvases(), 0)
                self.canvas1 = canvas
            else:
                self.assertEqual(canvas.getNumDependentCanvases(), 1)
                self.canvas2 = canvas

        def unlink():
            self.node = player.getElementByID("imagenode")
            self.node.unlink()
            self.assertEqual(self.canvas1.getNumDependentCanvases(), 0)
            gc.collect()

        def relink():
            root.appendChild(self.node)
            self.node = None
            self.assertEqual(self.canvas1.getNumDependentCanvases(), 1)
            
        def changeHRef(href):
            player.getElementByID("imagenode").href = href

        def setBitmap():
            bitmap = avg.Bitmap("media/rgb24-65x65.png")
            player.getElementByID("imagenode").setBitmap(bitmap)

        def deleteCanvases():
            changeHRef("")
            firstNode.href = ""
            player.deleteCanvas("testcanvas1")
            self.assertRaises(RuntimeError, lambda: changeHRef("canvas:testcanvas1"))
            changeHRef("canvas:testcanvas2")
            self.assertRaises(RuntimeError, lambda: player.deleteCanvas("testcanvas2"))
            changeHRef("")
            player.deleteCanvas("testcanvas2")
            self.assertRaises(RuntimeError, lambda: player.deleteCanvas("foo"))

        root = self.loadEmptyScene()
        root.mediadir = "media"
        createCanvas(True, "testcanvas1", 0)
        firstNode = player.getElementByID("imagenode")
        self.start(False,
                (lambda: self.compareImage("testOffscreen1"),
                 unlink,
                 lambda: self.compareImage("testOffscreen2"), 
                 relink,
                 lambda: self.compareImage("testOffscreen1"),
                 unlink,
                 lambda: createCanvas(False, "testcanvas2", 80),
                 lambda: self.compareImage("testOffscreen3"),
                 lambda: changeHRef("canvas:testcanvas1"),
                 lambda: self.assertEqual(self.canvas1.getNumDependentCanvases(), 1),
                 lambda: self.assertEqual(self.canvas2.getNumDependentCanvases(), 0),
                 lambda: self.compareImage("testOffscreen1"),
                 lambda: changeHRef("rgb24-65x65.png"),
                 lambda: self.assertEqual(self.canvas1.getNumDependentCanvases(), 0),
                 lambda: self.compareImage("testOffscreen4"),
                 lambda: changeHRef("canvas:testcanvas1"),
                 lambda: self.assertEqual(self.canvas1.getNumDependentCanvases(), 1),
                 lambda: self.compareImage("testOffscreen1"),
                 setBitmap,
                 lambda: self.compareImage("testOffscreen4"),
                 deleteCanvases,
                 lambda: self.compareImage("testOffscreen5"),
                ))

    def testCanvasLoadAfterPlay(self):
        def createOffscreenCanvas():
            self.__createOffscreenCanvas("offscreencanvas", False)
            self.node = avg.ImageNode(parent=root, href="canvas:offscreencanvas")
    
        root = self.loadEmptyScene()
        self.start(False,
                (createOffscreenCanvas,
                 lambda: self.compareImage("testOffscreen1"),
                ))

    def testCanvasResize(self):
        def setSize():
            self.node.size = (80, 60)

        mainCanvas, offscreenCanvas = self.__setupCanvas(False)
        self.start(False,
                (setSize,
                 lambda: self.compareImage("testCanvasResize")
                ))

    def testCanvasErrors(self):
        self.loadEmptyScene()
        # Missing size
        self.assertRaises(RuntimeError, 
                lambda: player.createCanvas(id="foo"))
        # Duplicate canvas id
        player.createCanvas(id="foo", size=(160, 120))
        self.assertRaises(RuntimeError, 
                lambda: player.createCanvas(id="foo", size=(160, 120)))

    def testCanvasAPI(self):
        def checkMainScreenshot():
            bmp1 = player.screenshot()
            bmp2 = mainCanvas.screenshot()
            self.assert_(self.areSimilarBmps(bmp1, bmp2, 0.01, 0.01))

        def checkCanvasScreenshot():
            bmp = offscreenCanvas.screenshot()
            self.compareBitmapToFile(bmp, "testOffscreenScreenshot")

        def createCompressed():
            avg.ImageNode(href="canvas:offscreencanvas", compression="B5G6R5", 
                    parent=root)

        root = self.loadEmptyScene()
        mainCanvas = player.getMainCanvas()
        self.assertEqual(mainCanvas.getRootNode(), root)
        offscreenCanvas = self.__createOffscreenCanvas("offscreencanvas", False)
        self.assertEqual(offscreenCanvas, player.getCanvas("offscreencanvas"))
        self.assertEqual(offscreenCanvas.getElementByID("test1").href, "rgb24-65x65.png")
        self.assertEqual(offscreenCanvas.getElementByID("missingnode"), None)
        self.assertRaises(RuntimeError, player.screenshot)
        self.assertRaises(RuntimeError, createCompressed)
        self.start(False,
                (checkMainScreenshot,
                 checkCanvasScreenshot))

    def testCanvasEvents(self):
        def onOffscreenImageDown(event):
            self.__offscreenImageDownCalled = True

        def onMainDown(event):
            self.__mainDownCalled = True

        def reset():
            self.__offscreenImageDownCalled = False
            self.__mainDownCalled = False

        def setPos():
            self.node.pos = (80, 60)
            self.node.size = (80, 60)

        mainCanvas, offscreenCanvas = self.__setupCanvas(True)
        offscreenImage = offscreenCanvas.getElementByID("test1")
        offscreenImage.subscribe(avg.Node.CURSOR_DOWN, onOffscreenImageDown)
        player.getRootNode().subscribe(avg.Node.CURSOR_DOWN, onMainDown)
        self.__offscreenImageDownCalled = False
        self.__mainDownCalled = False
        self.start(False,
                (lambda: self.fakeClick(10, 10),
                 lambda: self.assert_(self.__offscreenImageDownCalled),
                 reset,
                 lambda: self.fakeClick(80, 10),
                 lambda: self.assert_(not(self.__offscreenImageDownCalled)),
                 reset,
                 setPos,
                 lambda: self.fakeClick(70, 65),
                 lambda: self.assert_(not(self.__offscreenImageDownCalled)),
                 lambda: self.fakeClick(120, 65),
                 lambda: self.assert_(not(self.__offscreenImageDownCalled)),
                 reset,
                 lambda: self.fakeClick(110, 65),
                 lambda: self.assert_(self.__offscreenImageDownCalled and 
                        self.__mainDownCalled),
                 reset,
                 lambda: self.fakeClick(1, 1),
                 lambda: self.assert_(not(self.__offscreenImageDownCalled) and 
                        self.__mainDownCalled),
                ))

    def testCanvasEventCapture(self):
        def onOffscreenImageDown(event):
            self.__offscreenImageDownCalled = True

        mainCanvas, offscreenCanvas = self.__setupCanvas(True)
        offscreenImage = offscreenCanvas.getElementByID("test1")
        offscreenImage.subscribe(avg.Node.CURSOR_DOWN, onOffscreenImageDown);
        self.__offscreenImageDownCalled = False
        offscreenImage.setEventCapture()
        self.start(False,
                (lambda: self._sendMouseEvent(avg.Event.CURSOR_DOWN, 80, 10),
                 lambda: self.assert_(self.__offscreenImageDownCalled),
                ))
                
    def testCanvasRender(self):
        def createCanvas():
            canvas = player.createCanvas(id="testcanvas", size=(160,120),
                    mediadir="media", autorender=False)
            avg.ImageNode(id="test", href="rgb24-65x65.png", parent=canvas.getRootNode())
            return canvas

        def testEarlyScreenshotException():
            self.assertRaises(RuntimeError, self.__offscreenCanvas.screenshot)

        def renderCanvas():
            self.__offscreenCanvas.render()
            bmp = self.__offscreenCanvas.screenshot()
            self.compareBitmapToFile(bmp, "testOffscreenScreenshot")

        def deleteCanvas():
            player.deleteCanvas("testcanvas")
            self.__offscreenCanvas = None

        def recreateCanvas():
            self.__offscreenCanvas = createCanvas()

        self.loadEmptyScene()
        self.__offscreenCanvas = createCanvas()
        self.assertRaises(RuntimeError, renderCanvas)
        self.start(False,
                (testEarlyScreenshotException,
                 renderCanvas,
                 deleteCanvas,
                 recreateCanvas,
                 renderCanvas
                ))

    def testCanvasAutoRender(self):
        def createCanvas():
            canvas = self.__createOffscreenCanvas("testcanvas", False)
            avg.ImageNode(href="canvas:testcanvas", parent=root)
            return canvas

        def disableAutoRender():
            self.__offscreenCanvas.autorender = False

        def enableAutoRender():
            self.__offscreenCanvas.autorender = True

        def changeContent():
            self.__offscreenCanvas.getElementByID("test1").x = 42

        root = self.loadEmptyScene()
        self.__offscreenCanvas = createCanvas()
        self.start(False,
                (lambda: self.assert_(self.__offscreenCanvas.autorender),
                 lambda: self.compareImage("testOffscreenAutoRender1"),
                 disableAutoRender,
                 lambda: self.assert_(not(self.__offscreenCanvas.autorender)),
                 changeContent,
                 lambda: self.compareImage("testOffscreenAutoRender1"),
                 enableAutoRender,
                 lambda: self.assert_(self.__offscreenCanvas.autorender),
                 lambda: self.compareImage("testOffscreenAutoRender2")
                ))

    def testCanvasCrop(self):
        root = self.loadEmptyScene()
        canvas = player.createCanvas(id="testcanvas", size=(160,120), 
                mediadir="media")
        div = avg.DivNode(pos=(40,30), size=(80,60), crop=True, 
                parent=canvas.getRootNode())
        avg.ImageNode(id="test1", pos=(-32, -32), href="rgb24-65x65.png", parent=div)
        avg.ImageNode(parent=root, href="canvas:testcanvas")
        self.start(False, (lambda: self.compareImage("testCanvasCrop"),))

    def testCanvasAlpha(self):
        root = self.loadEmptyScene()
        canvas = player.createCanvas(id="testcanvas", size=(80,120), mediadir="media")
        avg.ImageNode(id="test1", href="rgb24alpha-64x64.png", 
                parent=canvas.getRootNode())
        avg.RectNode(parent=root, fillcolor="FFFFFF",
                pos=(0.5, 0.5), size=(160, 48), fillopacity=1)
        avg.ImageNode(parent=root, href="canvas:testcanvas")
        avg.ImageNode(parent=root, x=64, href="rgb24alpha-64x64.png")
        self.start(False, (lambda: self.compareImage("testCanvasAlpha"),))
    
    def testCanvasBlendModes(self):
        def createBaseCanvas():
            canvas = player.createCanvas(id="testcanvas", size=(64,64), 
                    mediadir="media")
            avg.ImageNode(href="rgb24alpha-64x64.png", parent=canvas.getRootNode())
            return canvas
       
        root = self.loadEmptyScene()
        createBaseCanvas()
        avg.RectNode(parent=root, pos=(48,0), size=(32, 120), strokewidth=2, 
                fillopacity=1, fillcolor="808080")
        avg.ImageNode(parent=root, href="canvas:testcanvas")
        avg.ImageNode(parent=root, pos=(0,64), href="canvas:testcanvas", 
                opacity=0.6)
        avg.ImageNode(parent=root, pos=(64,0), href="canvas:testcanvas", 
                blendmode="add")
        avg.ImageNode(parent=root, pos=(64,64), href="canvas:testcanvas", 
                opacity=0.6, blendmode="add")
        self.start(False, (lambda: self.compareImage("testCanvasBlendModes"),))

    def testCanvasMultisampling(self):
        def testIllegalSamples(numSamples):
            self.canvas = player.createCanvas(id="brokencanvas", size=(160,120), 
                            multisamplesamples=numSamples)

        def screenshot():
            bmp = self.canvas.screenshot()
            self.compareBitmapToFile(bmp, "testOffscreenMultisampleScreenshot")

        def createCanvas():
            if not(avg.OffscreenCanvas.isMultisampleSupported()):
                self.skip("Offscreen multisampling not supported")
                player.stop()
                return
            try:
                self.canvas = player.createCanvas(id="testcanvas", size=(160,120),
                        mediadir="media", multisamplesamples=2)
                avg.ImageNode(id="test1", href="rgb24-65x65.png", angle=0.1,
                        parent=self.canvas.getRootNode())
            except RuntimeError:
                self.skip("Offscreen multisampling init failed")
                player.stop()
                return
            self.assertEqual(self.canvas.multisamplesamples, 2)
            avg.ImageNode(parent=root, href="canvas:testcanvas")
            

        root = self.loadEmptyScene()
        self.start(False,
                (createCanvas,
                 lambda: self.compareImage("testCanvasMultisample"),
                 screenshot,
                 lambda: self.assertRaises(RuntimeError, lambda: testIllegalSamples(42)),
                 lambda: self.assertRaises(RuntimeError, lambda: testIllegalSamples(0)),
                ))
        self.canvas = None
       
    def testCanvasMipmap(self):
        root = self.loadEmptyScene()

        canvas = player.createCanvas(id="testcanvas", size=(80,120), mediadir="media",
                mipmap=True)
        avg.ImageNode(id="test1", href="rgb24alpha-64x64.png", 
                parent=canvas.getRootNode())
        avg.ImageNode(parent=root, size=(40, 30), href="canvas:testcanvas")
        try:
            self.start(False, (lambda: self.compareImage("testCanvasMipmap"),))
        except RuntimeError:
            self.skip("Offscreen mipmap init failed.")
            return

    def testCanvasDependencies(self):
        def makeCircularRef():
            self.offscreen1.getElementByID("test1").href = "canvas:offscreencanvas2"
            
        def makeSelfRef1():
            avg.ImageNode(href="canvas:offscreencanvas1", 
                    parent=self.offscreen1.getRootNode())

        def makeSelfRef2():
            self.offscreen1.getElementByID("test1").href = "canvas:offscreencanvas1"

        def createTwoCanvases():
            self.offscreen1 = self.__createOffscreenCanvas("offscreencanvas1", False)
            self.offscreen2 = self.__createOffscreenCanvas("offscreencanvas2", False)
            self.node = avg.ImageNode(parent=root, 
                    href="canvas:offscreencanvas1")
            node = self.offscreen1.getElementByID("test1")
            node.href = "canvas:offscreencanvas2"
            node.size = (80, 60)
            
        def exchangeCanvases():
            self.offscreen1.getElementByID("test1").href = "rgb24-65x65.png"
            self.offscreen2.getElementByID("test1").href = "canvas:offscreencanvas1"
            self.node.href = "canvas:offscreencanvas2"
            
        def loadCanvasDepString():
            player.createCanvas(id="canvas1", size=(160, 120))
            canvas2 = player.createCanvas(id="canvas2", size=(160, 120))
            avg.ImageNode(href="canvas:canvas1", parent=canvas2.getRootNode())
            player.deleteCanvas('canvas2')
            player.deleteCanvas('canvas1')

        root = self.loadEmptyScene()
        createTwoCanvases()
        self.offscreen1.getElementByID("test1").href = ""
        self.offscreen1 = None
        self.offscreen2 = None
        self.node.href = ""
        self.node = None
        player.deleteCanvas("offscreencanvas1")
        player.deleteCanvas("offscreencanvas2")
        self.start(False,
                (createTwoCanvases,
                 lambda: self.compareImage("testCanvasDependencies1"),
                 exchangeCanvases,
                 lambda: self.compareImage("testCanvasDependencies2"),
                 lambda: self.assertRaises(RuntimeError, makeCircularRef),
                 lambda: self.assertRaises(RuntimeError, makeSelfRef1),
                 lambda: self.assertRaises(RuntimeError, makeSelfRef2),
                 loadCanvasDepString,
                ))

    def __setupCanvas(self, handleEvents):
        root = self.loadEmptyScene()
        mainCanvas = player.getMainCanvas()
        offscreenCanvas = self.__createOffscreenCanvas("offscreencanvas", handleEvents)
        self.node = avg.ImageNode(parent=root, href="canvas:offscreencanvas")
        return (mainCanvas, offscreenCanvas)

    def __createOffscreenCanvas(self, canvasName, handleEvents):
        canvas = player.createCanvas(id=canvasName, size=(160,120), 
                handleevents=handleEvents)
        canvas.getRootNode().mediadir = "media"
        avg.ImageNode(id="test1", href="rgb24-65x65.png", parent=canvas.getRootNode())
        return canvas

def isOffscreenSupported():
    
    def testOffscreenSupported():
        global offscreenSupported
        offscreenSupported = avg.OffscreenCanvas.isSupported()
        player.stop()

    global offscreenSupported
    sceneString = """<avg id="avg" width="160" height="120"/>"""
    player.loadString(sceneString)
    player.setTimeout(0, testOffscreenSupported)
    player.play() 
    return offscreenSupported

def offscreenTestSuite(tests):
    if isOffscreenSupported():
        availableTests = (
                "testCanvasBasics",
                "testCanvasLoadAfterPlay",
                "testCanvasResize",
                "testCanvasErrors",
                "testCanvasAPI",
                "testCanvasEvents",
                "testCanvasEventCapture",
                "testCanvasRender",
                "testCanvasAutoRender",
                "testCanvasCrop",
                "testCanvasAlpha",
                "testCanvasBlendModes",
                "testCanvasMultisampling",
                "testCanvasMipmap",
                "testCanvasDependencies",
                )
        return createAVGTestSuite(availableTests, OffscreenTestCase, tests)
    else:
        sys.stderr.write("Skipping offscreen tests - no canvas support with this graphics configuration.\n")
        return unittest.TestSuite()
