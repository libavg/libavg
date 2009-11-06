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

def keyUp(Event):
    print "keyUp"

def keyDown(Event):
    print "keyDown"
    print Event
    print "  Type: "+str(Event.type)
    print "  keystring: "+Event.keystring
    print "  scancode: "+str(Event.scancode)
    print "  keycode: "+str(Event.keycode)
    print "  modifiers: "+str(Event.modifiers)

def dumpMouseEvent(Event):
    print Event
    print "  type: "+str(Event.type)
    print "  leftbuttonstate: "+str(Event.leftbuttonstate)
    print "  middlebuttonstate: "+str(Event.middlebuttonstate)
    print "  rightbuttonstate: "+str(Event.rightbuttonstate)
    print "  position: "+str(Event.x)+","+str(Event.y)
    print "  node: "+Event.node.id

mainMouseUpCalled = False
mainMouseDownCalled = False

def mainMouseUp(Event):
    global mainMouseUpCalled
    assert (Event.type == avg.CURSORUP)
    mainMouseUpCalled = True

def mainMouseDown(Event):
    global mainMouseDownCalled
    assert (Event.type == avg.CURSORDOWN)
    mainMouseDownCalled = True

def onErrMouseOver(Event):
    undefinedFunction()

mainCaptureMouseDownCalled = False
captureMouseDownCalled = False

def onMainCaptureMouseDown(Event):
    global mainCaptureMouseDownCalled
    mainCaptureMouseDownCalled = True

def onCaptureMouseDown(Event):
    global captureMouseDownCalled
    captureMouseDownCalled = True
    
class PlayerTestCase(AVGTestCase):
    def __init__(self, testFuncName, bpp):
        AVGTestCase.__init__(self, testFuncName, bpp)

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

    def testImage(self):
        def loadNewFile():
            self.assert_(Player.getElementByID("test").getMediaSize() == (65,65))
            Player.getElementByID("test").href = "rgb24alpha-64x64.png"
            Player.getElementByID("test1").href = "rgb24alpha-64x64.png"
            self.assert_(Player.getElementByID("test").getMediaSize() == (64,64))

        def getFramerate():
            framerate = Player.getEffectiveFramerate()
            self.assert_(framerate > 0)

        def testImageSize():
            img = Player.createNode('image', {'size':avg.Point2D(23,42), 
                    'href':'rgb24alpha-64x64.png'})
            self.assert_(img.size == avg.Point2D(23,42))

        def setUnicodeHref():
            # Can't check unicode filenames into svn or the windows client breaks.
            # So we rename the file locally.
            if not(os.path.exists(u"ö.png")):
                shutil.copyfile("oe.png", u"ö.png")
            Player.getElementByID("test").href = u"ö.png"

        Player.showCursor(0)
        Player.showCursor(1)
        Player.loadFile("image.avg")
        node = Player.getElementByID("test")
        self.assert_(node.width == 65)
        self.assert_(node.height == 65)
        self.assert_(node.pos == (64, 30))
        self.assert_(node.size == (65, 65))
        testImageSize()
        self.start(None,
                (lambda: self.compareImage("testimg", False), 
                 getFramerate,
                 loadNewFile, 
                 lambda: self.compareImage("testimgload", False),
                 lambda: Player.setGamma(0.3, 0.3, 0.3),
                 lambda: Player.showCursor(0),
                 lambda: Player.showCursor(1),
                 setUnicodeHref,
                 lambda: self.compareImage("testimg2", False),
                ))

    def testImageMask(self):
        def setMask():
            try:
                node.maskhref = "mask.png"
            except RuntimeError:
                print "Skipping testImageMask - no shader support."
                Player.stop()

        def setMaskSize():
            node.masksize = (32,32)

        def setMaskPos():
            node.maskpos = (32,32)

        def setMaskHref():
            node.maskhref = "mask2.png"

        def setBaseHref():
            node.href="freidrehen.jpg"

        def resetPos():
            node.maskpos = (0,0)
            node.masksize = (0,0)

        def setMaskNotFound():
            node.maskhref = "nonexistentmask.png"
           
        Player.loadString("""
            <?xml version="1.0"?>
            <avg width="160" height="120">
                <image id="test" href="rgb24-65x65.png"/>
            </avg>
        """)
        node = Player.getElementByID("test")
        self.start(None,
                (setMask,
                 lambda: self.compareImage("testimgmask1", False),
                 setMaskSize,
                 lambda: self.compareImage("testimgmask2", False),
                 setMaskPos,
                 lambda: self.compareImage("testimgmask3", False),
                 setMaskHref,
                 lambda: self.compareImage("testimgmask4", False),
                 setBaseHref,
                 lambda: self.compareImage("testimgmask5", False),
                 resetPos,
                 lambda: self.compareImage("testimgmask6", False),
                 setMaskNotFound,
                ))

    def testMipmap(self):
        Player.loadString("""
            <?xml version="1.0"?>
            <avg id="imageavg" width="160" height="120">
                <image width="64" height="64" href="checker.png"/>
                <image x="64" width="64" height="64" href="checker.png" mipmap="true"/>
            </avg>
        
        """)
        self.start(None, 
                [lambda: self.compareImage("testmipmap1", False)])

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

    def testBitmap(self):
        def getBitmap(node):
            Bmp = node.getBitmap()
            self.assert_(Bmp.getSize() == (65,65))
            self.assert_(Bmp.getFormat() == avg.R8G8B8X8 or 
                    Bmp.getFormat() == avg.B8G8R8X8)
            node.setBitmap(Bmp)
        
        def loadBitmap():
            Bmp = avg.Bitmap("greyscale.png")
            self.assert_(Bmp.getSize() == (64,64))
            self.assert_(Bmp.getFormat() == avg.I8)
        
        Player.loadFile("image.avg")
        node = Player.getElementByID("test")
        getBitmap(node)
#        loadBitmap()
        self.start(None,
                (lambda: getBitmap(Player.getElementByID("test")),
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

    def testEvents(self):
        def getMouseState():
            Event = Player.getMouseState()
            self.assert_(Event.pos == avg.Point2D(10,10))
            self.assert_(Event.lastdownpos == avg.Point2D(10,10))
            # Make sure we're getting a Point2D as return value.
            self.assert_(Event.lastdownpos/2 == avg.Point2D(5, 5))
        
        def testInactiveDiv():
            Player.getElementByID("div1").active = False
            Helper.fakeMouseEvent(avg.CURSORDOWN, True, False, False,
                70, 70, 1)
        
        def disableHandler():
            self.mouseDown1Called = False
            Player.getElementByID("img1").setEventHandler(avg.CURSORDOWN, avg.MOUSE, None)
        
        def onMouseMove1(Event):
            self.assert_ (Event.type == avg.CURSORMOTION)
            print "onMouseMove1"
        
        def onMouseUp1(Event):
            self.assert_ (Event.type == avg.CURSORUP)
            self.mouseUp1Called = True
        
        def onMouseDown1(Event):
            self.assert_ (Event.type == avg.CURSORDOWN)
            self.mouseDown1Called = True
        
        def onMouseOver1(Event):
            self.assert_ (Event.type == avg.CURSOROVER)
            self.mouseOver1Called = True
        
        def onMouseOut1(Event):
            self.assert_ (Event.type == avg.CURSOROUT)
            self.mouseOut1Called = True
        
        def onTouchDown(Event):
            self.touchDownCalled = True
        
        def onDivMouseDown(Event):
            self.assert_ (Event.type == avg.CURSORDOWN)
            self.divMouseDownCalled = True
        
        def onMouseDown2(Event):
            self.assert_ (Event.type == avg.CURSORDOWN)
            self.mouseDown2Called = True
        
        def onObscuredMouseDown(Event):
            self.obscuredMouseDownCalled = True
        
        def onDeactMouseDown(Event):
            self.deactMouseDownCalled = True
        
        def onDeactMouseOver(Event):
            self.deactMouseOverLate = self.deactMouseDownCalled
            self.deactMouseOverCalled = True
        
        def onDeactMouseMove(Event):
            print("move")
        
        def onDeactMouseOut(Event):
            pass
        
        def onTiltedMouseDown(Event):
            self.tiltedMouseDownCalled = True
        
        def onKeyDown(Event):
            if Event.keystring == 'A' and Event.keycode == 65 and Event.unicode == 65:
                self.keyDownCalled = True
        
        def onKeyUp(Event):
            if Event.keystring == 'A' and Event.keycode == 65 and Event.unicode == 65:
                self.keyUpCalled = True
        
        def neverCalled(Event):
            self.neverCalledCalled = True

        Helper = Player.getTestHelper()
        global mainMouseUpCalled
        global mainMouseDownCalled
        Player.loadFile("events.avg")
        
        self.mouseMove1Called=False
        self.mouseUp1Called=False
        self.mouseDown1Called=False
        self.mouseOver1Called=False
        self.mouseOut1Called=False
        self.touchDownCalled = False
        self.keyDownCalled = False
        self.keyUpCalled = False
        img1 = Player.getElementByID("img1")
        img1.setEventHandler(avg.CURSORMOTION, avg.MOUSE, onMouseMove1) 
        img1.setEventHandler(avg.CURSORUP, avg.MOUSE, onMouseUp1) 
        img1.setEventHandler(avg.CURSORDOWN, avg.MOUSE, onMouseDown1) 
        img1.setEventHandler(avg.CURSOROVER, avg.MOUSE, onMouseOver1) 
        img1.setEventHandler(avg.CURSOROUT, avg.MOUSE, onMouseOut1) 
        img1.setEventHandler(avg.CURSORDOWN, avg.TOUCH, onTouchDown) 
        Player.getRootNode().setEventHandler(avg.KEYDOWN, avg.NONE, onKeyDown)
        Player.getRootNode().setEventHandler(avg.KEYUP, avg.NONE, onKeyUp)

        self.neverCalledCalled=False
        hidden = Player.getElementByID("hidden")
        hidden.setEventHandler(avg.CURSORUP, avg.MOUSE, neverCalled)

        self.obscuredMouseDownCalled=False
        obscured = Player.getElementByID("obscured")
        obscured.setEventHandler(avg.CURSORDOWN, avg.MOUSE, onObscuredMouseDown)

        self.divMouseDownCalled=False
        div1 = Player.getElementByID("div1")
        div1.setEventHandler(avg.CURSORDOWN, avg.MOUSE, onDivMouseDown)

        self.mouseDown2Called=False
        img2 = Player.getElementByID("img2")
        img2.setEventHandler(avg.CURSORDOWN, avg.MOUSE, onMouseDown2)

        self.deactMouseOverCalled=False
        self.deactMouseOverLate=False
        self.deactMouseDownCalled=False
        deact = Player.getElementByID("deact")
        deact.setEventHandler(avg.CURSORDOWN, avg.MOUSE, onDeactMouseDown)
        deact.setEventHandler(avg.CURSOROVER, avg.MOUSE, onDeactMouseOver)
        deact.setEventHandler(avg.CURSOROUT, avg.MOUSE, onDeactMouseOut)
        deact.setEventHandler(avg.CURSORMOTION, avg.MOUSE, onDeactMouseMove)

        self.tiltedMouseDownCalled=False
        Player.getElementByID("tilted").setEventHandler(
                avg.CURSORDOWN, avg.MOUSE, onTiltedMouseDown)

        self.start(None, 
                (lambda: self.compareImage("testEvents", False),
                 lambda: Helper.fakeMouseEvent(avg.CURSORDOWN, True, False, False,
                        10, 10, 1),
                 lambda: self.assert_(self.mouseDown1Called and self.mouseOver1Called 
                        and mainMouseDownCalled and not(self.touchDownCalled)),
                 getMouseState,
                 lambda: Helper.fakeMouseEvent(avg.CURSORUP, True, False, False,
                        12, 12, 1),
                 lambda: Helper.fakeMouseEvent(avg.CURSORMOTION, False, False, False,
                        70, 70, 1),
                 lambda: self.assert_(self.mouseUp1Called and mainMouseUpCalled),
                 lambda: Helper.fakeMouseEvent(avg.CURSORDOWN, True, False, False,
                        70, 70, 1),
                 lambda: self.assert_(self.mouseDown2Called and self.divMouseDownCalled
                        and self.mouseOut1Called and not(self.obscuredMouseDownCalled)),
                 testInactiveDiv,
                 lambda: self.assert_(self.obscuredMouseDownCalled),
                 # Test if deactivation between mouse click and mouse out works.
                 lambda: Helper.fakeMouseEvent(avg.CURSORDOWN, True, False, False,
                        70, 10, 1),
                 lambda: self.assert_(self.deactMouseOverCalled 
                        and not(self.deactMouseOverLate) and not(self.neverCalledCalled)),
                 disableHandler,
                 lambda: Helper.fakeMouseEvent(avg.CURSORDOWN, True, False, False,
                        10, 10, 1),
                 lambda: self.assert_(not(self.mouseDown1Called)),
                 lambda: Helper.fakeMouseEvent(avg.CURSORDOWN, True, False, False,
                        0, 64, 1),
                 lambda: self.assert_(not(self.tiltedMouseDownCalled)),
                 lambda: Helper.fakeMouseEvent(avg.CURSORDOWN, True, False, False,
                        0, 80, 1),
                 lambda: self.assert_(self.tiltedMouseDownCalled),
                 lambda: Helper.fakeKeyEvent(avg.KEYDOWN, 65, 65, "A", 65, 
                        avg.KEYMOD_NONE),
                 lambda: self.assert_(self.keyDownCalled),
                 lambda: Helper.fakeKeyEvent(avg.KEYUP, 65, 65, "A", 65, avg.KEYMOD_NONE),
                 lambda: self.assert_(self.keyUpCalled)
                ))

    def testEventCapture(self):
        def captureEvent():
            global captureMouseDownCalled
            Helper = Player.getTestHelper()
            captureMouseDownCalled = False
            mainCaptureMouseDownCalled = False
            Player.getElementByID("img1").setEventCapture()
            Helper.fakeMouseEvent(avg.CURSORDOWN, True, False, False,
                    100, 10, 1)
        
        def noCaptureEvent():
            global captureMouseDownCalled
            Helper = Player.getTestHelper()
            captureMouseDownCalled = False
            mainCaptureMouseDownCalled = False
            Player.getElementByID("img1").releaseEventCapture()
            Helper.fakeMouseEvent(avg.CURSORDOWN, True, False, False,
                    100, 10, 1)
        
        global captureMouseDownCalled
        global mainCaptureMouseDownCalled
        Helper = Player.getTestHelper()
        self.start("eventcapture.avg",
                (lambda: Helper.fakeMouseEvent(avg.CURSORDOWN, True, False, False,
                        10, 10, 1),
                 lambda: self.assert_(captureMouseDownCalled),
                 captureEvent,
                 lambda: self.assert_(captureMouseDownCalled and 
                        mainCaptureMouseDownCalled),
                 noCaptureEvent,
                 lambda: self.assert_(not(captureMouseDownCalled) and 
                        mainCaptureMouseDownCalled)
                ))

    def testMouseOver(self):
        def onImg2MouseOver(Event):
            self.img2MouseOverCalled = True
        
        def onImg2MouseOut(Event):
            self.img2MouseOutCalled = True
        
        def onDivMouseOver(Event):
            self.divMouseOverCalled = True
        
        def onDivMouseOut(Event):
            self.divMouseOutCalled = True
        
        def onAVGMouseOver(Event):
            self.avgMouseOverCalled = True
        
        def onImg1MouseOver(Event):
            self.img1MouseOverCalled = True
        
        def printState():
            print "----"
            print "img2MouseOverCalled=", self.img2MouseOverCalled
            print "img2MouseOutCalled=", self.img2MouseOutCalled
            print "divMouseOverCalled=", self.divMouseOverCalled
            print "divMouseOutCalled=", self.divMouseOutCalled
            print "avgMouseOverCalled=", self.avgMouseOverCalled
            print "img1MouseOverCalled=", self.img1MouseOverCalled
        
        def resetState():
            self.img2MouseOverCalled = False
            self.img2MouseOutCalled = False
            self.divMouseOverCalled = False
            self.divMouseOutCalled = False
            self.avgMouseOverCalled = False
            self.img1MouseOverCalled = False
        
        def killNodeUnderCursor():
            Parent = img1.getParent()
            Parent.removeChild(Parent.indexOf(img1))
        
        Helper = Player.getTestHelper()
        Player.loadFile("mouseover.avg")
        img2 = Player.getElementByID("img2")
        img2.setEventHandler(avg.CURSOROVER, avg.MOUSE, onImg2MouseOver)
        img2.setEventHandler(avg.CURSOROUT, avg.MOUSE, onImg2MouseOut)
        div = Player.getElementByID("div1")
        div.setEventHandler(avg.CURSOROVER, avg.MOUSE, onDivMouseOver)
        div.setEventHandler(avg.CURSOROUT, avg.MOUSE, onDivMouseOut)
        avgNode = Player.getRootNode()
        avgNode.setEventHandler(avg.CURSOROVER, avg.MOUSE, onAVGMouseOver)
        img1 = Player.getElementByID("img1")
        img1.setEventHandler(avg.CURSOROVER, avg.MOUSE, onImg1MouseOver)
        self.start(None, 
                (resetState,
                 lambda: Helper.fakeMouseEvent(avg.CURSORDOWN, True, False, False,
                        70, 70, 1),
                 lambda: self.assert_(
                        self.img2MouseOverCalled and 
                        self.divMouseOverCalled and
                        self.avgMouseOverCalled and 
                        not(self.img2MouseOutCalled) and 
                        not(self.divMouseOutCalled) and 
                        not(self.img1MouseOverCalled)),

                 resetState,
                 lambda: Helper.fakeMouseEvent(avg.CURSORDOWN, True, False, False,
                        70, 10, 1),
                 lambda: self.assert_(
                        not(self.img2MouseOverCalled) and 
                        not(self.divMouseOverCalled) and 
                        not(self.avgMouseOverCalled) and 
                        self.img2MouseOutCalled and 
                        not(self.divMouseOutCalled) and 
                        not(self.img1MouseOverCalled)),
                 
                 resetState,
                 lambda: Helper.fakeMouseEvent(avg.CURSORDOWN, True, False, False,
                        10, 10, 1),
                 lambda: self.assert_(
                        not(self.img2MouseOverCalled) and 
                        not(self.divMouseOverCalled) and 
                        not(self.avgMouseOverCalled) and 
                        not(self.img2MouseOutCalled) and 
                        self.divMouseOutCalled and 
                        self.img1MouseOverCalled),

                 resetState,
                 killNodeUnderCursor,
                 lambda: Helper.fakeMouseEvent(avg.CURSORUP, True, False, False,
                        10, 10, 1),
                 lambda: self.assert_(
                        not(self.img2MouseOverCalled) and 
                        not(self.divMouseOverCalled) and 
                        not(self.avgMouseOverCalled) and 
                        not(self.img2MouseOutCalled) and 
                        not(self.divMouseOutCalled) and 
                        not(self.img1MouseOverCalled)),

                 resetState,
                 lambda: Player.getElementByID("img2").setEventCapture(),
                 lambda: Helper.fakeMouseEvent(avg.CURSORUP, True, False, False,
                        70, 70, 1),
                 lambda: self.assert_(
                        self.img2MouseOverCalled and 
                        not(self.divMouseOverCalled) and 
                        not(self.avgMouseOverCalled) and 
                        not(self.img2MouseOutCalled) and 
                        not(self.divMouseOutCalled) and 
                        not(self.img1MouseOverCalled)),

                 resetState,
                 lambda: Helper.fakeMouseEvent(avg.CURSORUP, True, False, False,
                        10, 10, 1),
                 lambda: self.assert_(
                        not(self.img2MouseOverCalled) and 
                        not(self.divMouseOverCalled) and 
                        not(self.avgMouseOverCalled) and 
                        self.img2MouseOutCalled and 
                        not(self.divMouseOutCalled) and 
                        not(self.img1MouseOverCalled))
                ))

    def testTimeouts(self):
        def timeout1():
            Player.clearInterval(self.timeout1ID)
            Player.clearInterval(self.timeout2ID)
            self.timeout1called = True
        
        def timeout2():
            self.timeout2called = True
        
        def wait():
            pass
        
        def setupTimeouts():
            self.timeout1ID = Player.setTimeout(0, timeout1)
            self.timeout2ID = Player.setTimeout(1, timeout2)
        
        self.timeout1called = False
        self.timeout2called = False
        self.start("image.avg",
                (setupTimeouts,
                 wait,
                 lambda: self.assert_(self.timeout1called),
                 lambda: self.assert_(not(self.timeout2called))
                ))

    def testEventErr(self):
        Player.loadFile("errevent.avg")
        Player.setTimeout(10, Player.stop)
        try:
            Player.play()
        except NameError:
            print("(Intentional) NameError caught")
            self.assert_(1)

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
        
        def testIllegalSet1():
            node = Player.getElementByID("nestedimg1")
            node.pos.x = 23
        
        def testIllegalSet2():
            node = Player.getElementByID("nestedimg1")
            node.size.x = 23
        
        self.start("avg.avg",
                (lambda: self.compareImage("testMove1", False),
                 moveit,
                 checkRelPos,
                 lambda: self.compareImage("testMove2", False),
                 lambda: self.assertException(testIllegalSet1),
                 lambda: self.assertException(testIllegalSet2),
                ))

    def testSetBitmap(self):
        def setBitmap():
            node = Player.createNode('image',{'y':65})
            bitmap = avg.Bitmap('rgb24-65x65.png')
            node.setBitmap(bitmap)
            parent = Player.getElementByID("mainavgnode")
            parent.appendChild(node)
        
        def setBitmapLinked(nodeID):
            node = Player.getElementByID(nodeID)
            bitmap = avg.Bitmap('rgb24-65x65.png')
            node.setBitmap(bitmap)
        
        def setNullBitmap():
            node = Player.getElementByID("fullimg")
            node.setBitmap(None)

        self.start("setbitmap.avg",
                (
                    setBitmap,
                    lambda: self.compareImage("testSetBitmap1", False),
                    lambda: setBitmapLinked("fullimg"),
                    lambda: self.compareImage("testSetBitmap2", False),
                    lambda: setBitmapLinked("emptyimg"),
                    lambda: self.compareImage("testSetBitmap3", False),
                    lambda: self.assertException(setNullBitmap)
                ))

    def testBlend(self):
        self.start("blend.avg",
                [lambda: self.compareImage("testBlend", False)])

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
            node = Player.createNode("image", {"href":"rgb24-64x64.png"})
            self.assert_(node.size == avg.Point2D(1,1)) # File not found
            node.href = "rgb24-64x64a.png"
            self.assert_(node.size == avg.Point2D(64,64)) # File found
            node = Player.createNode("image", 
                    {"href":"rgb24-64x64.png", "width":23, "height":42})
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
            node = Player.createNode("video", {"href":"mjpeg1-48x48.avi", "fps":30})
        
        self.start("mediadir.avg",
                (createImageNode,
                 lambda: Player.getElementByID("video").play(),
                 lambda: self.compareImage("testMediaDir1", False),
                 setDir,
                 lambda: Player.getElementByID("video").play(), 
                 lambda: self.compareImage("testMediaDir2", False),
                 lambda: self.assert_(Player.getElementByID("img").width == 1),
                 createNode,
                 setAbsDir
                ))

    def testMemoryQuery(self):
        setUpVideo(Player)
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
            
def playerTestSuite(bpp, tests):
    availableTests = (
            "testPoint",
            "testImage",
            "testImageMask",
            "testMipmap",
            "testDivResize",
            "testBitmap",
            "testRotate",
            "testRotate2",
            "testRotate3",
            "testRotatePivot",
            "testOutlines",
            "testError",
            "testExceptionInTimeout",
            "testInvalidImageFilename",
            "testInvalidVideoFilename",
            "testEvents",
            "testEventCapture",
            "testMouseOver",
            "testTimeouts",
            "testEventErr",
            "testPanoImage",
            "testBroken",
            "testMove",
            "testSetBitmap",
            "testBlend",
            "testCropImage",
            "testCropMovie",
            "testWarp",
            "testMediaDir",
            "testMemoryQuery",
            "testStopOnEscape",
            )
    return AVGTestSuite(availableTests, PlayerTestCase, tests, (), {'bpp' : bpp})

Player = avg.Player.get()

if __name__ == '__main__':
    runStandaloneTest(playerTestSuite)
