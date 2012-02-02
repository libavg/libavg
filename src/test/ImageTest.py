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


import shutil

from libavg import avg
from testcase import *

g_IsMaskSupported = None

class ImageTestCase(AVGTestCase):
    def __init__(self, testFuncName):
        AVGTestCase.__init__(self, testFuncName)

    def testImageHRef(self):
        def createXmlNode(pos):
            node = Player.createNode(
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
                shutil.copyfile("oe.png", u"ö.png")
                node = createXmlNode((16, 16))
                root.appendChild(node)
                node.href = u"ö.png"
                os.remove(u"ö.png")

        def compareUnicode():
            if self._isCurrentDirWriteable():
                self.compareImage("testImgHRef3", False)

        root = self.loadEmptyScene()
        addNodes(16)
        self.start((
                 lambda: self.compareImage("testImgHRef1", False),
                 lambda: addNodes(48),
                 lambda: self.compareImage("testImgHRef2", False),
                 setUnicodeHref,
                 compareUnicode
                ))
      
    def testImagePos(self):
        def createXmlNode(pos):
            return Player.createNode(
                    """<image pos="%s" href="rgb24-32x32.png"/>"""%str(pos))        

        def createDictNode(root, p):
            return avg.ImageNode(pos=p, href="rgb24-32x32.png", parent=root)

        def illegalMove(node):
            self.assertException(node.pos.x == 23)
            self.assertException(node.pos.y == 23)

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
        self.start((
                 lambda: self.compareImage("testImgPos1", False),
                 lambda: addNodes(48),
                 lambda: self.compareImage("testImgPos2", False),
                ))

    def testImageSize(self):
        def createXmlNode(pos, size):
            return Player.createNode(
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
        self.start((
                 lambda: self.compareImage("testImgSize1", False),
                 lambda: addNodes(48),
                 lambda: self.compareImage("testImgSize2", False),
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
            self.assertException(node.getWarpedVertexCoords)
            node.unlink()

        def addNode():
            node = createNode((16, 16))
            root.appendChild(node)
            moveVertex(node)
        
        root = self.loadEmptyScene()
        testEarlyAccessException()
        self.start((
                 lambda: addNode(),
                 lambda: self.compareImage("testImgWarp1", False),
                ))

    def testBitmap(self):
        def getBitmap(node):
            bmp = node.getBitmap()
            self.assertEqual(bmp.getSize(), (65,65))
            self.assert_(bmp.getFormat() == avg.R8G8B8X8 or 
                    bmp.getFormat() == avg.B8G8R8X8)
            node.setBitmap(bmp)
            self.assertEqual(node.getMediaSize(), (65,65))
        
        def loadFromBitmap(p, orighref):
            node = avg.ImageNode(pos=p, size=(32, 32), href=orighref)
            bmp = avg.Bitmap('rgb24-65x65.png')
            self.assertEqual(bmp.getSize(), (65,65))
            node.setBitmap(bmp)
            self.assertEqual(node.getMediaSize(), (65,65))
            root.appendChild(node)
        
        def testStringConversion():
            bmp = avg.Bitmap('rgb24-65x65.png')
            s = bmp.getPixels()
            bmp1 = avg.Bitmap(bmp.getSize(), avg.B8G8R8X8, "sample")
            bmp1.setPixels(s)
            self.assert_(self.areSimilarBmps(bmp, bmp1, 0.01, 0.01))

        def testUnicode():
            if self._isCurrentDirWriteable():
                # Can't check unicode filenames into svn or the windows client breaks.
                # So we rename the file locally.
                shutil.copyfile("oe.png", u"ö.png")
                bmp = avg.Bitmap(u"ö.png")
                os.remove(u"ö.png")

        def testGetPixel():
            bmp = avg.Bitmap('rgb24-65x65.png')
            self.assertEqual(bmp.getPixel((1,1)), (255,0,0,255))
            self.assertEqual(bmp.getPixel((33,1)), (0,255,0,255))
            bmp = avg.Bitmap('rgb24alpha-64x64.png')
            self.assertEqual(bmp.getPixel((1,1)), (0,0,0,0))
            self.assertEqual(bmp.getPixel((63,1)), (83,255,83,142))
            bmp = avg.Bitmap('greyscale.png')
            self.assertEqual(bmp.getPixel((1,1)), (255,255,255,255))
            self.assertEqual(bmp.getPixel((1,63)), (0,0,0,255))
            self.assertException(lambda: bmp.getPixel((64,0)))

        def setNullBitmap():
            node.setBitmap(None)

        node = avg.ImageNode(href="rgb24-65x65.png", size=(32, 32))
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
        self.start((
                 lambda: getBitmap(node),
                 lambda: loadFromBitmap((32,32), ""),
                 lambda: loadFromBitmap((64,32), "rgb24alpha-64x64.png"),
                 lambda: self.compareImage("testBitmap1", False),
                 testGetPixel,
                 lambda: self.assertException(setNullBitmap)
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
                Player.setTimeout(0, loadUnexistentBitmap)

            avg.BitmapManager.get().loadBitmap("rgb24alpha-64x64.png", validBitmapCb)

        def loadUnexistentBitmap():
            avg.BitmapManager.get().loadBitmap("nonexistent.png",
                    lambda bmp: expectException(
                            returnValue=bmp,
                            nextAction=lambda: Player.setTimeout(0, loadBrokenImage)))

        def loadBrokenImage():
            import tempfile
            tempFileName = os.path.join(tempfile.gettempdir(),
                    "broken.png")
            open(tempFileName, "w")

            def cleanupAndTestReturnValue(returnValue):
                os.unlink(tempFileName)
                expectException(returnValue=returnValue, nextAction=Player.stop)

            avg.BitmapManager.get().loadBitmap(tempFileName,
                    cleanupAndTestReturnValue)
        
        def reportStuck():
            raise RuntimeError("BitmapManager didn't reply "
                    "within %dms timeout" % WAIT_TIMEOUT)
            Player.stop()
            
        root = self.loadEmptyScene()
        
        Player.setTimeout(WAIT_TIMEOUT, reportStuck)
        Player.setResolution(0, 0, 0, 0)
        loadValidBitmap()
        Player.play()
        
    def testBlendMode(self):
        def setBlendMode():
            blendNode.blendmode="add"
        
        root = self.loadEmptyScene()
        avg.ImageNode(href="freidrehen.jpg", parent=root)
        blendNode = avg.ImageNode(opacity=0.6, href="rgb24-65x65.png", parent=root)
        avg.ImageNode(pos=(0,48), opacity=0.6, href="rgb24-65x65.png", blendmode="add",
                parent=root)
        avg.ImageNode(pos=(48,0), opacity=1, href="rgb24-65x65.png", blendmode="min",
                parent=root)
        avg.ImageNode(pos=(48,48), opacity=1, href="rgb24-65x65.png", blendmode="max",
                parent=root)

        self.start((
                 lambda: self.compareImage("testBlend1", False),
                 setBlendMode,
                 lambda: self.compareImage("testBlend2", False)
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
            
        if not(self._hasShaderSupport()):
            return
        root = self.loadEmptyScene()
        createNode((0,0))
        node = root.getChild(0)
        setNoAttach((32,0))
        setAttach((64,0))
        self.start((
                 lambda: createNode((0, 32)),
                 lambda: setNoAttach((32,32)),
                 lambda: setAttach((64,32)),
                 lambda: self.compareImage("testImgMask1", False),
                 changeHRef,
                 lambda: self.compareImage("testImgMask2", False),
                 changeBaseHRef,
                 lambda: self.compareImage("testImgMask3", False),
                 setMaskNotFound
                ))

    def testImageMaskCanvas(self):
        if not(self._hasShaderSupport()):
            return
        root = self.loadEmptyScene()
        canvas = Player.createCanvas(id="testcanvas", size=(64,64))
        avg.ImageNode(href="rgb24-64x64.png", parent=canvas.getRootNode())
        avg.RectNode(size=(160,120), fillcolor="FFFFFF", fillopacity=1, parent=root)
        node = avg.ImageNode(href="canvas:testcanvas", maskhref="mask.png", parent=root)
        self.start([lambda: self.compareImage("testImgMaskCanvas", False)])

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
        if not(self._hasShaderSupport()):
            return
        createNode((0,0))
        setNoAttach((32,0))
        setAttach((64,0))
        self.start((
                 lambda: createNode((0, 32)),
                 lambda: setNoAttach((32,32)),
                 lambda: setAttach((64,32)),
                 lambda: self.compareImage("testImgMaskPos", False)
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
        if not(self._hasShaderSupport()):
            return
        createNode((0,0))
        node = root.getChild(0)
        setNoAttach((32,0))
        setAttach((64,0))
        self.start((
                 lambda: createNode((0, 32)),
                 lambda: setNoAttach((32,32)),
                 lambda: setAttach((64,32)),
                 lambda: self.compareImage("testImgMaskSize1", False),
                 setPos,
                 lambda: self.compareImage("testImgMaskSize2", False),
                 resetPos,
                 lambda: self.compareImage("testImgMaskSize3", False)
                ))

    def testImageMipmap(self):
        root = self.loadEmptyScene()
        avg.ImageNode(size=(64,64), href="checker.png", parent=root)
        avg.ImageNode(pos=(64,0), size=(64,64), href="checker.png", mipmap=True, 
                parent=root)
        self.start([lambda: self.compareImage("testMipmap", False)])

    def testImageCompression(self):
        def loadBitmap():
            bmp = avg.Bitmap("colorramp.png")
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
        self.start([
                 lambda: self.compareImage("testTexCompression1", False),
                 loadBitmap,
                 lambda: self.compareImage("testTexCompression2", False),
                 relink,
                 lambda: self.compareImage("testTexCompression2", False),
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

#        spline = avg.CubicSpline([(0,1),(1,2),(2,1)], True)
#        self.assertAlmostEqual(spline.interpolate(0), 1)
#        self.assertAlmostEqual(spline.interpolate(0.5), 1.5)
#        self.assertAlmostEqual(spline.interpolate(2), 1)
#        self.assertAlmostEqual(spline.interpolate(3), 2)

def imageTestSuite(tests):
    availableTests = (
            "testImageHRef",
            "testImagePos",
            "testImageSize",
            "testImageWarp",
            "testBitmap",
            "testBitmapManager",
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

Player = avg.Player.get()

