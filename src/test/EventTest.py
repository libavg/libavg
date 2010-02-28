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

mainCaptureMouseDownCalled = False
captureMouseDownCalled = False

def onMainCaptureMouseDown(Event):
    global mainCaptureMouseDownCalled
    mainCaptureMouseDownCalled = True

def onCaptureMouseDown(Event):
    global captureMouseDownCalled
    captureMouseDownCalled = True
    
class EventTestCase(AVGTestCase):
    def __init__(self, testFuncName):
        AVGTestCase.__init__(self, testFuncName, 24)

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

    def testEventErr(self):
        def onErrMouseOver(Event):
            undefinedFunction()

        Helper = Player.getTestHelper()
        self._loadEmpty()
        Player.getRootNode().setEventHandler(avg.CURSORDOWN, avg.MOUSE, onErrMouseOver)
        self.assertException(lambda:
                self.start(None,
                        (lambda: Helper.fakeMouseEvent(avg.CURSORDOWN, 
                                False, False, False, 10, 10, 0),
                )))

def eventTestSuite(tests):
    availableTests = (
            "testEvents",
            "testEventCapture",
            "testMouseOver",
            "testEventErr"
            )
    return AVGTestSuite(availableTests, EventTestCase, tests)

Player = avg.Player.get()

