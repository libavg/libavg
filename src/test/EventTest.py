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

# TODO: Get rid of the following globals
mainCaptureMouseDownCalled = False
captureMouseDownCalled = False

def onMainCaptureMouseDown(Event):
    global mainCaptureMouseDownCalled
    mainCaptureMouseDownCalled = True

def onCaptureMouseDown(Event):
    global captureMouseDownCalled
    captureMouseDownCalled = True
   
class NodeHandlerTester:
    def __init__(self, testCase, node):
        self.__testCase=testCase
        self.reset()
        self.__node = node
        self.setHandlers()

    def assertState(self, down, up, over, out, move):
        self.__testCase.assert_(down == self.__downCalled)
        self.__testCase.assert_(up == self.__upCalled)
        self.__testCase.assert_(over == self.__overCalled)
        self.__testCase.assert_(out == self.__outCalled)
        self.__testCase.assert_(move == self.__moveCalled)
        self.__testCase.assert_(not(self.__touchDownCalled))
        self.reset()

    def reset(self):
        self.__upCalled=False
        self.__downCalled=False
        self.__overCalled=False
        self.__outCalled=False
        self.__moveCalled=False
        self.__touchDownCalled=False

    def setHandlers(self):
        self.__node.setEventHandler(avg.CURSORDOWN, avg.MOUSE, self.__onDown) 
        self.__node.setEventHandler(avg.CURSORUP, avg.MOUSE, self.__onUp) 
        self.__node.setEventHandler(avg.CURSOROVER, avg.MOUSE, self.__onOver) 
        self.__node.setEventHandler(avg.CURSOROUT, avg.MOUSE, self.__onOut) 
        self.__node.setEventHandler(avg.CURSORMOTION, avg.MOUSE, self.__onMove) 
        self.__node.setEventHandler(avg.CURSORDOWN, avg.TOUCH, self.__onTouchDown) 

    def clearHandlers(self):
        self.__node.setEventHandler(avg.CURSORDOWN, avg.MOUSE, None) 
        self.__node.setEventHandler(avg.CURSORUP, avg.MOUSE, None) 
        self.__node.setEventHandler(avg.CURSOROVER, avg.MOUSE, None) 
        self.__node.setEventHandler(avg.CURSOROUT, avg.MOUSE, None) 
        self.__node.setEventHandler(avg.CURSORMOTION, avg.MOUSE, None) 
        self.__node.setEventHandler(avg.CURSORDOWN, avg.TOUCH, None) 

    def __onDown(self, Event):
        self.__testCase.assert_(Event.type == avg.CURSORDOWN)
        self.__downCalled = True
    
    def __onUp(self, Event):
        self.__testCase.assert_(Event.type == avg.CURSORUP)
        self.__upCalled = True

    def __onOver(self, Event):
        self.__testCase.assert_(Event.type == avg.CURSOROVER)
        self.__overCalled = True
    
    def __onOut(self, Event):
        self.__testCase.assert_(Event.type == avg.CURSOROUT)
        self.__outCalled = True
    
    def __onMove(self, Event):
        self.__testCase.assert_(Event.type == avg.CURSORMOTION)
        self.__moveCalled = True
    
    def __onTouchDown(self, Event):
        self.__touchDownCalled = True
        

class EventTestCase(AVGTestCase):
    def __init__(self, testFuncName):
        AVGTestCase.__init__(self, testFuncName, 24)

    def testKeyEvents(self):
        def onKeyDown(Event):
            if Event.keystring == 'A' and Event.keycode == 65 and Event.unicode == 65:
                self.keyDownCalled = True
        
        def onKeyUp(Event):
            if Event.keystring == 'A' and Event.keycode == 65 and Event.unicode == 65:
                self.keyUpCalled = True
        
        self._loadEmpty()
        Player.getRootNode().setEventHandler(avg.KEYDOWN, avg.NONE, onKeyDown)
        Player.getRootNode().setEventHandler(avg.KEYUP, avg.NONE, onKeyUp)
        self.start(None, 
                (lambda: Helper.fakeKeyEvent(avg.KEYDOWN, 65, 65, "A", 65, 
                        avg.KEYMOD_NONE),
                 lambda: self.assert_(self.keyDownCalled),
                 lambda: Helper.fakeKeyEvent(avg.KEYUP, 65, 65, "A", 65, avg.KEYMOD_NONE),
                 lambda: self.assert_(self.keyUpCalled)
                ))

    def testGlobalEvents(self):
        global mainMouseUpCalled
        global mainMouseDownCalled

        def reset():
            global mainMouseUpCalled
            global mainMouseDownCalled
            mainMouseUpCalled = False
            mainMouseDownCalled = False
        
        Player.loadString("""
            <avg width="160" height="120"  
                    oncursordown="mainMouseDown" oncursorup="mainMouseUp"/>
        """)
        self.start(None, 
                (lambda: Helper.fakeMouseEvent(avg.CURSORDOWN, True, False, False,
                        10, 10, 1),
                 lambda: self.assert_(mainMouseDownCalled and not(mainMouseUpCalled)),
                 reset,
                 lambda: Helper.fakeMouseEvent(avg.CURSORUP, True, False, False,
                        10, 10, 1),
                 lambda: self.assert_(not(mainMouseDownCalled) and mainMouseUpCalled)
                ))

    def testSimpleEvents(self):
        def getMouseState():
            Event = Player.getMouseState()
            self.assert_(Event.pos == avg.Point2D(10,10))
            self.assert_(Event.lastdownpos == avg.Point2D(10,10))
            # Make sure we're getting a Point2D as return value.
            self.assert_(Event.lastdownpos/2 == avg.Point2D(5, 5))
        
        self._loadEmpty()
        root = Player.getRootNode()
        img1 = avg.ImageNode(pos=(0,0), href="rgb24-65x65.png", parent=root)
        handlerTester1 = NodeHandlerTester(self, img1)

        img2 = avg.ImageNode(pos=(64,0), href="rgb24-65x65.png", parent=root)
        handlerTester2 = NodeHandlerTester(self, img2)

        self.start(None, 
                (# down, getMouseState(), move, up.
                 # events are inside img1 but outside img2.
                 lambda: Helper.fakeMouseEvent(avg.CURSORDOWN, True, False, False,
                        10, 10, 1),
                 lambda: handlerTester1.assertState(
                        down=True, up=False, over=True, out=False, move=False),
                 lambda: handlerTester2.assertState(
                        down=False, up=False, over=False, out=False, move=False),
                 getMouseState,

                 lambda: Helper.fakeMouseEvent(avg.CURSORMOTION, True, False, False,
                        12, 12, 1),
                 lambda: handlerTester1.assertState(
                        down=False, up=False, over=False, out=False, move=True),

                 lambda: Helper.fakeMouseEvent(avg.CURSORUP, True, False, False,
                        12, 12, 1),
                 lambda: handlerTester1.assertState(
                        down=False, up=True, over=False, out=False, move=False)
                ))

    def testTilted(self):
        self._loadEmpty()
        root = Player.getRootNode()
        img = avg.ImageNode(pos=(0,0), href="rgb24-65x65.png", angle=0.785, parent=root)
        handlerTester = NodeHandlerTester(self, img)
        
        self.start(None, 
                (lambda: Helper.fakeMouseEvent(avg.CURSORDOWN, True, False, False,
                        32, 32, 1),
                 lambda: handlerTester.assertState(
                        down=True, up=False, over=True, out=False, move=False),
                 lambda: Helper.fakeMouseEvent(avg.CURSORUP, True, False, False,
                        0, 0, 1),
                 lambda: handlerTester.assertState(
                        down=False, up=False, over=False, out=True, move=False),
                ))

    def testDivEvents(self):
        self._loadEmpty()
        root = Player.getRootNode()
        div = avg.DivNode(pos=(0,0), parent=root)
        divHandlerTester = NodeHandlerTester(self, div)

        img = avg.ImageNode(pos=(0,0), href="rgb24-65x65.png", parent=div)
        imgHandlerTester = NodeHandlerTester(self, img)
        
        self.start(None, 
                (# down, move, up.
                 # events are inside img and therefore should bubble to div. 
                 lambda: Helper.fakeMouseEvent(avg.CURSORDOWN, True, False, False,
                        10, 10, 1),
                 lambda: divHandlerTester.assertState(
                        down=True, up=False, over=True, out=False, move=False),
                 lambda: imgHandlerTester.assertState(
                        down=True, up=False, over=True, out=False, move=False),

                 lambda: Helper.fakeMouseEvent(avg.CURSORMOTION, True, False, False,
                        12, 12, 1),
                 lambda: divHandlerTester.assertState(
                        down=False, up=False, over=False, out=False, move=True),
                 lambda: imgHandlerTester.assertState(
                        down=False, up=False, over=False, out=False, move=True),
        
                 lambda: Helper.fakeMouseEvent(avg.CURSORUP, True, False, False,
                        12, 12, 1),
                 lambda: divHandlerTester.assertState(
                        down=False, up=True, over=False, out=False, move=False),
                 lambda: imgHandlerTester.assertState(
                        down=False, up=True, over=False, out=False, move=False)
                ))

    def testObscuringEvents(self):
        self._loadEmpty()
        root = Player.getRootNode()
        img1 = avg.ImageNode(pos=(0,0), href="rgb24-65x65.png", parent=root)
        handlerTester1 = NodeHandlerTester(self, img1)

        img2 = avg.ImageNode(pos=(0,0), href="rgb24-65x65.png", parent=root)
        handlerTester2 = NodeHandlerTester(self, img2)
        self.start(None, 
                (# down, move, up.
                 # events should only arrive at img2 because img1 is obscured by img1.
                 lambda: Helper.fakeMouseEvent(avg.CURSORDOWN, True, False, False,
                        10, 10, 1),
                 lambda: handlerTester1.assertState(
                        down=False, up=False, over=False, out=False, move=False),
                 lambda: handlerTester2.assertState(
                        down=True, up=False, over=True, out=False, move=False),

                 lambda: Helper.fakeMouseEvent(avg.CURSORMOTION, True, False, False,
                        12, 12, 1),
                 lambda: handlerTester1.assertState(
                        down=False, up=False, over=False, out=False, move=False),
                 lambda: handlerTester2.assertState(
                        down=False, up=False, over=False, out=False, move=True),

                 lambda: Helper.fakeMouseEvent(avg.CURSORUP, True, False, False,
                        12, 12, 1),
                 lambda: handlerTester1.assertState(
                        down=False, up=False, over=False, out=False, move=False),
                 lambda: handlerTester2.assertState(
                        down=False, up=True, over=False, out=False, move=False)
                ))

    def testSensitive(self):
        # Tests both sensitive and active attributes.
        def activateNode(useSensitiveAttr, b):
            if useSensitiveAttr:
                self.img.sensitive = b
            else:
                self.img.active = b
        for useSensitiveAttr in (True, False):
            self._loadEmpty()
            root = Player.getRootNode()
            self.img = avg.ImageNode(id="img", pos=(0,0), href="rgb24-65x65.png", 
                    parent=root)
            handlerTester = NodeHandlerTester(self, self.img)

            activateNode(useSensitiveAttr, False)
            self.start(None,
                    (# Node is inactive -> no events.
                     lambda: Helper.fakeMouseEvent(avg.CURSORDOWN, True, False, False,
                            10, 10, 1),
                     lambda: handlerTester.assertState(
                            down=False, up=False, over=False, out=False, move=False),
                     lambda: Helper.fakeMouseEvent(avg.CURSORUP, True, False, False,
                            10, 10, 1),

                     # Activate the node -> events arrive.
                     lambda: activateNode(useSensitiveAttr, True),
                     lambda: Helper.fakeMouseEvent(avg.CURSORDOWN, True, False, False,
                            10, 10, 1),
                     lambda: handlerTester.assertState(
                            down=True, up=False, over=True, out=False, move=False),
                     lambda: Helper.fakeMouseEvent(avg.CURSORUP, True, False, False,
                            10, 10, 1),
                    ))
            self.img = None

    def testChangingHandlers(self):
        self._loadEmpty()
        root = Player.getRootNode()
        img = avg.ImageNode(pos=(0,0), href="rgb24-65x65.png", parent=root)
        handlerTester = NodeHandlerTester(self, img)
        
        self.start(None, 
                (lambda: handlerTester.clearHandlers(),
                 lambda: Helper.fakeMouseEvent(avg.CURSORDOWN, True, False, False,
                        10, 10, 1),
                 lambda: handlerTester.assertState(
                        down=False, up=False, over=False, out=False, move=False),
                 lambda: handlerTester.setHandlers(),
                 lambda: Helper.fakeMouseEvent(avg.CURSORUP, True, False, False,
                        10, 10, 1),
                 lambda: handlerTester.assertState(
                        down=False, up=True, over=False, out=False, move=False),
                ))

    def testEventCapture(self):
        def captureEvent():
            global captureMouseDownCalled
            captureMouseDownCalled = False
            mainCaptureMouseDownCalled = False
            Player.getElementByID("img1").setEventCapture()
            Helper.fakeMouseEvent(avg.CURSORDOWN, True, False, False,
                    100, 10, 1)
        
        def noCaptureEvent():
            global captureMouseDownCalled
            captureMouseDownCalled = False
            mainCaptureMouseDownCalled = False
            Player.getElementByID("img1").releaseEventCapture()
            Helper.fakeMouseEvent(avg.CURSORDOWN, True, False, False,
                    100, 10, 1)
        
        global captureMouseDownCalled
        global mainCaptureMouseDownCalled
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

        self._loadEmpty()
        Player.getRootNode().setEventHandler(avg.CURSORDOWN, avg.MOUSE, onErrMouseOver)
        self.assertException(lambda:
                self.start(None,
                        (lambda: Helper.fakeMouseEvent(avg.CURSORDOWN, 
                                False, False, False, 10, 10, 0),
                )))

def eventTestSuite(tests):
    availableTests = (
            "testKeyEvents",
            "testGlobalEvents",
            "testSimpleEvents",
            "testTilted",
            "testDivEvents",
            "testObscuringEvents",
            "testSensitive",
            "testChangingHandlers",
            "testEventCapture",
            "testMouseOver",
            "testEventErr"
            )
    return AVGTestSuite(availableTests, EventTestCase, tests)

Player = avg.Player.get()
Helper = Player.getTestHelper()

