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

import time
import math

from libavg import avg
from testcase import *

def almostEqual(a,b):
    try:
        bOk = True
        for i in range(len(a)):
            if math.fabs(a[i]-b[i]) > 0.000001:
                bOk = False
        return bOk
    except:
        return math.fabs(a-b) < 0.000001


class PlayerTestCase(AVGTestCase):
    def __init__(self, testFuncName):
        AVGTestCase.__init__(self, testFuncName)

    def testPoint(self):
        def almostEqual(p1, p2):
            return (abs(p1.x-p2.x) < 0.0001 and abs(p1.y-p2.y) < 0.0001)

        def testHash():
            ptMap = {}
            ptMap[avg.Point2D(0,0)] = 0
            ptMap[avg.Point2D(1,0)] = 1
            ptMap[avg.Point2D(0,0)] = 2
            self.assert_(len(ptMap) == 2)
            self.assert_(ptMap[avg.Point2D(0,0)] == 2)

        pt = avg.Point2D(10, 10)
        self.assert_(pt[0]==pt.x)
        self.assert_(pt[1]==pt.y)
        self.assert_(pt == avg.Point2D(10, 10))
        self.assert_(pt == (10, 10))
        self.assert_(pt != avg.Point2D(11, 10))
        self.assert_(str(pt) == "(10,10)")
        pt2 = eval(repr(pt))
        self.assert_(pt2 == pt)
        testHash()
        self.assert_(almostEqual(avg.Point2D(10,0).getNormalized(), avg.Point2D(1,0)))
        self.assert_(almostEqual(pt.getRotated(math.pi, (5,5)), avg.Point2D(0,0)))
        self.assert_(-pt == (-10, -10))
        self.assert_(pt-(10,0) == (0,10))
        self.assert_(pt+(10,0) == (20,10))
        self.assert_(pt*2 == (20,20))
        self.assert_(2*pt == (20,20))
        pt.x = 21
        pt.y = 23
        self.assert_(pt == avg.Point2D(21, 23))
        pt.x -= 11
        pt.y -= 13
        pt += avg.Point2D(10, 10)
        self.assert_(pt == avg.Point2D(20, 20))
        pt -= avg.Point2D(6, 6)
        self.assert_(pt == avg.Point2D(14, 14))
        self.assert_(pt != avg.Point2D(13, 13))
        pt = pt/2.
        self.assert_(pt == avg.Point2D(7, 7))
        pt = avg.Point2D((10, 10))
        self.assert_(pt == (10, 10))
        self.assert_(len(pt) == 2)
        self.assert_(pt[0]==pt.x)
        self.assert_(pt[1]==pt.y)
        self.assertException(lambda: pt[2])
        self.assert_(almostEqual(avg.Point2D(10,0), avg.Point2D.fromPolar(0,10)))
        self.assertException(avg.Point2D(0,0).getNormalized)

    def testBasics(self):
        def getFramerate():
            framerate = Player.getEffectiveFramerate()
            self.assert_(framerate > 0)

        Player.showCursor(0)
        Player.showCursor(1)
        Player.loadString("""
            <avg width="160" height="120">
                <image id="test1" href="rgb24-65x65.png"/>
            </avg>
        """)
        self.start(None,
                 (getFramerate,
                  lambda: self.compareImage("testbasics", False), 
                  lambda: Player.setGamma(0.3, 0.3, 0.3),
                  lambda: Player.showCursor(0),
                  lambda: Player.showCursor(1),
                 ))

    def testDivResize(self):
        def checkSize (w, h):
            self.assert_(node.width == w)
            self.assert_(node.height == h)
            self.assert_(node.size == (w,h))
        
        def setSize (size):
            node.size = size
        
        def setWidth (w):
            node.width = w
        
        def setHeight (h):
            node.height = h

        Player.loadFile("avg.avg")
        node = Player.getElementByID('nestedavg')

        self.start(None,
                (lambda: checkSize(128, 32),
                    lambda: setSize((14,15)),
                    lambda: checkSize(14,15),
                    lambda: setWidth(23),
                    lambda: checkSize(23,15),
                    lambda: setHeight(22),
                    lambda: checkSize(23,22),
                    ))

    def testRotate(self):
        def onOuterDown(Event):
            self.onOuterDownCalled = True
        
        def fakeRotate():
            Player.getElementByID("outer").angle += 0.1
            Player.getElementByID("inner").angle -= 0.1
        
        def testCoordConversions():
            innerNode = Player.getElementByID("inner")
            relPos = innerNode.getRelPos((90, 80))
            self.assert_(almostEqual(relPos, (10, 10)))
            outerNode = Player.getElementByID("outer")
            relPos = outerNode.getRelPos((90, 80))
            self.assert_(almostEqual(relPos[0], 12.332806394528092) and
                    almostEqual(relPos[1], 6.9211188716194592))
            absPos = outerNode.getAbsPos(relPos)
            self.assert_(almostEqual(absPos, (90, 80)))
            self.assert_(outerNode.getElementByPos((10, 10)) == innerNode)
            self.assert_(outerNode.getElementByPos((0, 10)) == outerNode)
            self.assert_(outerNode.getElementByPos((-10, -110)) == None)
        
        def sendEvent(x, y):
            Helper = Player.getTestHelper()
            Helper.fakeMouseEvent(avg.CURSORDOWN, True, False, False,
                        x, y, 1)
        
        def disableCrop():
            Player.getElementByID("outer").crop = False
            Player.getElementByID("inner").crop = False
            
        Player.loadFile("rotate.avg")
        Player.getElementByID("outer").setEventHandler(
                avg.CURSORDOWN, avg.MOUSE, onOuterDown) 
        self.onOuterDownCalled = False
        self.start(None,
                (lambda: self.compareImage("testRotate1", False),
                 testCoordConversions,
                 fakeRotate,
                 lambda: self.compareImage("testRotate1a", False),
                 lambda: sendEvent(85, 70),
                 lambda: self.assert_(not(self.onOuterDownCalled)),
                 lambda: sendEvent(85, 75),
                 lambda: self.assert_(self.onOuterDownCalled),
                 disableCrop,
                 lambda: self.compareImage("testRotate1b", False),
                ))

    def testRotate2(self):
        self.start("rotate2.avg",
                [lambda: self.compareImage("testRotate2", False),
                 ])
        
    def testRotate3(self):
        self.start("rotate3.avg",
                [lambda: self.compareImage("testRotate3", False)])

    def testRotatePivot(self):
        def setPivot (pos):
            node.pivot = pos
        
        def addPivot (offset):
            node.pivot += offset
        
        Player.loadFile("rotate3.avg")
        node = Player.getElementByID('div1')
        self.start(None, (
            lambda: setPivot((10, 10)),
            lambda: self.compareImage("testRotatePivot1", False),
            lambda: addPivot((-8, 0)),
            lambda: self.compareImage("testRotatePivot2", False),
           ))

    def testOutlines(self):
        Player.loadFile("rotate.avg")
        Player.getRootNode().elementoutlinecolor = "FFFFFF"
        Player.getElementByID("inner").width = 10000
        Player.getElementByID("inner").height = 10000
        self.start(None, 
                [lambda: self.compareImage("testOutlines", False)
                ])

    def testError(self):
        Player.loadFile("image.avg")
        Player.setTimeout(1, lambda: undefinedFunction)
        Player.setTimeout(50, Player.stop)
        try:
            Player.play()
        except NameError:
            self.assert_(1)
        else:
            self.assert_(0)

    def testExceptionInTimeout(self):
        def throwException():
            raise ZeroDivisionError
        
        try:
            self.start("image.avg",
                    [throwException])
        except ZeroDivisionError:
            self.assert_(1)
        else:
            self.assert_(0)

    def testInvalidImageFilename(self):
        def activateNode():
            Player.getElementByID("enclosingdiv").active = 1
        
        self.start("invalidfilename.avg",
                [activateNode])

    def testInvalidVideoFilename(self):
        def tryplay():
            assertException(lambda: Player.getElementByID("brokenvideo").play())
        
        self.start("invalidvideofilename.avg",
                (lambda: tryplay,
                 lambda: Player.getElementByID("brokenvideo").stop()
                ))

    def testTimeouts(self):
        class TestException(Exception):
            pass
        
        def timeout1():
            Player.clearInterval(self.timeout1ID)
            Player.clearInterval(self.timeout2ID)
            self.timeout1called = True
        
        def timeout2():
            self.timeout2called = True
        
        def wait():
            pass
        
        def throwException():
            raise TestException

        def initException():
            self.timeout3ID = Player.setTimeout(0, throwException)
            
        def setupTimeouts():
            self.timeout1ID = Player.setTimeout(0, timeout1)
            self.timeout2ID = Player.setTimeout(1, timeout2)
            
        self.timeout1called = False
        self.timeout2called = False
        self.__exceptionThrown = False
        try:
            self.start("image.avg",
                    (setupTimeouts,
                     wait,
                     lambda: self.assert_(self.timeout1called),
                     lambda: self.assert_(not(self.timeout2called)),
                     lambda: initException(),
                     wait,
                     wait,
                     wait,
                     wait,
                     wait))
        except TestException:
            self.__exceptionThrown = True
            
        self.assert_(self.__exceptionThrown)
        Player.clearInterval(self.timeout3ID)

    def testPanoImage(self):
        def changeProperties():
            node = Player.getElementByID("pano")
            node.sensorheight=10
            node.sensorwidth=15
            node.focallength=25
        
        def loadImage():
            node = Player.getElementByID("pano")
            node.href = "rgb24-65x65.png"
        self.start("panoimage.avg",
                (lambda: self.compareImage("testPanoImage", False),
                 lambda: time.sleep,
                 changeProperties,
                 loadImage
                ))

    def testBroken(self):
        def testBrokenFile(filename):
            self.assertException(lambda: Player.loadFile(filename))
        
        testBrokenFile("filedoesntexist.avg")
        testBrokenFile("noxml.avg")
        testBrokenFile("noavg.avg")
        testBrokenFile("noavg2.avg")

    def testMove(self):
        def moveit():
            node = Player.getElementByID("nestedimg1")
            node.x += 50
            node.opacity -= 0.7
            node = Player.getElementByID("nestedavg")
            node.x += 50
        
        def checkRelPos():
            RelPos = Player.getElementByID("obscured").getRelPos((50,52))
            self.assert_(RelPos == (0, 0))
        
        self.start("avg.avg",
                (lambda: self.compareImage("testMove1", False),
                 moveit,
                 checkRelPos
                ))

    def testCropImage(self):
        def moveTLCrop():
            node = Player.getElementByID("img")
            node.x = -20
            node.y = -20
        
        def moveBRCrop():
            node = Player.getElementByID("img")
            node.x = 60
            node.y = 40
        
        def moveTLNegative():
            node = Player.getElementByID("img")
            node.x = -60
            node.y = -50
        
        def moveBRGone():
            node = Player.getElementByID("img")
            node.x = 140
            node.y = 100
        
        def rotate():
            node = Player.getElementByID("img")
            node.x = 10
            node.y = 10
            Player.getElementByID("nestedavg").angle = 1.0
            Player.getElementByID("bkgd").angle = 1.0
        
        self.start("crop2.avg",
                (lambda: self.compareImage("testCropImage1", False),
                 moveTLCrop,
                 lambda: self.compareImage("testCropImage2", False),
                 moveBRCrop,
                 lambda: self.compareImage("testCropImage3", False),
                 moveTLNegative,
                 lambda: self.compareImage("testCropImage4", False),
                 moveBRGone,
                 lambda: self.compareImage("testCropImage5", False),

                 rotate,
                 lambda: self.compareImage("testCropImage6", False),
                 moveTLCrop,
                 lambda: self.compareImage("testCropImage7", False),
                 moveBRCrop,
                 lambda: self.compareImage("testCropImage8", False),
                 moveTLNegative,
                 lambda: self.compareImage("testCropImage9", False),
                 moveBRGone,
                 lambda: self.compareImage("testCropImage10", False)
               ))

    def testCropMovie(self):
        def playMovie():
            node = Player.getElementByID("movie")
            node.play()
        
        def moveTLCrop():
            node = Player.getElementByID("movie")
            node.x = -20
            node.y = -20
        
        def moveBRCrop():
            node = Player.getElementByID("movie")
            node.x = 60
            node.y = 40
        
        def moveTLNegative():
            node = Player.getElementByID("movie")
            node.x = -60
            node.y = -50
        
        def moveBRGone():
            node = Player.getElementByID("movie")
            node.x = 140
            node.y = 100
        
        def rotate():
            node = Player.getElementByID("movie")
            node.x = 10
            node.y = 10
            Player.getElementByID("nestedavg").angle = 1.0
            Player.getElementByID("bkgd").angle = 1.0
        
        Player.setFakeFPS(30)
        self.start("crop.avg",
                (playMovie,
                 lambda: self.compareImage("testCropMovie1", False),
                 moveTLCrop,
                 lambda: self.compareImage("testCropMovie2", False),
                 moveBRCrop,
                 lambda: self.compareImage("testCropMovie3", False),
                 moveTLNegative,
                 lambda: self.compareImage("testCropMovie4", False),
                 moveBRGone,
                 lambda: self.compareImage("testCropMovie5", False),

                 rotate,
                 lambda: self.compareImage("testCropMovie6", False),
                 moveTLCrop,
                 lambda: self.compareImage("testCropMovie7", False),
                 moveBRCrop,
                 lambda: self.compareImage("testCropMovie8", False),
                 moveTLNegative,
                 lambda: self.compareImage("testCropMovie9", False),
                 moveBRGone,
                 lambda: self.compareImage("testCropMovie10", False)
                ))

    def testWarp(self):
        def moveVertex():
            node = Player.getElementByID("testtiles")
            grid = node.getWarpedVertexCoords()
            grid[1][1] = (grid[1][1][0]+0.06, grid[1][1][1]+0.06)
            node.setWarpedVertexCoords(grid)
            node = Player.getElementByID("clogo1")
            grid = node.getWarpedVertexCoords()
            grid[0][0] = (grid[0][0][0]+0.06, grid[0][0][1]+0.06)
            grid[1][1] = (grid[1][1][0]-0.06, grid[1][1][1]-0.06)
            node.setWarpedVertexCoords(grid)
        
        def flip():
            node = Player.getElementByID("testtiles")
            grid = node.getOrigVertexCoords()
            grid = [ [ (1-pos[0], pos[1]) for pos in line ] for line in grid]
            node.setWarpedVertexCoords(grid)
        
        Player.loadFile("video.avg")
        node = Player.getElementByID("testtiles")
        self.assertException(node.getOrigVertexCoords)
        self.assertException(node.getWarpedVertexCoords)
        Player.setFakeFPS(30)
        self.start(None,
                (lambda: Player.getElementByID("clogo1").play(),
                 lambda: self.compareImage("testWarp1", False),
                 moveVertex,
                 lambda: self.compareImage("testWarp2", False),
                 flip,
                 lambda: self.compareImage("testWarp3", False)
                ))

    def testMediaDir(self):
        def createImageNode():
            # Node is not in tree; mediadir should be root node dir.
            node = avg.ImageNode(href="rgb24-64x64.png")
            self.assert_(node.size == avg.Point2D(0,0)) # File not found
            node.href = "rgb24-64x64a.png"
            self.assert_(node.size == avg.Point2D(64,64)) # File found
            node = avg.ImageNode(href="rgb24-64x64.png", width=23, height=42)
            # File not found, but custom size
            self.assert_(node.size == avg.Point2D(23,42))
            node.href = "rgb24-64x64a.png"
            # File found, custom size stays
            self.assert_(node.size == avg.Point2D(23,42))
            node.size = (0,0)
            # File found, custom size cleared. Media size should be used.
            self.assert_(node.size == avg.Point2D(64,64))

        def setDir():
            Player.getElementByID("main").mediadir="../video/testfiles"
        
        def setAbsDir():
            def absDir():
                # Should not find any media here...
                Player.getElementByID("main").mediadir="/testmediadir"

            self.assertException(absDir)
        
        def createNode():
            node = avg.VideoNode(href="mjpeg1-48x48.avi", fps=30)
        
        self.start("mediadir.avg",
                (createImageNode,
                 lambda: Player.getElementByID("video").play(),
                 lambda: self.compareImage("testMediaDir1", False),
                 setDir,
                 lambda: Player.getElementByID("video").play(), 
                 lambda: self.compareImage("testMediaDir2", False),
                 lambda: self.assert_(Player.getElementByID("img").width == 0),
                 createNode,
                 setAbsDir
                ))

    def testMemoryQuery(self):
        self.assert_(avg.getMemoryUsage() != 0)

    def testStopOnEscape(self):
        def pressEscape():
            Helper = Player.getTestHelper()
            escape = 27
            Helper.fakeKeyEvent(avg.KEYDOWN, escape, escape, "escape", escape, 
                    avg.KEYMOD_NONE),
            Helper.fakeKeyEvent(avg.KEYUP, escape, escape, "escape", escape, 
                    avg.KEYMOD_NONE),
        
        def testEscape1():
            Player.stopOnEscape(False)
            pressEscape()
        
        def testEscape2():
            Player.stopOnEscape(True)
            Player.stopOnEscape(False)
            pressEscape()
        
        def testEscape3():
            Player.stopOnEscape(True)
            pressEscape()
        
        def setAlive():
            self.testStopOnEscapeAlive = True

        self.testStopOnEscapeAlive = False
        self.start('avg.avg',
                (testEscape1,
                testEscape2,
                setAlive))
        self.assert_(self.testStopOnEscapeAlive)
        self.start('avg.avg',
                (testEscape3, # this should exit the player
                lambda: self.assert_(False),
                ))

    def testOffscreen(self):
        Player.loadSceneString("""
            <scene width="160" height="120">
                <image id="test1" href="rgb24-65x65.png" angle="0.4"/>
            </scene>
        """)
        self.loadEmptyScene()
        self.start(None, 
                [None, None])


def playerTestSuite(tests):
    availableTests = (
            "testPoint",
            "testBasics",
            "testDivResize",
            "testRotate",
            "testRotate2",
            "testRotate3",
            "testRotatePivot",
            "testOutlines",
            "testError",
            "testExceptionInTimeout",
            "testInvalidImageFilename",
            "testInvalidVideoFilename",
            "testTimeouts",
            "testPanoImage",
            "testBroken",
            "testMove",
            "testCropImage",
            "testCropMovie",
            "testWarp",
            "testMediaDir",
            "testMemoryQuery",
            "testStopOnEscape",
            )
    return createAVGTestSuite(availableTests, PlayerTestCase, tests)

Player = avg.Player.get()
