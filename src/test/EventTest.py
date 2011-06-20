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
import sys

from libavg import avg
from testcase import *

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


class EventTestCase(AVGTestCase):
    def __init__(self, testFuncName):
        AVGTestCase.__init__(self, testFuncName)

    def testKeyEvents(self):
        def onKeyDown(Event):
            if Event.keystring == 'A' and Event.keycode == 65 and Event.unicode == 65:
                self.keyDownCalled = True
        
        def onKeyUp(Event):
            if Event.keystring == 'A' and Event.keycode == 65 and Event.unicode == 65:
                self.keyUpCalled = True
        
        self.loadEmptyScene()
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
                 lambda: Helper.fakeMouseEvent(avg.CURSORUP, False, False, False,
                        10, 10, 1),
                 lambda: self.assert_(not(mainMouseDownCalled) and mainMouseUpCalled)
                ))

    def testSimpleEvents(self):
        def getMouseState():
            Event = Player.getMouseState()
            self.assert_(Event.pos == avg.Point2D(10,10))
        
        self.loadEmptyScene()
        root = Player.getRootNode()
        img1 = avg.ImageNode(pos=(0,0), href="rgb24-65x65.png", parent=root)
        handlerTester1 = NodeHandlerTester(self, img1)

        img2 = avg.ImageNode(pos=(64,0), href="rgb24-65x65.png", parent=root)
        handlerTester2 = NodeHandlerTester(self, img2)

        self.start(None, 
                (# down, getMouseState(), move, up.
                 # events are inside img1 but outside img2.
                 lambda: self.assert_(not(Player.isMultitouchAvailable())),
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

                 lambda: Helper.fakeMouseEvent(avg.CURSORUP, False, False, False,
                        12, 12, 1),
                 lambda: handlerTester1.assertState(
                        down=False, up=True, over=False, out=False, move=False)
                 
                ))

    def testTilted(self):
        self.loadEmptyScene()
        root = Player.getRootNode()
        img = avg.ImageNode(pos=(0,0), href="rgb24-65x65.png", angle=0.785, parent=root)
        handlerTester = NodeHandlerTester(self, img)
        
        self.start(None, 
                (lambda: Helper.fakeMouseEvent(avg.CURSORDOWN, True, False, False,
                        32, 32, 1),
                 lambda: handlerTester.assertState(
                        down=True, up=False, over=True, out=False, move=False),
                 lambda: Helper.fakeMouseEvent(avg.CURSORUP, False, False, False,
                        0, 0, 1),
                 lambda: handlerTester.assertState(
                        down=False, up=False, over=False, out=True, move=False),
                ))

    def testDivEvents(self):
        self.loadEmptyScene()
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
        
                 lambda: Helper.fakeMouseEvent(avg.CURSORUP, False, False, False,
                        12, 12, 1),
                 lambda: divHandlerTester.assertState(
                        down=False, up=True, over=False, out=False, move=False),
                 lambda: imgHandlerTester.assertState(
                        down=False, up=True, over=False, out=False, move=False)
                ))

    def testConnectHandler(self):
        def onDown1(event):
            self.down1Called = True

        def onDown2(event):
            self.down2Called = True

        def resetDownCalled():
            self.down1Called = False
            self.down2Called = False

        def connectTwoHandlers():
            self.img.connectEventHandler(avg.CURSORDOWN, avg.MOUSE, self, onDown1)
            self.img.connectEventHandler(avg.CURSORDOWN, avg.MOUSE, self, onDown2)
        
        def connectUnlinkHandler():
            self.img.disconnectEventHandler(self)
            self.img.connectEventHandler(avg.CURSORDOWN, avg.MOUSE, self, unlinkHandler)
            self.img.connectEventHandler(avg.CURSORDOWN, avg.MOUSE, self, onDown2)

        def unlinkHandler(event):
            self.img.disconnectEventHandler(self)

        self.loadEmptyScene()
        root = Player.getRootNode()
        self.img = avg.ImageNode(pos=(0,0), href="rgb24-65x65.png", parent=root)
        connectTwoHandlers()
        self.img.disconnectEventHandler(self, onDown1)
        self.img.disconnectEventHandler(self, onDown2)
        connectTwoHandlers()
        self.img.disconnectEventHandler(self)

        resetDownCalled()
        self.start(None, 
                (connectTwoHandlers,
                 lambda: self.fakeClick(10,10),
                 lambda: self.assert_(self.down1Called and self.down2Called),
                 resetDownCalled,
                 lambda: self.img.disconnectEventHandler(self, onDown1),
                 lambda: self.fakeClick(10,10),
                 lambda: self.assert_(not(self.down1Called) and self.down2Called),
                 connectUnlinkHandler,
                 lambda: self.fakeClick(10,10),
                ))

    def testObscuringEvents(self):
        self.loadEmptyScene()
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

                 lambda: Helper.fakeMouseEvent(avg.CURSORUP, False, False, False,
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
            self.loadEmptyScene()
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
                     lambda: Helper.fakeMouseEvent(avg.CURSORUP, False, False, False,
                            10, 10, 1),

                     # Activate the node -> events arrive.
                     lambda: activateNode(useSensitiveAttr, True),
                     lambda: Helper.fakeMouseEvent(avg.CURSORDOWN, True, False, False,
                            10, 10, 1),
                     lambda: handlerTester.assertState(
                            down=True, up=False, over=True, out=False, move=False),
                     lambda: Helper.fakeMouseEvent(avg.CURSORUP, False, False, False,
                            10, 10, 1),
                    ))
            self.img = None

    def testChangingHandlers(self):
        self.loadEmptyScene()
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
                 lambda: Helper.fakeMouseEvent(avg.CURSORUP, False, False, False,
                        10, 10, 1),
                 lambda: handlerTester.assertState(
                        down=False, up=True, over=False, out=False, move=False),
                ))

    def testEventCapture(self):
        def onMainMouseDown(Event):
            self.mainMouseDownCalled = True

        def onMouseDown(Event):
            self.mouseDownCalled = True
   
        def captureEvent():
            self.mouseDownCalled = False
            self.mainMouseDownCalled = False
            self.img.setEventCapture()
                    
        def noCaptureEvent():
            self.mouseDownCalled = False
            self.mainMouseDownCalled = False
            self.img.releaseEventCapture()
      
        def doubleCaptureEvent():
            self.mouseDownCalled = False
            self.mainMouseDownCalled = False
            self.img.setEventCapture()
            self.img.setEventCapture()
            self.img.releaseEventCapture()

        def releaseTooMuch():
            self.img.releaseEventCapture()
            self.assertException(self.img.releaseEventCapture)

        self.mouseDownCalled = False
        self.mainMouseDownCalled = False

        self.loadEmptyScene()
        root = Player.getRootNode()
        root.setEventHandler(avg.CURSORDOWN, avg.MOUSE, onMainMouseDown)
        self.img = avg.ImageNode(pos=(0,0), href="rgb24-65x65.png", parent=root)
        self.img.setEventHandler(avg.CURSORDOWN, avg.MOUSE, onMouseDown)

        self.start(None,
                (lambda: Helper.fakeMouseEvent(avg.CURSORDOWN, True, False, False,
                        10, 10, 1),
                 lambda: self.assert_(self.mouseDownCalled),
                 lambda: Helper.fakeMouseEvent(avg.CURSORUP, False, False, False,
                        10, 10, 1),
                 captureEvent,
                 lambda: Helper.fakeMouseEvent(avg.CURSORDOWN, True, False, False,
                        100, 10, 1),
                 lambda: self.assert_(self.mouseDownCalled and 
                        self.mainMouseDownCalled),
                 lambda: Helper.fakeMouseEvent(avg.CURSORUP, False, False, False,
                        100, 10, 1),
                 noCaptureEvent,
                 lambda: Helper.fakeMouseEvent(avg.CURSORDOWN, True, False, False,
                        100, 10, 1),
                 lambda: self.assert_(not(self.mouseDownCalled) and 
                        self.mainMouseDownCalled),
                 lambda: Helper.fakeMouseEvent(avg.CURSORUP, False, False, False,
                        100, 10, 1),
                 doubleCaptureEvent,
                 lambda: Helper.fakeMouseEvent(avg.CURSORDOWN, True, False, False,
                        100, 10, 1),
                 lambda: self.assert_(self.mouseDownCalled and 
                        self.mainMouseDownCalled),
                 releaseTooMuch,
                 lambda: Helper.fakeMouseEvent(avg.CURSORUP, False, False, False,
                        100, 10, 1),
                ))
        self.img = None

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
                 lambda: Helper.fakeMouseEvent(avg.CURSORUP, False, False, False,
                        70, 70, 1),
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
                 lambda: Helper.fakeMouseEvent(avg.CURSORUP, False, False, False,
                        70, 10, 1),
                 
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
                 lambda: Helper.fakeMouseEvent(avg.CURSORUP, False, False, False,
                        10, 10, 1),
                 lambda: self.assert_(
                        not(self.img2MouseOverCalled) and 
                        not(self.divMouseOverCalled) and 
                        not(self.avgMouseOverCalled) and 
                        not(self.img2MouseOutCalled) and 
                        not(self.divMouseOutCalled) and 
                        not(self.img1MouseOverCalled)),

                 lambda: Helper.fakeMouseEvent(avg.CURSORDOWN, True, False, False,
                        10, 10, 1),
                 resetState,
                 lambda: Player.getElementByID("img2").setEventCapture(),
                 lambda: Helper.fakeMouseEvent(avg.CURSORUP, False, False, False,
                        70, 70, 1),
                 lambda: self.assert_(
                        self.img2MouseOverCalled and 
                        self.divMouseOverCalled and 
                        not(self.avgMouseOverCalled) and 
                        not(self.img2MouseOutCalled) and 
                        not(self.divMouseOutCalled) and 
                        not(self.img1MouseOverCalled)),

                 lambda: Helper.fakeMouseEvent(avg.CURSORDOWN, True, False, False,
                        70, 70, 1),
                 resetState,
                 lambda: Helper.fakeMouseEvent(avg.CURSORUP, False, False, False,
                        10, 10, 1),
                 lambda: self.assert_(
                        not(self.img2MouseOverCalled) and 
                        not(self.divMouseOverCalled) and 
                        not(self.avgMouseOverCalled) and 
                        self.img2MouseOutCalled and 
                        self.divMouseOutCalled and 
                        not(self.img1MouseOverCalled))
                ))

    def testEventErr(self):
        def onErrMouseOver(Event):
            undefinedFunction()

        self.loadEmptyScene()
        Player.getRootNode().setEventHandler(avg.CURSORDOWN, avg.MOUSE, onErrMouseOver)
        self.assertException(lambda:
                self.start(None,
                        (lambda: Helper.fakeMouseEvent(avg.CURSORDOWN, 
                                False, False, False, 10, 10, 0),
                )))
    
    def testEventHook(self):
        def resetState():
            self.ehookMouseEvent = False
            self.ehookKeyboardEvent = False

        def cleanup():
            resetState()
            Player.setEventHook(None)
            
        def handleEvent(event):
            if isinstance(event, avg.MouseEvent) and event.source == avg.MOUSE:
                if event.type == avg.CURSORDOWN:
                    self.ehookMouseEvent = True
            elif isinstance(event, avg.KeyEvent):
                self.ehookKeyboardEvent = True
            else:
                self.assert_(False)
            
        self.loadEmptyScene()
        resetState()

        Player.setEventHook(handleEvent)
        self.start(None,
                (lambda: self.fakeClick(10, 10),
                 lambda: self.assert_(self.ehookMouseEvent),
                 lambda: Helper.fakeKeyEvent(avg.KEYDOWN, 65, 65, "A", 65, 0),
                 lambda: self.assert_(self.ehookKeyboardEvent),
                 cleanup,
                 lambda: self.fakeClick(10, 10),
                 lambda: self.assert_(not self.ehookMouseEvent),
                 lambda: Helper.fakeKeyEvent(avg.KEYDOWN, 65, 65, "A", 65, 0),
                 lambda: self.assert_(not self.ehookKeyboardEvent),
            ))
        
    def testException(self):

        class TestException(Exception):
            pass
        
        def throwException(event):
            raise TestException
        
        rect = avg.RectNode(size = (50, 50))
        rect.setEventHandler(avg.CURSORDOWN, avg.MOUSE, throwException)
        
        self.loadEmptyScene()
        Player.getRootNode().appendChild(rect)
        
        self.__exceptionThrown = False
        try:
            self.start(None,
                (lambda: Helper.fakeMouseEvent(avg.CURSORDOWN, True, False, False, 10, 
                        10, 0),
                 lambda: None))
        except TestException:
            self.__exceptionThrown = True
            
        self.assert_(self.__exceptionThrown)
               
    def testContacts(self):

        def onDown(event):
            contact = event.contact
            self.assert_(event.cursorid == contact.id)
            self.assert_(contact.age == 0)
            self.assert_(contact.distancefromstart == 0)
            self.assert_(contact.motionangle == 0)
            self.assert_(contact.motionvec == (0,0))
            self.assert_(contact.distancetravelled == 0)
            contact.connectListener(onMotion, onUp)

        def onMotion(event):
            contact = event.contact
            self.assert_(event.cursorid == contact.id)
            self.assert_(contact.age == 40)
            self.assert_(contact.distancefromstart == 10)
            self.assert_(contact.motionangle == 0)
            self.assert_(contact.motionvec == (10,0))
            self.assert_(contact.distancetravelled == 10)
            self.numContactCallbacks += 1
 
        def onUp(event):
            contact = event.contact
            self.assert_(event.cursorid == contact.id)
            self.assert_(contact.age == 80)
            self.assert_(contact.distancefromstart == 0)
            self.assert_(contact.motionangle == 0)
            self.assert_(contact.motionvec == (0,0))
            self.assert_(contact.distancetravelled == 20)
            self.numContactCallbacks += 1

        def onOver(event): 
            self.numOverCallbacks += 1
            self.assert_(event.cursorid == event.contact.id)

        def onOut(event): 
            self.numOutCallbacks += 1
            self.assert_(event.cursorid == event.contact.id)

        self.loadEmptyScene()
        root = Player.getRootNode()
        root.connectEventHandler(avg.CURSORDOWN, avg.TOUCH, self, onDown)
        self.numContactCallbacks = 0
        rect = avg.RectNode(pos=(5,5), size=(10,10), parent=root)
        rect.connectEventHandler(avg.CURSOROVER, avg.TOUCH, self, onOver)
        self.numOverCallbacks = 0
        rect.connectEventHandler(avg.CURSOROUT, avg.TOUCH, self, onOut)
        self.numOutCallbacks = 0
        Player.setFakeFPS(25)
        self.start(None,
            (lambda: Helper.fakeTouchEvent(1, avg.CURSORDOWN, avg.TOUCH, (10,10)),
             lambda: Helper.fakeTouchEvent(1, avg.CURSORMOTION, avg.TOUCH, (20,10)),
             lambda: Helper.fakeTouchEvent(1, avg.CURSORUP, avg.TOUCH, (10,10)),
            ))
        self.assert_(self.numContactCallbacks == 2)
        self.assert_(self.numOverCallbacks == 2)
        self.assert_(self.numOutCallbacks == 2)
        
        self.loadEmptyScene()
        root = Player.getRootNode()
        root.connectEventHandler(avg.CURSORDOWN, avg.MOUSE, self, onDown)
        self.numContactCallbacks = 0
        self.start(None,
            (lambda: Helper.fakeMouseEvent(avg.CURSORDOWN, 1, 0, 0, 10, 10, 0),
             lambda: Helper.fakeMouseEvent(avg.CURSORMOTION, 1, 0, 0, 20, 10, 0),
             lambda: Helper.fakeMouseEvent(avg.CURSORUP, 0, 0, 0, 10, 10, 0),
            ))
        self.assert_(self.numContactCallbacks == 2)

    def testContactRegistration(self):

        def onDown(event):
            root.setEventCapture(event.cursorid)
            root.releaseEventCapture(event.cursorid)

        def onMotion(event):
            contact = event.contact
            self.contactID = contact.connectListener(onContactMotion, None)
            self.numMotionCallbacks += 1
            root.disconnectEventHandler(self)

        def onContactMotion(event):
            contact = event.contact
            contact.disconnectListener(self.contactID)
            self.assertException(lambda: contact.disconnectListener(self.contactID))
            self.numContactCallbacks += 1
       
        self.loadEmptyScene()
        root = Player.getRootNode()
        root.connectEventHandler(avg.CURSORDOWN, avg.TOUCH, self, onDown)
        self.numMotionCallbacks = 0
        root.connectEventHandler(avg.CURSORMOTION, avg.TOUCH, self, onMotion)
        self.numContactCallbacks = 0
        Player.setFakeFPS(25)
        self.start(None,
            (lambda: Helper.fakeTouchEvent(1, avg.CURSORDOWN, avg.TOUCH, (10,10)),
             lambda: Helper.fakeTouchEvent(1, avg.CURSORMOTION, avg.TOUCH, (20,10)),
             lambda: Helper.fakeTouchEvent(1, avg.CURSORMOTION, avg.TOUCH, (30,10)),
             lambda: Helper.fakeTouchEvent(1, avg.CURSORMOTION, avg.TOUCH, (40,10)),
            ))
        self.assert_(self.numContactCallbacks == 1)
        self.assert_(self.numMotionCallbacks == 1)
        
    def testMultiContactRegistration(self):

        def onDown(event):
            contact = event.contact
            self.contactid = contact.connectListener(onContact2, onContact2)
            contact.connectListener(onContact1, onContact1)

        def onContact1(event):
            if self.numContact1Callbacks == 0:
                event.contact.disconnectListener(self.contactid)
            self.numContact1Callbacks += 1

        def onContact2(event):
            self.assert_(self.numContact1Callbacks == 0)
            self.numContact2Callbacks += 1
        
        self.loadEmptyScene()
        root = Player.getRootNode()
        root.connectEventHandler(avg.CURSORDOWN, avg.TOUCH, self, onDown)
        Player.setFakeFPS(25)
        self.numContact1Callbacks = 0
        self.numContact2Callbacks = 0
        self.start(None,
            (lambda: Helper.fakeTouchEvent(1, avg.CURSORDOWN, avg.TOUCH, (10,10)),
             lambda: Helper.fakeTouchEvent(1, avg.CURSORMOTION, avg.TOUCH, (20,10)),
             lambda: Helper.fakeTouchEvent(1, avg.CURSORUP, avg.TOUCH, (10,10)),
            ))
        self.assert_(self.numContact1Callbacks == 2)
        # The order of callbacks is unspecified, so onContact2 might be called once.
        self.assert_(self.numContact2Callbacks <= 1)


def eventTestSuite(tests):
    availableTests = (
            "testKeyEvents",
            "testGlobalEvents",
            "testSimpleEvents",
            "testTilted",
            "testDivEvents",
            "testConnectHandler",
            "testObscuringEvents",
            "testSensitive",
            "testChangingHandlers",
            "testEventCapture",
            "testMouseOver",
            "testEventErr",
            "testEventHook",
            "testException",
            "testContacts",
            "testContactRegistration",
            "testMultiContactRegistration",
            )
    return createAVGTestSuite(availableTests, EventTestCase, tests)

Player = avg.Player.get()
Helper = Player.getTestHelper()
