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

import sys, time, platform, os.path, shutil
import math

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

from testcase import *

g_IsMaskSupported = None

class ImageTestCase(AVGTestCase):
    def __init__(self, testFuncName):
        AVGTestCase.__init__(self, testFuncName, 24)

    def testImageHRef(self):
        def createXmlNode(pos):
            node = Player.createNode(
                    """<image pos="%s" href="rgb24-32x32.png"/>"""%str(pos))
            self.assert_(node.getMediaSize() == avg.Point2D(32, 32))
            return node

        def createDictNode(pos):
            node = Player.createNode("image", {"pos":pos, "href":"rgb24-32x32.png"})
            self.assert_(node.getMediaSize() == avg.Point2D(32, 32))
            self.assert_(node.size == avg.Point2D(32, 32))
            return node

        def addNodes(y):
            root = Player.getRootNode()
            
            xmlNode = createXmlNode((16, y))
            root.appendChild(xmlNode)
            
            dictNode = createDictNode((48, y))
            root.appendChild(dictNode)
            
            noAttachNode = createXmlNode((80, y))
            noAttachNode.href = "rgb24alpha-32x32.png"
            self.assert_(noAttachNode.getMediaSize() == avg.Point2D(32, 32))
            self.assert_(noAttachNode.size == avg.Point2D(32,32))
            root.appendChild(noAttachNode)

            attachNode = createXmlNode((112, y))
            root.appendChild(attachNode)
            attachNode.href = "rgb24alpha-32x32.png"
            self.assert_(attachNode.getMediaSize() == avg.Point2D(32, 32))
            self.assert_(attachNode.size == avg.Point2D(32,32))

        def setUnicodeHref():
            if isDirWritable():
                root = Player.getRootNode()
                # Can't check unicode filenames into svn or the windows client breaks.
                # So we rename the file locally.
                shutil.copyfile("oe.png", u"ö.png")
                node = createXmlNode((16, 16))
                root.appendChild(node)
                node.href = u"ö.png"
                os.remove(u"ö.png")

        def compareUnicode():
            if isDirWritable():
                self.compareImage("testImgHRef3", False)

        self._loadEmpty()
        addNodes(16)
        self.start(None,
                (lambda: self.compareImage("testImgHRef1", False),
                 lambda: addNodes(48),
                 lambda: self.compareImage("testImgHRef2", False),
                 setUnicodeHref,
                 compareUnicode
                ))
      
    def testImagePos(self):
        def createXmlNode(pos):
            return Player.createNode(
                    """<image pos="%s" href="rgb24-32x32.png"/>"""%str(pos))        

        def createDictNode(pos):
            return Player.createNode("image", {"pos":pos, "href":"rgb24-32x32.png"})

        def illegalMove(node):
            self.assertException(node.pos.x == 23)
            self.assertException(node.pos.y == 23)

        def addNodes(y):
            root = Player.getRootNode()
            xmlNode = createXmlNode((16, y))
            root.appendChild(xmlNode)
            dictNode = createDictNode((48, y))
            root.appendChild(dictNode)
            noAttachNode = createXmlNode((0, 0))
            noAttachNode.pos = avg.Point2D(80, y)
            illegalMove(noAttachNode)
            root.appendChild(noAttachNode)
            attachNode = createXmlNode((0, 0))
            root.appendChild(attachNode)
            attachNode.pos = avg.Point2D(112, y)
            illegalMove(attachNode)

        self._loadEmpty()
        addNodes(16)
        self.start(None,
                (lambda: self.compareImage("testImgPos1", False),
                 lambda: addNodes(48),
                 lambda: self.compareImage("testImgPos2", False),
                ))

    def testImageSize(self):
        def createXmlNode(pos, size):
            return Player.createNode(
                    """<image pos="%s" size="%s" href="rgb24-64x64.png"/>"""
                    %(str(pos), str(size))) 

        def createDictNode(pos, size):
            return Player.createNode("image", {"pos":pos, "size":size,
                    "href":"rgb24-64x64.png"}) 

        def addNodes(y):
            root = Player.getRootNode()
            xmlNode = createXmlNode((16, y), (32, 32))
            self.assert_(xmlNode.size == avg.Point2D(32, 32))
            root.appendChild(xmlNode)
            dictNode = createDictNode((48, y), (32, 32))
            self.assert_(dictNode.size == avg.Point2D(32, 32))
            root.appendChild(dictNode)
            noAttachNode = createXmlNode((80, y), (0, 0))
            noAttachNode.size = avg.Point2D(32, 32)
            self.assert_(noAttachNode.size == avg.Point2D(32, 32))
            root.appendChild(noAttachNode)
            attachNode = createXmlNode((112, y), (0, 0))
            root.appendChild(attachNode)
            attachNode.size = avg.Point2D(32, 32)
            self.assert_(attachNode.size == avg.Point2D(32, 32))

        self._loadEmpty()
        addNodes(16)
        self.start(None,
                (lambda: self.compareImage("testImgSize1", False),
                 lambda: addNodes(48),
                 lambda: self.compareImage("testImgSize2", False),
                ))
       
    def testImageWarp(self):
        def createNode(pos):
            return Player.createNode("image", {"pos":pos, "href":"rgb24-32x32.png",
                    "maxtilewidth":16, "maxtileheight":8}) 

        def moveVertex(node):
            grid = node.getWarpedVertexCoords()
            grid[0][1] = (grid[0][1][0]+0.25, grid[0][1][1]+0.25)
            node.setWarpedVertexCoords(grid)

        def testEarlyAccessException():
            root = Player.getRootNode()
            node = createNode((16, 16))
            root.appendChild(node)
            self.assertException(node.getWarpedVertexCoords)
            node.unlink()

        def addNode():
            node = createNode((16, 16))
            Player.getRootNode().appendChild(node)
            moveVertex(node)
        
        self._loadEmpty()
        testEarlyAccessException()
        self.start(None,
                (lambda: addNode(),
                 lambda: self.compareImage("testImgWarp1", False),
                ))

    def testBitmap(self):
        def getBitmap(node):
            bmp = node.getBitmap()
            self.assert_(bmp.getSize() == (65,65))
            self.assert_(bmp.getFormat() == avg.R8G8B8X8 or 
                    bmp.getFormat() == avg.B8G8R8X8)
            node.setBitmap(bmp)
            self.assert_(node.getMediaSize() == (65,65))
        
        def loadFromBitmap(pos, orighref):
            node = Player.createNode('image',
                    {'pos':pos, 'size':(32, 32), 'href':orighref})
            bmp = avg.Bitmap('rgb24-65x65.png')
            self.assert_(bmp.getSize() == (65,65))
            node.setBitmap(bmp)
            self.assert_(node.getMediaSize() == (65,65))
            Player.getRootNode().appendChild(node)
        
        def testUnicode():
            if isDirWritable():
                # Can't check unicode filenames into svn or the windows client breaks.
                # So we rename the file locally.
                shutil.copyfile("oe.png", u"ö.png")
                bmp = avg.Bitmap(u"ö.png")
                os.remove(u"ö.png")

        def setNullBitmap():
            node.setBitmap(None)

        node = Player.createNode("image", {"href":"rgb24-65x65.png", "size":(32, 32)})
        getBitmap(node)

        self._loadEmpty()
        node = Player.createNode('image',
                {'pos':(0,0), 'size':(32, 32), 'href':"rgb24-65x65.png"})
        Player.getRootNode().appendChild(node)
        getBitmap(node)
        self.assert_(node.size == (32,32))
        loadFromBitmap((32,0), "")
        loadFromBitmap((64,0), "rgb24alpha-64x64.png")
        testUnicode()
        self.start(None,
                (lambda: getBitmap(node),
                 lambda: loadFromBitmap((32,32), ""),
                 lambda: loadFromBitmap((64,32), "rgb24alpha-64x64.png"),
                 lambda: self.compareImage("testBitmap1", False),
                 lambda: self.assertException(setNullBitmap)
                ))

    def testBlendMode(self):
        def setBlendMode():
            Player.getElementByID("blend").blendmode="add"
        
        Player.loadString("""
            <?xml version="1.0"?>
            <!DOCTYPE avg SYSTEM "../../doc/avg.dtd">
            <avg width="160" height="120">
                <image x="0" y="0" href="freidrehen.jpg"/>
                <image id="blend" x="0" y="0" opacity="0.6" href="rgb24-65x65.png"/>
                <image x="0" y="48" opacity="0.6" href="rgb24-65x65.png" blendmode="add"/>
                <image x="48" y="0" opacity="0.6" href="rgb24-65x65.png" blendmode="min"/>
                <image x="48" y="48" opacity="0.6" href="rgb24-65x65.png" 
                        blendmode="max"/>
            </avg>
        """)
        self.start(None,
                (lambda: self.compareImage("testBlend1", False),
                 setBlendMode,
                 lambda: self.compareImage("testBlend2", False)
                ))

    def _isMaskSupported(self):
        global g_IsMaskSupported
        if g_IsMaskSupported == None:
            self._loadEmpty()
            node = Player.createNode("image", {"href": "rgb24-65x65.png", 
                    "maskhref": "mask.png"})
            Player.getRootNode().appendChild(node)
            try:
                self.start(None, [])
                g_IsMaskSupported = True
            except RuntimeError:
                g_IsMaskSupported = False
        return g_IsMaskSupported

    def testImageMask(self):
        def createNode(pos):
            node = Player.createNode("image", {"href": "rgb24-65x65.png", 
                    "maskhref": "mask.png", "pos":pos, "size":(32, 32)})
            Player.getRootNode().appendChild(node)

        def setNoAttach(pos):
            node = Player.createNode("image", {"href": "rgb24-65x65.png", 
                    "pos":pos, "size":(32, 32)})
            node.maskhref = "mask.png"
            Player.getRootNode().appendChild(node)

        def setAttach(pos):
            node = Player.createNode("image", {"href": "rgb24-65x65.png", 
                    "pos":pos, "size":(32, 32)})
            Player.getRootNode().appendChild(node)
            node.maskhref = "mask.png"

        def changeHRef():
            node.maskhref = "mask2.png" 

        def changeBaseHRef():
            node.href = "greyscale.png" 

        def setMaskNotFound():
            node.maskhref = "nonexistentmask.png"        
            
        if not(self._isMaskSupported()):
            print "Skipping testImageMask - no shader support."
            return
        self._loadEmpty()
        createNode((0,0))
        node = Player.getRootNode().getChild(0)
        setNoAttach((32,0))
        setAttach((64,0))
        self.start(None,
                (lambda: createNode((0, 32)),
                 lambda: setNoAttach((32,32)),
                 lambda: setAttach((64,32)),
                 lambda: self.compareImage("testImgMask1", False),
                 changeHRef,
                 lambda: self.compareImage("testImgMask2", False),
                 changeBaseHRef,
                 lambda: self.compareImage("testImgMask3", False),
                 setMaskNotFound
                ))

    def testImageMaskPos(self):
        def createNode(pos):
            node = Player.createNode("image", {"href": "rgb24-65x65.png", 
                    "maskhref": "mask.png", "pos":pos, "size":(32, 32), 
                    "maskpos": (32, 32)})
            Player.getRootNode().appendChild(node)
            
        def setNoAttach(pos):
            node = Player.createNode("image", {"href": "rgb24-65x65.png", 
                    "maskhref": "mask.png", "pos":pos, "size":(32, 32)})
            node.maskpos = (32, 32)
            Player.getRootNode().appendChild(node)

        def setAttach(pos):
            node = Player.createNode("image", {"href": "rgb24-65x65.png", 
                    "maskhref": "mask.png", "pos":pos, "size":(32, 32)})
            Player.getRootNode().appendChild(node)
            node.maskpos = (32, 32)

        self._loadEmpty()
        if not(self._isMaskSupported()):
            print "Skipping testImageMaskPos - no shader support."
            return
        createNode((0,0))
        setNoAttach((32,0))
        setAttach((64,0))
        self.start(None,
                (lambda: createNode((0, 32)),
                 lambda: setNoAttach((32,32)),
                 lambda: setAttach((64,32)),
                 lambda: self.compareImage("testImgMaskPos", False)
                ))

    def testImageMaskSize(self):
        def createNode(pos):
            node = Player.createNode("image", {"href": "rgb24-65x65.png", 
                    "maskhref": "mask.png", "pos":pos, "size":(32, 32), 
                    "masksize": (48, 48)})
            Player.getRootNode().appendChild(node)
            
        def setNoAttach(pos):
            node = Player.createNode("image", {"href": "rgb24-65x65.png", 
                    "maskhref": "mask.png", "pos":pos, "size":(32, 32)})
            node.masksize = (48, 48)
            Player.getRootNode().appendChild(node)

        def setAttach(pos):
            node = Player.createNode("image", {"href": "rgb24-65x65.png", 
                    "maskhref": "mask.png", "pos":pos, "size":(32, 32)})
            Player.getRootNode().appendChild(node)
            node.masksize = (48, 48)

        def setPos():
            node.maskpos = (16, 16)

        def resetPos():
            node.maskpos = (0, 0)
            node.masksize = (0, 0)

        self._loadEmpty()
        if not(self._isMaskSupported()):
            print "Skipping testImageMaskPos - no shader support."
            return
        createNode((0,0))
        node = Player.getRootNode().getChild(0)
        setNoAttach((32,0))
        setAttach((64,0))
        self.start(None,
                (lambda: createNode((0, 32)),
                 lambda: setNoAttach((32,32)),
                 lambda: setAttach((64,32)),
                 lambda: self.compareImage("testImgMaskSize1", False),
                 setPos,
                 lambda: self.compareImage("testImgMaskSize2", False),
                 resetPos,
                 lambda: self.compareImage("testImgMaskSize3", False)
                ))

    def testImageMipmap(self):
        Player.loadString("""
            <?xml version="1.0"?>
            <avg id="imageavg" width="160" height="120">
                <image width="64" height="64" href="checker.png"/>
                <image x="64" width="64" height="64" href="checker.png" mipmap="true"/>
            </avg>
        
        """)
        self.start(None, 
                [lambda: self.compareImage("testMipmap", False)
                ])


def imageTestSuite(tests):
    availableTests = (
            "testImageHRef",
            "testImagePos",
            "testImageSize",
            "testImageWarp",
            "testBitmap",
            "testBlendMode",
            "testImageMask",
            "testImageMaskPos",
            "testImageMaskSize",
            "testImageMipmap",
            )
    return AVGTestSuite(availableTests, ImageTestCase, tests)

Player = avg.Player.get()

