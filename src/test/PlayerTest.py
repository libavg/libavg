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

import math
import threading

from libavg import avg, player
from testcase import *

class PlayerTestCase(AVGTestCase):
    def __init__(self, testFuncName):
        AVGTestCase.__init__(self, testFuncName)

    def testPoint(self):
        def testHash():
            ptMap = {}
            ptMap[avg.Point2D(0,0)] = 0
            ptMap[avg.Point2D(1,0)] = 1
            ptMap[avg.Point2D(0,0)] = 2
            self.assertEqual(len(ptMap), 2)
            self.assertEqual(ptMap[avg.Point2D(0,0)], 2)
        
        def testToTupleConversion():
            pt = avg.Point2D(10, 20)
            tpl = tuple(pt)
            self.assertEqual(pt.x, tpl[0])
            self.assertEqual(pt.y, tpl[1])

        def testFromTupleConversion():
            tpl = (15, 20)
            pt = avg.Point2D(tpl)
            self.assertEqual(pt.x, tpl[0])
            self.assertEqual(pt.y, tpl[1])

        pt = avg.Point2D()
        self.assertEqual(pt, avg.Point2D(0,0))
        pt = avg.Point2D(10, 20)
        self.assertEqual(pt[0], pt.x)
        self.assertEqual(pt[1], pt.y)
        pt = avg.Point2D(10, 10)
        self.assertEqual(pt, avg.Point2D(10, 10))
        self.assertEqual(pt, (10, 10))
        self.assertEqual(pt, avg.Point2D([10, 10]))
        self.assertNotEqual(pt, avg.Point2D(11, 10))
        self.assertEqual(str(pt), "(10,10)")
        pt2 = eval(repr(pt))
        self.assertEqual(pt2, pt)
        testHash()
        testFromTupleConversion()
        testToTupleConversion()
        self.assertAlmostEqual(avg.Point2D(10,0).getNormalized(), avg.Point2D(1,0))
        self.assertAlmostEqual(pt.getRotated(math.pi, (5,5)), avg.Point2D(0,0))
        self.assertEqual(-pt, (-10, -10))
        self.assertEqual(pt-(10,0), (0,10))
        self.assertEqual(pt+(10,0), (20,10))
        self.assertEqual(pt*2, (20,20))
        self.assertEqual(2*pt, (20,20))
        pt.x = 21
        pt.y = 23
        self.assertEqual(pt, avg.Point2D(21, 23))
        pt.x -= 11
        pt.y -= 13
        pt += avg.Point2D(10, 10)
        self.assertEqual(pt, avg.Point2D(20, 20))
        pt -= avg.Point2D(6, 6)
        self.assertEqual(pt, avg.Point2D(14, 14))
        self.assertNotEqual(pt, avg.Point2D(13, 13))
        pt = pt/2.
        self.assertEqual(pt, avg.Point2D(7, 7))
        pt = avg.Point2D((10, 10))
        self.assertEqual(pt, (10, 10))
        self.assertEqual(len(pt), 2)
        self.assertEqual(pt[0], pt.x)
        self.assertEqual(pt[1], pt.y)
        self.assertRaises(IndexError, lambda: pt[2])
        self.assertAlmostEqual(avg.Point2D(10,0), avg.Point2D.fromPolar(0,10))
        self.assertRaises(RuntimeError, avg.Point2D(0,0).getNormalized)
        # boost ArgumentError can't be caught explicitly
        self.assertRaises(Exception, lambda: avg.Point2D(0,))
        self.assertRaises(Exception, lambda: avg.Point2D(0,1,2))
        for point in ((10,0), (0,10), (-10,0), (0,-10)):
            pt = avg.Point2D(point)
            angle = pt.getAngle()
            norm = pt.getNorm()
            self.assertAlmostEqual(pt, avg.Point2D.fromPolar(angle,norm))

    def testBasics(self):
        def getFramerate():
            framerate = player.getEffectiveFramerate()
            self.assert_(framerate > 0)

        def invalidCreateNode():
            avg.ImageNode(1, 2, 3)

        player.showCursor(0)
        player.showCursor(1)
        root = self.loadEmptyScene()
        avg.ImageNode(href="rgb24-65x65.png", parent=root)
        self.assertRaises(RuntimeError, invalidCreateNode)
        self.start(False,
                (getFramerate,
                 lambda: self.compareImage("testbasics"), 
                 lambda: player.setGamma(0.3, 0.3, 0.3),
                 lambda: player.showCursor(0),
                 lambda: player.showCursor(1),
                ))

    def testColorParse(self):
        def setColor(colorName):
            node.color = colorName

        node = avg.LineNode(pos1=(0.5, 0), pos2=(0.5, 50), color="FF0000")
        setColor("ff00ff")
        self.assertRaises(RuntimeError, lambda: setColor("foo"))
        self.assertRaises(RuntimeError, lambda: setColor("ff00f"))
        self.assertRaises(RuntimeError, lambda: setColor("ff00ffx"))
        self.assertRaises(RuntimeError, lambda: setColor("ff00fx"))

    def testFakeTime(self):
        def checkTime():
            self.assertEqual(player.getFrameTime(), 50)
            self.assertEqual(player.getFrameDuration(), 50)
            self.assertEqual(player.getEffectiveFramerate(), 20)

        self.loadEmptyScene()
        player.setFakeFPS(20)
        self.start(False,
                (checkTime,
                ))

    def testDivResize(self):
        def checkSize (w, h):
            self.assertEqual(node.width, w)
            self.assertEqual(node.height, h)
            self.assertEqual(node.size, (w,h))
        
        def setSize (size):
            node.size = size
        
        def setWidth (w):
            node.width = w
        
        def setHeight (h):
            node.height = h

        self.__initDefaultScene()
        node = player.getElementByID('nestedavg')

        self.start(False,
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
            player.getElementByID("outer").angle += 0.1
            player.getElementByID("inner").angle -= 0.1
        
        def testCoordConversions():
            innerNode = player.getElementByID("inner")
            relPos = innerNode.getRelPos((90, 80))
            self.assertAlmostEqual(relPos, (10, 10))
            outerNode = player.getElementByID("outer")
            relPos = outerNode.getRelPos((90, 80))
            self.assertAlmostEqual(relPos[0], 12.332806394528092)
            self.assertAlmostEqual(relPos[1], 6.9211188716194592)
            absPos = outerNode.getAbsPos(relPos)
            self.assertAlmostEqual(absPos, (90, 80))
            self.assertEqual(outerNode.getElementByPos((10, 10)), innerNode)
            self.assertEqual(outerNode.getElementByPos((0, 10)), outerNode)
            self.assertEqual(outerNode.getElementByPos((-10, -110)), None)
        
        def disableCrop():
            player.getElementByID("outer").crop = False
            player.getElementByID("inner").crop = False
           
        self.__initDefaultRotateScene()
        player.getElementByID("outer").subscribe(avg.Node.CURSOR_DOWN, onOuterDown) 
        self.onOuterDownCalled = False
        self.start(False,
                (lambda: self.compareImage("testRotate1"),
                 testCoordConversions,
                 fakeRotate,
                 lambda: self.compareImage("testRotate1a"),
                 lambda: self._sendMouseEvent(avg.Event.CURSOR_DOWN, 85, 70),
                 lambda: self._sendMouseEvent(avg.Event.CURSOR_UP, 85, 70),
                 lambda: self.assert_(not(self.onOuterDownCalled)),
                 lambda: self._sendMouseEvent(avg.Event.CURSOR_DOWN, 85, 75),
                 lambda: self._sendMouseEvent(avg.Event.CURSOR_UP, 85, 75),
                 lambda: self.assert_(self.onOuterDownCalled),
                 disableCrop,
                 lambda: self.compareImage("testRotate1b"),
                ))

    def testRotate2(self):
        root = self.loadEmptyScene()
        
        div1 = avg.DivNode(pos=(80,0), size=(160,120), pivot=(0,0), angle=1.57, 
                parent=root)
        avg.ImageNode(size=(16,16), href="rgb24-65x65.png", parent=div1)
        div2 = avg.DivNode(pos=(40,0), size=(110,80), pivot=(0,0), angle=1.57,
                crop=True, parent=div1)
        avg.ImageNode(pos=(0,0), size=(16,16), href="rgb24-65x65.png", parent=div2)
        avg.ImageNode(pos=(30,-6), size=(16,16), href="rgb24-65x65.png", parent=div2)
        self.start(False, [lambda: self.compareImage("testRotate2")])
        
    def testRotatePivot(self):
        def setPivot (pos):
            node.pivot = pos
        
        def addPivot (offset):
            node.pivot += offset
        
        root = self.loadEmptyScene()
        node = avg.DivNode(pos=(80,0), size=(160,120), pivot=(0,0), angle=1.57,
                crop=True, parent=root)
        div = avg.DivNode(pos=(40,-20), size=(160,120), pivot=(0,0), angle=0.79,
                crop=True, parent=node)
        avg.ImageNode(pos=(-10,-10), size=(128,128), href="rgb24-65x65.png", parent=div)
        avg.ImageNode(pos=(0,10), size=(32,32), href="rgb24-65x65.png", parent=node)
        self.start(False,
                (lambda: self.compareImage("testRotatePivot1"),
                 lambda: setPivot((10, 10)),
                 lambda: self.compareImage("testRotatePivot2"),
                 lambda: addPivot((-8, 0)),
                 lambda: self.compareImage("testRotatePivot3"),
                ))

    def testOpacity(self):
        root = self.loadEmptyScene()
        avg.ImageNode(pos=(0,0), href="rgb24-65x65.png", opacity=0.5, parent=root)
        avg.RectNode(pos=(0,64), size=(64,64), opacity=0.5, fillopacity=0.5,
                fillcolor="FF0000", strokewidth=2, parent=root)
        div = avg.DivNode(pos=(80,0), opacity=0.5, parent=root)
        avg.ImageNode(pos=(0,0), href="rgb24-65x65.png", parent=div)
        avg.RectNode(pos=(0,64), size=(64,64), opacity=1, fillopacity=1,
                fillcolor="FF0000", strokewidth=2, parent=div)
        self.start(False, 
                (lambda: self.compareImage("testOpacity"),
                )) 

    def testOutlines(self):
        root = self.__initDefaultRotateScene()
        root.elementoutlinecolor = "FFFFFF"
        innerDiv = player.getElementByID("inner")
        innerDiv.size = (0, 0)
        innerDiv.getChild(0).elementoutlinecolor = "00FF00"
        self.start(False, [lambda: self.compareImage("testOutlines")])

    def testWordsOutlines(self):
        root = self.loadEmptyScene()
        root.elementoutlinecolor = "FFFFFF"
        avg.WordsNode(pos=(40,40), alignment="center", text="test", parent=root)
        self.start(True, [lambda: self.compareImage("testWordsOutlines")])

    def testError(self):
        self.initDefaultImageScene()
        player.setTimeout(1, lambda: undefinedFunction)
        player.setTimeout(50, player.stop)
        try:
            player.play()
        except NameError:
            self.assert_(1)
        else:
            self.assert_(0)

    def testExceptionInTimeout(self):
        def throwException():
            raise ZeroDivisionError
        
        try:
            self.initDefaultImageScene()
            self.start(False, [throwException])
        except ZeroDivisionError:
            self.assert_(1)
        else:
            self.assert_(0)

    def testInvalidImageFilename(self):
        def activateNode():
            div.active = 1
        
        root = self.loadEmptyScene()
        div = avg.DivNode(active=False, parent=root)
        avg.ImageNode(href="filedoesntexist.png", parent=div)
        self.start(False, [activateNode])

    def testInvalidVideoFilename(self):
        def tryplay():
            assertRaises(SystemError, lambda: video.play())
        
        root = self.loadEmptyScene()
        video = avg.VideoNode(href="filedoesntexist.avi", parent=root)
        self.start(False,
                (lambda: tryplay,
                 lambda: video.stop()
                ))

    def testTimeouts(self):
        class TestException(Exception):
            pass
        
        def timeout1():
            player.clearInterval(self.timeout1ID)
            player.clearInterval(self.timeout2ID)
            self.timeout1called = True
        
        def timeout2():
            self.timeout2called = True
       
        def onFrame():
            self.numOnFramesCalled += 1
            if self.numOnFramesCalled == 3:
                player.clearInterval(self.intervalID)

        def throwException():
            raise TestException

        def initException():
            self.timeout3ID = player.setTimeout(0, throwException)
            
        def setupTimeouts():
            self.timeout1ID = player.setTimeout(0, timeout1)
            self.timeout2ID = player.setTimeout(1, timeout2)
            self.intervalID = player.setOnFrameHandler(onFrame)
            
        self.timeout1called = False
        self.timeout2called = False
        self.__exceptionThrown = False
        self.numOnFramesCalled = 0
        try:
            self.initDefaultImageScene()

            self.start(False,
                    (setupTimeouts,
                     None,
                     lambda: self.assert_(self.timeout1called),
                     lambda: self.assert_(not(self.timeout2called)),
                     lambda: self.assert_(self.numOnFramesCalled == 3),
                     lambda: initException(),
                     lambda: self.delay(10)
                    ))
        except TestException:
            self.__exceptionThrown = True
            
        self.assert_(self.__exceptionThrown)
        player.clearInterval(self.timeout3ID)


    def testTimeoutOnFrameHandling(self):

        def onTimeOut():
            self.callCount += 1

        def onFrame():
                player.clearInterval(self.longTimeoutId)
                self.longTimeoutId = None
                player.setTimeout(0, onTimeOut)
                player.unsubscribe(player.ON_FRAME, self.onFrameID)

        self.initDefaultImageScene()
        self.callCount = 0
        # long running dummy timeout. Will be removed before the timer elapses.
        self.longTimeoutId = player.setTimeout(10000, lambda: None)
        self.onFrameID = player.subscribe(player.ON_FRAME, onFrame)

        self.start(False,
                (None,
                 None,
                 None,
                 lambda: self.assert_(self.callCount == 1),
                ))


    def testCallFromThread(self):

        def onAsyncCall():
            self.asyncCalled = True

        def threadFunc():
            player.setTimeout(0, onAsyncCall)

        def startThread():
            self.thread = threading.Thread(target=threadFunc)
            self.thread.start()

        self.initDefaultImageScene()
        self.asyncCalled = False
        player.setFakeFPS(-1)
        self.start(False,
                (startThread,
                 lambda: self.thread.join(),
                 None,
                 lambda: self.assert_(self.asyncCalled),
                ))

    def testAVGFile(self):
        player.loadFile("image.avg")
        self.start(False, 
                (lambda: self.compareImage("testAVGFile"),
                ))
        self.assertRaises(RuntimeError, lambda: player.loadFile("filedoesntexist.avg"))

    def testBroken(self):
        def testBrokenString(string):
            self.assertRaises(RuntimeError, lambda: player.loadString(string))
        
        # This isn't xml
        testBrokenString("""
            xxx<avg width="400" height="300">
            </avg>
        """)
        # This isn't avg
        testBrokenString("""
            <bla>hallo
            </bla>""")
        testBrokenString("""
            <avg width="640" height="480" invalidattribute="bla">
            </avg>
        """)

    def testMove(self):
        def moveit():
            node = player.getElementByID("nestedimg1")
            node.x += 50
            node.opacity -= 0.7
            node = player.getElementByID("nestedavg")
            node.x += 50
        
        def checkRelPos():
            RelPos = player.getElementByID("obscured").getRelPos((50,52))
            self.assertEqual(RelPos, (0, 0))
      
        self.__initDefaultScene()
        self.start(False,
                (lambda: self.compareImage("testMove1"),
                 moveit,
                 checkRelPos
                ))

    def testCropImage(self):
        def moveTLCrop():
            node = player.getElementByID("img")
            node.x = -20
            node.y = -20
        
        def moveBRCrop():
            node = player.getElementByID("img")
            node.x = 60
            node.y = 40
        
        def moveTLNegative():
            node = player.getElementByID("img")
            node.x = -60
            node.y = -50
        
        def moveBRGone():
            node = player.getElementByID("img")
            node.x = 140
            node.y = 100
        
        def rotate():
            node = player.getElementByID("img")
            node.x = 10
            node.y = 10
            player.getElementByID("nestedavg").angle = 1.0
            player.getElementByID("bkgd").angle = 1.0
        
        root = self.loadEmptyScene()
        avg.ImageNode(id="bkgd", href="crop_bkgd.png", parent=root)
        root.appendChild(
                player.createNode("""
                  <div id="nestedavg" x="40" y="30" width="80" height="60" crop="True">
                    <div id="nestedavg2" crop="True">
                      <div id="nestedavg3" crop="True">
                        <image id="img" x="10" y="10" width="40" height="40" 
                                href="rgb24-64x64.png"/>
                      </div>
                    </div>
                  </div>
                """))
        self.start(False,
                (lambda: self.compareImage("testCropImage1"),
                 moveTLCrop,
                 lambda: self.compareImage("testCropImage2"),
                 moveBRCrop,
                 lambda: self.compareImage("testCropImage3"),
                 moveTLNegative,
                 lambda: self.compareImage("testCropImage4"),
                 moveBRGone,
                 lambda: self.compareImage("testCropImage5"),

                 rotate,
                 lambda: self.compareImage("testCropImage6"),
                 moveTLCrop,
                 lambda: self.compareImage("testCropImage7"),
                 moveBRCrop,
                 lambda: self.compareImage("testCropImage8"),
                 moveTLNegative,
                 lambda: self.compareImage("testCropImage9"),
                 moveBRGone,
                 lambda: self.compareImage("testCropImage10")
               ))

    def testCropMovie(self):
        def playMovie():
            node = player.getElementByID("movie")
            node.play()
        
        def moveTLCrop():
            node = player.getElementByID("movie")
            node.x = -20
            node.y = -20
        
        def moveBRCrop():
            node = player.getElementByID("movie")
            node.x = 60
            node.y = 40
        
        def moveTLNegative():
            node = player.getElementByID("movie")
            node.x = -60
            node.y = -50
        
        def moveBRGone():
            node = player.getElementByID("movie")
            node.x = 140
            node.y = 100
        
        def rotate():
            node = player.getElementByID("movie")
            node.x = 10
            node.y = 10
            player.getElementByID("nestedavg").angle = 1.0
            player.getElementByID("bkgd").angle = 1.0
        
        player.setFakeFPS(30)
        root = self.loadEmptyScene()
        avg.ImageNode(id="bkgd", href="crop_bkgd.png", parent=root)
        root.appendChild(
                player.createNode("""
                  <div id="nestedavg" x="40" y="30" width="80" height="60" crop="True">
                    <video id="movie" x="10" y="10" width="40" height="40" 
                            threaded="false" href="mpeg1-48x48.mov"
                            fps="30"/>
                  </div>
                """))
        self.start(False,
                (playMovie,
                 lambda: self.compareImage("testCropMovie1"),
                 moveTLCrop,
                 lambda: self.compareImage("testCropMovie2"),
                 moveBRCrop,
                 lambda: self.compareImage("testCropMovie3"),
                 moveTLNegative,
                 lambda: self.compareImage("testCropMovie4"),
                 moveBRGone,
                 lambda: self.compareImage("testCropMovie5"),

                 rotate,
                 lambda: self.compareImage("testCropMovie6"),
                 moveTLCrop,
                 lambda: self.compareImage("testCropMovie7"),
                 moveBRCrop,
                 lambda: self.compareImage("testCropMovie8"),
                 moveTLNegative,
                 lambda: self.compareImage("testCropMovie9"),
                 moveBRGone,
                 lambda: self.compareImage("testCropMovie10")
                ))

    def testWarp(self):
        def moveVertex():
            grid = image.getWarpedVertexCoords()
            grid[1][1] = (grid[1][1][0]+0.06, grid[1][1][1]+0.06)
            image.setWarpedVertexCoords(grid)
            grid = video.getWarpedVertexCoords()
            grid[0][0] = (grid[0][0][0]+0.06, grid[0][0][1]+0.06)
            grid[1][1] = (grid[1][1][0]-0.06, grid[1][1][1]-0.06)
            video.setWarpedVertexCoords(grid)
        
        def flip():
            grid = image.getOrigVertexCoords()
            grid = [ [ (1-pos[0], pos[1]) for pos in line ] for line in grid]
            image.setWarpedVertexCoords(grid)
       
        root = self.loadEmptyScene()
        image = avg.ImageNode(href="rgb24-64x64.png",
                maxtilewidth=32, maxtileheight=16, parent=root)
        video = avg.VideoNode(pos=(40,0), size=(80,80), opacity=0.5, loop=True,
                href="mpeg1-48x48.mov", threaded=False, fps=30, parent=root)

        self.assertRaises(RuntimeError, image.getOrigVertexCoords)
        self.assertRaises(RuntimeError, image.getWarpedVertexCoords)
        player.setFakeFPS(30)
        self.start(False,
                (lambda: video.play(),
                 lambda: self.compareImage("testWarp1"),
                 moveVertex,
                 lambda: self.compareImage("testWarp2"),
                 flip,
                 lambda: self.compareImage("testWarp3")
                ))

    def testMediaDir(self):
        def createImageNode():
            # Node is not in tree; mediadir should be root node dir.
            node = avg.ImageNode(href="rgb24-64x64a.png")
            self.assertEqual(node.size, avg.Point2D(0,0)) # File not found
            node.href = "rgb24-64x64.png"
            self.assertEqual(node.size, avg.Point2D(64,64)) # File found
            node = avg.ImageNode(href="rgb24-64x64a.png", width=23, height=42)
            # File not found, but custom size
            self.assertEqual(node.size, avg.Point2D(23,42))
            node.href = "rgb24-64x64.png"
            # File found, custom size stays
            self.assertEqual(node.size, avg.Point2D(23,42))
            node.size = (0,0)
            # File found, custom size cleared. Media size should be used.
            self.assertEqual(node.size, avg.Point2D(64,64))

        def setDir():
            div.mediadir=""
        
        def setAbsDir():
            def absDir():
                # Should not find any media here...
                div.mediadir="/testmediadir"

            self.assertRaises(RuntimeError, absDir)
        
        def createNode():
            avg.VideoNode(href="mjpeg1-48x48.avi", fps=30)

        root = self.loadEmptyScene()
        div = avg.DivNode(mediadir="../testmediadir", parent=root)
        image = avg.ImageNode(pos=(0,30), href="rgb24-64x64a.png", parent=div)
        video = avg.VideoNode(href="mjpeg-48x48.avi", threaded=False, parent=div)
        self.start(False,
                (createImageNode,
                 lambda: video.play(),
                 lambda: self.compareImage("testMediaDir1"),
                 setDir,
                 lambda: video.play(), 
                 lambda: self.compareImage("testMediaDir2"),
                 lambda: self.assertEqual(image.width, 0),
                 createNode,
                 setAbsDir
                ))

    def testMemoryQuery(self):
        self.assertNotEqual(player.getMemoryUsage(), 0)

    def testStopOnEscape(self):
        def pressEscape():
            Helper = player.getTestHelper()
            escape = 27
            Helper.fakeKeyEvent(avg.Event.KEY_DOWN, escape, escape, "escape", escape, 
                    avg.KEYMOD_NONE),
            Helper.fakeKeyEvent(avg.Event.KEY_UP, escape, escape, "escape", escape, 
                    avg.KEYMOD_NONE),
        
        def testEscape1():
            player.stopOnEscape(False)
            pressEscape()
        
        def testEscape2():
            player.stopOnEscape(True)
            player.stopOnEscape(False)
            pressEscape()
        
        def testEscape3():
            player.stopOnEscape(True)
            pressEscape()
        
        def setAlive():
            self.testStopOnEscapeAlive = True

        self.testStopOnEscapeAlive = False
        self.__initDefaultScene()
        self.start(False,
                (testEscape1,
                 testEscape2,
                 setAlive
                ))
        self.assert_(self.testStopOnEscapeAlive)
        self.__initDefaultScene()
        self.start(False,
                (testEscape3, # this should exit the player
                 lambda: self.fail(),
                ))

    def testScreenDimensions(self):
        def queryDimensions():
            res = player.getScreenResolution()
            self.assert_(res.x > 0 and res.y > 0 and res.x < 10000 and res.y < 10000)
            ppmm = player.getPixelsPerMM()
            self.assert_(ppmm > 0 and ppmm < 10000)
            mm = player.getPhysicalScreenDimensions()
            self.assert_(mm.x > 0 and mm.y > 0 and mm.x < 10000 and mm.y < 10000)
            player.assumePixelsPerMM(ppmm)
            newPPMM = player.getPixelsPerMM()
            self.assertAlmostEqual(newPPMM, ppmm)
            newMM = player.getPhysicalScreenDimensions()
            self.assertEqual(newMM, mm)

        queryDimensions()
        self.__initDefaultScene()
        self.start(False,
                (queryDimensions,
                ))

    def testSVG(self):
        svgFile = avg.SVG("media/rect.svg", False)

        # renderElement
        bmp = svgFile.renderElement("rect")
        self.compareBitmapToFile(bmp, "testSvgBmp")
        self.assertEqual(svgFile.getElementSize("rect"), avg.Point2D(22,12))
        bmp = svgFile.renderElement("pos_rect")
        self.compareBitmapToFile(bmp, "testSvgPosBmp")
        bmp = svgFile.renderElement("rect", 5)
        self.compareBitmapToFile(bmp, "testSvgScaleBmp1")
        bmp = svgFile.renderElement("rect", (20,20))
        self.compareBitmapToFile(bmp, "testSvgScaleBmp2")

        # error handling
        self.assertRaises(RuntimeError, lambda: avg.SVG("filedoesntexist.svg", False))
        self.assertRaises(RuntimeError, lambda: svgFile.renderElement("missing_id"))

        # unescapeIllustratorIDs
        svgIllustratorFile = avg.SVG("illustratorRect.svg", True)
        svgIllustratorFile.getElementSize("pos_rect")

        # createImageNode
        root = self.loadEmptyScene()
        self.start(False,
                (lambda: svgFile.createImageNode("rect", {"pos":(10,10), "parent":root}),
                 lambda: self.compareImage("testSvgNode"),
                 lambda: svgFile.createImageNode("rect", {"pos":(5,5), "parent":root},
                        5),
                 lambda: self.compareImage("testSvgScaledNode1"),
                 lambda: svgFile.createImageNode("rect", {"pos":(1,1), "parent":root},
                        (40,40)),
                 lambda: self.compareImage("testSvgScaledNode2")
                ))

    def testGetConfigOption(self):
        self.assert_(len(player.getConfigOption("scr", "bpp")) > 0)
        self.assertRaises(RuntimeError, lambda: 
                player.getConfigOption("scr", "illegalOption"))
        self.assertRaises(RuntimeError, lambda:
                player.getConfigOption("illegalGroup", "illegalOption"))

    def testValidateXml(self):
        schema = """<?xml version="1.0" encoding="UTF-8"?>
        <xs:schema xmlns:xs="http://www.w3.org/2001/XMLSchema">

        <xs:element name="shiporder">
          <xs:complexType>
            <xs:sequence>
              <xs:element name="orderperson" type="xs:string"/>
            </xs:sequence>
            <xs:attribute name="orderid" type="xs:string" use="required"/>
          </xs:complexType>
        </xs:element>

        </xs:schema>
        """
        xmlString = """<?xml version="1.0" encoding="UTF-8"?>

        <shiporder orderid="889923">
          <orderperson>John Smith</orderperson>
        </shiporder>
        """
        avg.validateXml(xmlString, schema, "shiporder.xml", "shiporder.xsd")
       
        brokenSchema = "ff"+schema
        self.assertRaises(RuntimeError, lambda: avg.validateXml(xmlString, brokenSchema,
                "shiporder.xml", "shiporder.xsd"))

        brokenXml = xmlString+"ff"
        self.assertRaises(RuntimeError, lambda: avg.validateXml(brokenXml, schema,
                "shiporder.xml", "shiporder.xsd"))

    # Not executed due to bug #145 - hangs with some window managers.
    def testWindowFrame(self):
        def revertWindowFrame():
            player.setWindowFrame(True)

        player.setWindowFrame(False)
        self.__initDefaultScene()
        self.start(False, [revertWindowFrame])

    def __initDefaultScene(self):
        root = self.loadEmptyScene()
        avg.ImageNode(id="mainimg", size=(100, 75), href="rgb24-65x65.png", parent=root)
        div = avg.DivNode(id="nestedavg", pos=(0,32), opacity=1, size=(128, 32),
                crop=True, parent=root)
        avg.ImageNode(id="obscured", pos=(0,20), size=(96,40), href="rgb24-65x65.png",
                parent=div)
        avg.ImageNode(id="nestedimg1", size=(96,48), href="rgb24-65x65.png",
                parent=div)
        avg.ImageNode(id="nestedimg2", pos=(65,0), href="rgb24alpha-64x64.png",
                parent=div)

    def __initDefaultRotateScene(self):
        root = self.loadEmptyScene()
        div = avg.DivNode(pos=(80,10), size=(80,60), pivot=(0,0), angle=0.274,
                crop=True, parent=root)
        avg.ImageNode(pos=(10,10), size=(32,32), href="rgb24-65x65.png", parent=div)
        outerDiv = avg.DivNode(id="outer", pos=(80,70), size=(80,60),
                pivot=(0,0), angle=0.274, crop=True, parent=root)
        innerDiv = avg.DivNode(id="inner", size=(80,60), pivot=(0,0), angle=-0.274,
                crop=True, parent=outerDiv)
        avg.ImageNode(pos=(10,10), size=(32,32), href="rgb24-65x65.png", parent=innerDiv)
        return root

def playerTestSuite(tests):
    availableTests = (
            "testPoint",
            "testBasics",
            "testColorParse",
            "testFakeTime",
            "testDivResize",
            "testRotate",
            "testRotate2",
            "testRotatePivot",
            "testOpacity",
            "testOutlines",
            "testWordsOutlines",
            "testError",
            "testExceptionInTimeout",
            "testInvalidImageFilename",
            "testInvalidVideoFilename",
            "testTimeouts",
            "testTimeoutOnFrameHandling",
            "testCallFromThread",
            "testAVGFile",
            "testBroken",
            "testMove",
            "testCropImage",
            "testCropMovie",
            "testWarp",
            "testMediaDir",
            "testMemoryQuery",
            "testStopOnEscape",
            "testScreenDimensions",
            "testSVG",
            "testGetConfigOption",
            "testValidateXml",
#            "testWindowFrame",
            )
    return createAVGTestSuite(availableTests, PlayerTestCase, tests)
