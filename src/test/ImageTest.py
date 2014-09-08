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


import shutil

from libavg import avg, player
from testcase import *

class ImageTestCase(AVGTestCase):
    def __init__(self, testFuncName):
        AVGTestCase.__init__(self, testFuncName)

    def testImageHRef(self):
        def createXmlNode(pos):
            node = player.createNode(
                    """<image pos="%s" href="rgb24-32x32.png"/>"""%str(pos))
            self.assertEqual(node.getMediaSize(), avg.Point2D(32, 32))
            return node

        def createDictNode(root, p):
            node = avg.ImageNode(pos=p, href="rgb24-32x32.png", parent=root)
            self.assertEqual(node.getMediaSize(), avg.Point2D(32, 32))
            self.assertEqual(node.size, avg.Point2D(32, 32))
            return node

        def addNodes(y):
            xmlNode = createXmlNode((16, y))
            root.appendChild(xmlNode)
            
            createDictNode(root, (48, y))
            
            noAttachNode = createXmlNode((80, y))
            noAttachNode.href = "rgb24alpha-32x32.png"
            self.assertEqual(noAttachNode.getMediaSize(), avg.Point2D(32, 32))
            self.assertEqual(noAttachNode.size, avg.Point2D(32,32))
            root.appendChild(noAttachNode)

            attachNode = createXmlNode((112, y))
            root.appendChild(attachNode)
            attachNode.href = "rgb24alpha-32x32.png"
            self.assertEqual(attachNode.getMediaSize(), avg.Point2D(32, 32))
            self.assertEqual(attachNode.size, avg.Point2D(32,32))

        def setUnicodeHref():
            if self._isCurrentDirWriteable():
                # Can't check unicode filenames into svn or the windows client breaks.
                # So we rename the file locally.
                shutil.copyfile("media/oe.png", u"media/ö.png")
                node = createXmlNode((16, 16))
                root.appendChild(node)
                node.href = u"ö.png"
                os.remove(u"media/ö.png")

        def compareUnicode():
            if self._isCurrentDirWriteable():
                self.compareImage("testImgHRef3")

        root = self.loadEmptyScene()
        addNodes(16)
        self.start(False,
                (lambda: self.compareImage("testImgHRef1"),
                 lambda: addNodes(48),
                 lambda: self.compareImage("testImgHRef2"),
                 setUnicodeHref,
                 compareUnicode
                ))
      
    def testImagePos(self):
        def createXmlNode(pos):
            return player.createNode(
                    """<image pos="%s" href="rgb24-32x32.png"/>"""%str(pos))        

        def createDictNode(root, p):
            return avg.ImageNode(pos=p, href="rgb24-32x32.png", parent=root)

        def illegalMove(node):
            def xMove():
                node.pos.x = 23
            def yMove():
                node.pos.x = 23
            self.assertRaises(AttributeError, xMove)
            self.assertRaises(AttributeError, yMove)

        def addNodes(y):
            xmlNode = createXmlNode((16, y))
            root.appendChild(xmlNode)
            createDictNode(root, (48, y))
            noAttachNode = createXmlNode((0, 0))
            noAttachNode.pos = avg.Point2D(80, y)
            illegalMove(noAttachNode)
            root.appendChild(noAttachNode)
            attachNode = createXmlNode((0, 0))
            root.appendChild(attachNode)
            attachNode.pos = avg.Point2D(112, y)
            illegalMove(attachNode)

        root = self.loadEmptyScene()
        addNodes(16)
        self.start(False,
                (lambda: self.compareImage("testImgPos1"),
                 lambda: addNodes(48),
                 lambda: self.compareImage("testImgPos2"),
                ))

    def testImageSize(self):
        def createXmlNode(pos, size):
            return player.createNode(
                    """<image pos="%s" size="%s" href="rgb24-64x64.png"/>"""
                    %(str(pos), str(size))) 

        def createDictNode(p, s):
            return avg.ImageNode(pos=p, size=s, href="rgb24-64x64.png")

        def addNodes(y):
            xmlNode = createXmlNode((16, y), (32, 32))
            self.assertEqual(xmlNode.size, avg.Point2D(32, 32))
            root.appendChild(xmlNode)
            dictNode = createDictNode((48, y), (32, 32))
            self.assertEqual(dictNode.size, avg.Point2D(32, 32))
            root.appendChild(dictNode)
            noAttachNode = createXmlNode((80, y), (0, 0))
            noAttachNode.size = avg.Point2D(32, 32)
            self.assertEqual(noAttachNode.size, avg.Point2D(32, 32))
            root.appendChild(noAttachNode)
            attachNode = createXmlNode((112, y), (0, 0))
            root.appendChild(attachNode)
            attachNode.size = avg.Point2D(32, 32)
            self.assertEqual(attachNode.size, avg.Point2D(32, 32))

        root = self.loadEmptyScene()
        addNodes(16)
        self.start(False,
                (lambda: self.compareImage("testImgSize1"),
                 lambda: addNodes(48),
                 lambda: self.compareImage("testImgSize2"),
                ))
       
    def testImageWarp(self):
        def createNode(p):
            return avg.ImageNode(pos=p, href="rgb24-32x32.png",
                    maxtilewidth=16, maxtileheight=8) 

        def moveVertex(node):
            grid = node.getWarpedVertexCoords()
            grid[0][1] = (grid[0][1][0]+0.25, grid[0][1][1]+0.25)
            node.setWarpedVertexCoords(grid)

        def testEarlyAccessException():
            node = createNode((16, 16))
            root.appendChild(node)
            self.assertRaises(RuntimeError, node.getWarpedVertexCoords)
            node.unlink()

        def addNode():
            self.node = createNode((16, 16))
            root.appendChild(self.node)
            moveVertex(self.node)
       
        def changeHref():
            self.node.href = "rgb24-65x65.png"
            grid = self.node.getWarpedVertexCoords()
            self.assert_(len(grid) == 10)
            self.assert_(len(grid[0]) == 6)
            

        root = self.loadEmptyScene()
        testEarlyAccessException()
        self.start(False,
                (lambda: addNode(),
                 lambda: self.compareImage("testImgWarp1"),
                 lambda: changeHref(),
                 lambda: self.compareImage("testImgWarp2"),
                ))

    def testBitmap(self):
        def getBitmap(node):
            bmp = node.getBitmap()
            self.assertEqual(bmp.getSize(), (65,65))
            self.compareBitmapToFile(bmp, "rgb24-65x65")
            self.assert_(bmp.getFormat() == avg.R8G8B8X8 or 
                    bmp.getFormat() == avg.B8G8R8X8)
            node.setBitmap(bmp)
            self.assertEqual(node.getMediaSize(), (65,65))
        
        def immediateGetBitmap():
            node = avg.ImageNode(href="rgb24-65x65.png", size=(32, 32), parent=root)
            bmp = node.getBitmap()
            self.compareBitmapToFile(bmp, "rgb24-65x65")
            node.unlink(True)
            node = avg.ImageNode(href="rgb24-65x65.png", size=(32, 32), parent=root)
            node.unlink()
            root.appendChild(node)
            bmp = node.getBitmap()
            self.compareBitmapToFile(bmp, "rgb24-65x65")
            node.unlink(True)


        def loadFromBitmap(p, orighref):
            node = avg.ImageNode(pos=p, size=(32, 32), href=orighref)
            bmp = avg.Bitmap('media/rgb24-65x65.png')
            self.assertEqual(bmp.getSize(), (65,65))
            node.setBitmap(bmp)
            self.assertEqual(node.getMediaSize(), (65,65))
            root.appendChild(node)
        
        def testStringConversion():
            bmp = avg.Bitmap('media/rgb24-65x65.png')
            s = bmp.getPixels()
            bmp1 = avg.Bitmap(bmp.getSize(), bmp.getFormat(), "sample")
            bmp1.setPixels(s)
            self.assert_(self.areSimilarBmps(bmp, bmp1, 0.01, 0.01))

        def testCropRect():
            bmp = avg.Bitmap('media/rgb24-65x65.png')
            bmp1 = avg.Bitmap(bmp, (32,32), (64,64))
            self.assert_(bmp1.getSize() == (32,32))
            node = avg.ImageNode(pos=(96,0), parent=root)
            node.setBitmap(bmp1)

        def testBlt():
            srcBmp = avg.Bitmap('media/rgb24-65x65.png')
            destBmp = avg.Bitmap((65,65), srcBmp.getFormat(), "bmp")
            destBmp.blt(srcBmp, (0,0))
            destBmp.blt(srcBmp, (32,32))
            node = avg.ImageNode(pos=(96,32), size=(32,32), parent=root)
            node.setBitmap(destBmp)
            
        def testResize():
            srcBmp = avg.Bitmap('media/rgb24-32x32.png')
            destBmp = srcBmp.getResized((64,64))
            self.assert_(destBmp.getSize() == (64,64))
            node = avg.ImageNode(pos=(128,0), size=(32,32), parent=root)
            node.setBitmap(destBmp)

        def testUnicode():
            if self._isCurrentDirWriteable():
                # Can't check unicode filenames into svn or the windows client breaks.
                # So we rename the file locally.
                shutil.copyfile("media/oe.png", u"media/ö.png")
                avg.Bitmap(u"media/ö.png")
                os.remove(u"media/ö.png")

        def testGetPixel():
            bmp = avg.Bitmap('media/rgb24-65x65.png')
            self.assertEqual(bmp.getPixel((1,1)), (255,0,0,255))
            self.assertEqual(bmp.getPixel((33,1)), (0,255,0,255))
            bmp = avg.Bitmap('media/rgb24alpha-64x64.png')
            self.assertEqual(bmp.getPixel((1,1)), (0,0,0,0))
            self.assertEqual(bmp.getPixel((63,1)), (83,255,83,142))
            bmp = avg.Bitmap('media/greyscale.png')
            self.assertEqual(bmp.getPixel((1,1)), (255,255,255,255))
            self.assertEqual(bmp.getPixel((1,63)), (0,0,0,255))
            self.assertRaises(RuntimeError, lambda: bmp.getPixel((64,0)))

        def setNullBitmap():
            node.setBitmap(None)

        def testSubBitmap():
            srcBmp = avg.Bitmap('media/rgb24-32x32.png')
            destBmp = avg.Bitmap(srcBmp, (16,16), (32,32))
            self.assertEqual(srcBmp.getPixel((16,16)), destBmp.getPixel((0,0)))
            self.assertRaises(RuntimeError, lambda: avg.Bitmap(srcBmp, (16,16), (16,32)))

        node = avg.ImageNode(href="media/rgb24-65x65.png", size=(32, 32))
        getBitmap(node)

        root = self.loadEmptyScene()
        node = avg.ImageNode(pos=(0,0), size=(32, 32), href="rgb24-65x65.png")
        root.appendChild(node)
        getBitmap(node)
        self.assertEqual(node.size, (32,32))
        loadFromBitmap((32,0), "")
        loadFromBitmap((64,0), "rgb24alpha-64x64.png")
        testStringConversion()
        testUnicode()
        self.start(False,
                (lambda: getBitmap(node),
                 immediateGetBitmap,
                 lambda: loadFromBitmap((32,32), ""),
                 lambda: loadFromBitmap((64,32), "rgb24alpha-64x64.png"),
                 lambda: self.compareImage("testBitmap1"),
                 testCropRect,
                 lambda: self.compareImage("testBitmap2"),
                 testBlt,
                 lambda: self.compareImage("testBitmap3"),
                 testResize,
                 lambda: self.compareImage("testBitmap4"),
                 testGetPixel,
                 lambda: self.assertRaises(RuntimeError, setNullBitmap),
                 testSubBitmap,
                ))

    def testBitmapManager(self):
        WAIT_TIMEOUT = 2000
        def expectException(returnValue, nextAction):
            if isinstance(returnValue, Exception):
                nextAction()
            else:
                raise RuntimeError("Expected exception, got %s (%s)" % (
                        returnValue, type(returnValue)))
            
        def loadValidBitmap():
            def validBitmapCb(bitmap):
                self.assert_(not isinstance(bitmap, Exception))
                player.setTimeout(0, loadBitmapWithPixelFormat)

            avg.BitmapManager.get().loadBitmap("media/rgb24alpha-64x64.png",
                    validBitmapCb)

        def loadBitmapWithPixelFormat():
            def validBitmapCb(bitmap):
                self.assert_(not isinstance(bitmap, Exception))
                self.assert_(bitmap.getFormat() == avg.B5G6R5)
                player.setTimeout(0, loadUnexistentBitmap)

            avg.BitmapManager.get().loadBitmap("media/rgb24alpha-64x64.png",
                    validBitmapCb, avg.B5G6R5)

        def loadUnexistentBitmap():
            avg.BitmapManager.get().loadBitmap("nonexistent.png",
                    lambda bmp: expectException(
                            returnValue=bmp,
                            nextAction=lambda: player.setTimeout(0, loadBrokenBitmap)))

        def loadBrokenBitmap():
            import tempfile
            tempFileName = os.path.join(tempfile.gettempdir(),
                    "broken.png")
            open(tempFileName, "w")

            def cleanupAndTestReturnValue(returnValue):
                os.unlink(tempFileName)
                expectException(returnValue=returnValue, nextAction=player.stop)

            avg.BitmapManager.get().loadBitmap(tempFileName,
                    cleanupAndTestReturnValue)

        def reportStuck():
            raise RuntimeError("BitmapManager didn't reply "
                    "within %dms timeout" % WAIT_TIMEOUT)
            player.stop()
            
        for multithread in [False, True]:
            self.loadEmptyScene()
            if multithread:
                avg.BitmapManager.get().setNumThreads(2)
            player.setTimeout(WAIT_TIMEOUT, reportStuck)
            player.setResolution(0, 0, 0, 0)
            loadValidBitmap()
            player.play()
        avg.BitmapManager.get().setNumThreads(1)
        
    def testBitmapManagerException(self):
        def bitmapCb(bitmap):
            raise RuntimeError

        self.loadEmptyScene()
        avg.BitmapManager.get().loadBitmap("rgb24alpha-64x64.png", bitmapCb),
        self.assertRaises(RuntimeError, player.play)

    def testBlendMode(self):
        def isBlendMinMaxSupported():
            def tryInsertNode():
                try:
                    avg.ImageNode(href="rgb24-65x65.png", blendmode="min", parent=root)
                except RuntimeError:
                    self.supported = False
            root = self.loadEmptyScene()
            self.supported = True
            self.start(False,
                    (tryInsertNode,
                    ))
            return self.supported
            

        def setBlendMode():
            blendNode.blendmode="add"
        
        if not(isBlendMinMaxSupported()):
            self.skip("Blend modes min and max not supported.")
            return
        root = self.loadEmptyScene()
        avg.ImageNode(href="freidrehen.jpg", parent=root)
        blendNode = avg.ImageNode(opacity=0.6, href="rgb24-65x65.png", parent=root)
        avg.ImageNode(pos=(0,48), opacity=0.6, href="rgb24-65x65.png", blendmode="add",
                parent=root)
        avg.ImageNode(pos=(48,0), opacity=1, href="rgb24-65x65.png", blendmode="min",
                parent=root)
        avg.ImageNode(pos=(48,48), opacity=1, href="rgb24-65x65.png", blendmode="max",
                parent=root)

        self.start(False,
                (lambda: self.compareImage("testBlend1"),
                 setBlendMode,
                 lambda: self.compareImage("testBlend2")
                ))

    def testImageMask(self):
        def createNode(p):
            node = avg.ImageNode(href="rgb24-65x65.png", maskhref="mask.png", 
                    pos=p, size=(32, 32))
            root.appendChild(node)

        def setNoAttach(p):
            node = avg.ImageNode(href="rgb24-65x65.png", pos=p, size=(32, 32))
            node.maskhref = "mask.png"
            root.appendChild(node)

        def setAttach(p):
            node = avg.ImageNode(href="rgb24-65x65.png", pos=p, size=(32, 32))
            root.appendChild(node)
            node.maskhref = "mask.png"

        def changeHRef():
            node.maskhref = "mask2.png" 

        def changeBaseHRef():
            node.href = "greyscale.png" 

        def setMaskNotFound():
            node.maskhref = "nonexistentmask.png"        
            
        root = self.loadEmptyScene()
        createNode((0,0))
        node = root.getChild(0)
        setNoAttach((32,0))
        setAttach((64,0))
        self.start(False,
                (lambda: createNode((0, 32)),
                 lambda: setNoAttach((32,32)),
                 lambda: setAttach((64,32)),
                 lambda: self.compareImage("testImgMask1"),
                 changeHRef,
                 lambda: self.compareImage("testImgMask2"),
                 changeBaseHRef,
                 lambda: self.compareImage("testImgMask3"),
                 setMaskNotFound
                ))

    def testImageMaskCanvas(self):
        root = self.loadEmptyScene()
        canvas = player.createCanvas(id="testcanvas", size=(64,64), mediadir="media")
        avg.ImageNode(href="rgb24-64x64.png", parent=canvas.getRootNode())
        avg.RectNode(size=(160,120), fillcolor="FFFFFF", fillopacity=1, parent=root)
        avg.ImageNode(href="canvas:testcanvas", maskhref="mask.png", parent=root)
        self.start(False,
                (lambda: self.compareImage("testImgMaskCanvas"),))

    def testImageMaskPos(self):
        def createNode(p):
            node = avg.ImageNode(href="rgb24-65x65.png", maskhref="mask.png", 
                    pos=p, size=(32, 32), maskpos=(32, 32))
            root.appendChild(node)
            
        def setNoAttach(p):
            node = avg.ImageNode(href="rgb24-65x65.png", maskhref="mask.png", 
                    pos=p, size=(32, 32))
            node.maskpos = (32, 32)
            root.appendChild(node)

        def setAttach(p):
            node = avg.ImageNode(href="rgb24-65x65.png", maskhref="mask.png", 
                    pos=p, size=(32, 32))
            root.appendChild(node)
            node.maskpos = (32, 32)

        root = self.loadEmptyScene()
        createNode((0,0))
        setNoAttach((32,0))
        setAttach((64,0))
        self.start(False,
                (lambda: createNode((0, 32)),
                 lambda: setNoAttach((32,32)),
                 lambda: setAttach((64,32)),
                 lambda: self.compareImage("testImgMaskPos")
                ))

    def testImageMaskSize(self):
        def createNode(p):
            avg.ImageNode(href="rgb24-65x65.png", maskhref="mask.png", 
                    pos=p, size=(32, 32), masksize=(48, 48), parent=root)
            
        def setNoAttach(p):
            node = avg.ImageNode(href="rgb24-65x65.png", maskhref="mask.png", 
                    pos=p, size=(32, 32))
            node.masksize = (48, 48)
            root.appendChild(node)

        def setAttach(p):
            node = avg.ImageNode(href="rgb24-65x65.png", maskhref="mask.png", 
                    pos=p, size=(32, 32), parent=root)
            node.masksize = (48, 48)

        def setPos():
            node.maskpos = (16, 16)

        def resetPos():
            node.maskpos = (0, 0)
            node.masksize = (0, 0)

        root = self.loadEmptyScene()
        createNode((0,0))
        node = root.getChild(0)
        setNoAttach((32,0))
        setAttach((64,0))
        self.start(False,
                (lambda: createNode((0, 32)),
                 lambda: setNoAttach((32,32)),
                 lambda: setAttach((64,32)),
                 lambda: self.compareImage("testImgMaskSize1"),
                 setPos,
                 lambda: self.compareImage("testImgMaskSize2"),
                 resetPos,
                 lambda: self.compareImage("testImgMaskSize3")
                ))

    def testImageMipmap(self):
        root = self.loadEmptyScene()
        avg.ImageNode(size=(64,64), href="checker.png", mipmap=True, parent=root)
        self.start(False,
                (lambda: self.compareImage("testMipmap"),))

    def testImageCompression(self):
        def loadBitmap():
            bmp = avg.Bitmap("media/colorramp.png")
            self.image.setBitmap(bmp)

        def relink():
            self.image.unlink(False)
            root.appendChild(self.image)

        def checkAlpha():
            self.image.href="rgb24alpha-64x64.png"

        root = self.loadEmptyScene()
        self.image = avg.ImageNode(href="rgb24-64x64.png", compression="B5G6R5",
                parent=root)
        self.assertEqual(self.image.compression, "B5G6R5")
        self.start(False, 
                [lambda: self.compareImage("testTexCompression1"),
                 loadBitmap,
                 lambda: self.compareImage("testTexCompression2"),
                 relink,
                 lambda: self.compareImage("testTexCompression2"),
                 checkAlpha,
                ])

    def testSpline(self):
        spline = avg.CubicSpline([(0,3),(1,2),(2,1),(3,0)])
        self.assertAlmostEqual(spline.interpolate(0), 3)
        self.assertAlmostEqual(spline.interpolate(0.5), 2.5)
        self.assertAlmostEqual(spline.interpolate(1), 2)
        self.assertAlmostEqual(spline.interpolate(-1), 4)
        self.assertAlmostEqual(spline.interpolate(4), -1)

        spline = avg.CubicSpline([(2,0),(4,1),(6,3),(8,6)])
        self.assertAlmostEqual(spline.interpolate(2), 0)
        self.assert_(spline.interpolate(3) < 0.5)
        self.assert_(spline.interpolate(3) > 0.0)
        self.assert_(spline.interpolate(7) < 4.5)
        self.assert_(spline.interpolate(7) > 4)


def imageTestSuite(tests):
    availableTests = (
            "testImageHRef",
            "testImagePos",
            "testImageSize",
            "testImageWarp",
            "testBitmap",
            "testBitmapManager",
            "testBitmapManagerException",
            "testBlendMode",
            "testImageMask",
            "testImageMaskCanvas",
            "testImageMaskPos",
            "testImageMaskSize",
            "testImageMipmap",
            "testImageCompression",
            "testSpline",
            )
    return createAVGTestSuite(availableTests, ImageTestCase, tests)
