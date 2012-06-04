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

from libavg import avg, player
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
        
        root = self.loadEmptyScene()
        root.setEventHandler(avg.KEYDOWN, avg.NONE, onKeyDown)
        root.setEventHandler(avg.KEYUP, avg.NONE, onKeyUp)
        self.start(False,
                (lambda: Helper.fakeKeyEvent(avg.KEYDOWN, 65, 65, "A", 65, 
                        avg.KEYMOD_NONE),
                 lambda: self.assert_(self.keyDownCalled),
                 lambda: Helper.fakeKeyEvent(avg.KEYUP, 65, 65, "A", 65, avg.KEYMOD_NONE),
                 lambda: self.assert_(self.keyUpCalled)
                ))

    def testSimpleEvents(self):
        def getMouseState():
            Event = player.getMouseState()
            self.assertEqual(Event.pos, avg.Point2D(10,10))
        
        root = self.loadEmptyScene()
        img1 = avg.ImageNode(pos=(0,0), href="rgb24-65x65.png", parent=root)
        handlerTester1 = NodeHandlerTester(self, img1)

        img2 = avg.ImageNode(pos=(64,0), href="rgb24-65x65.png", parent=root)
        handlerTester2 = NodeHandlerTester(self, img2)

        self.start(False,
                (# down, getMouseState(), move, up.
                 # events are inside img1 but outside img2.
                 lambda: self.assert_(not(player.isMultitouchAvailable())),
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
        root = self.loadEmptyScene()
        root = root
        img = avg.ImageNode(pos=(0,0), href="rgb24-65x65.png", angle=0.785, parent=root)
        handlerTester = NodeHandlerTester(self, img)
        
        self.start(False,
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
        root = self.loadEmptyScene()
        div = avg.DivNode(pos=(0,0), parent=root)
        divHandlerTester = NodeHandlerTester(self, div)

        img = avg.ImageNode(pos=(0,0), href="rgb24-65x65.png", parent=div)
        imgHandlerTester = NodeHandlerTester(self, img)
        
        self.start(False,
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

    def testUnlinkInHandler(self):
        def onImgDown(event):
            self.__imgDownCalled = True
            self.div.unlink(True)

        def onDivDown(event):
            self.__divDownCalled = True

        def checkState():
            self.assert_(self.__imgDownCalled and not(self.__divDownCalled))

        self.__imgDownCalled = False
        self.__divDownCalled = False
        root = self.loadEmptyScene()
        self.div = avg.DivNode(pos=(0,0), parent=root)
        self.div.connectEventHandler(avg.CURSORDOWN, avg.MOUSE, self, onDivDown)

        img = avg.ImageNode(pos=(0,0), href="rgb24-65x65.png", parent=self.div)
        img.connectEventHandler(avg.CURSORDOWN, avg.MOUSE, self, onImgDown)
        
        self.start(False,
                (lambda: Helper.fakeMouseEvent(avg.CURSORDOWN, True, False, False,
                        10, 10, 1),
                 checkState))
        

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

        root = self.loadEmptyScene()
        self.img = avg.ImageNode(pos=(0,0), href="rgb24-65x65.png", parent=root)
        connectTwoHandlers()
        self.img.disconnectEventHandler(self, onDown1)
        self.img.disconnectEventHandler(self, onDown2)
        connectTwoHandlers()
        self.img.disconnectEventHandler(self)

        resetDownCalled()
        self.start(False,
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
        root = self.loadEmptyScene()
        img1 = avg.ImageNode(pos=(0,0), href="rgb24-65x65.png", parent=root)
        handlerTester1 = NodeHandlerTester(self, img1)

        img2 = avg.ImageNode(pos=(0,0), href="rgb24-65x65.png", parent=root)
        handlerTester2 = NodeHandlerTester(self, img2)
        self.start(False,
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
        def activateNode(node, useSensitiveAttr, b):
            if useSensitiveAttr:
                node.sensitive = b
            else:
                node.active = b

        def onNode2Down(event):
            self.__node2Down = True
        
        for useSensitiveAttr in (True, False):
            root = self.loadEmptyScene()
            self.img = avg.ImageNode(pos=(0,0), href="rgb24-65x65.png", parent=root)
            handlerTester = NodeHandlerTester(self, self.img)

            activateNode(self.img, useSensitiveAttr, False)
            self.start(False,
                    (# Node is inactive -> no events.
                     lambda: Helper.fakeMouseEvent(avg.CURSORDOWN, True, False, False,
                            10, 10, 1),
                     lambda: handlerTester.assertState(
                            down=False, up=False, over=False, out=False, move=False),
                     lambda: Helper.fakeMouseEvent(avg.CURSORUP, False, False, False,
                            10, 10, 1),

                     # Activate the node -> events arrive.
                     lambda: activateNode(self.img, useSensitiveAttr, True),
                     lambda: Helper.fakeMouseEvent(avg.CURSORDOWN, True, False, False,
                            10, 10, 1),
                     lambda: handlerTester.assertState(
                            down=True, up=False, over=True, out=False, move=False),
                     lambda: Helper.fakeMouseEvent(avg.CURSORUP, False, False, False,
                            10, 10, 1),
                    ))
            self.img = None

            # Check if sensitive is deactivated immediately, not at the end of the frame.
            root = self.loadEmptyScene()
            self.img1 = avg.ImageNode(pos=(0,0), href="rgb24-65x65.png", parent=root)
            self.img2 = avg.ImageNode(pos=(64,0), href="rgb24-65x65.png", parent=root)
            self.img1.connectEventHandler(avg.CURSORDOWN, avg.TOUCH, self, 
                    lambda event: activateNode(self.img2, useSensitiveAttr, False))
            self.img2.connectEventHandler(avg.CURSORDOWN, avg.TOUCH, self, onNode2Down)
            self.__node2Down = False

            self.start(False,
                    (lambda: self._sendTouchEvents((
                            (1, avg.CURSORDOWN, 10, 10),
                            (2, avg.CURSORDOWN, 80, 10),)),
                     lambda: self.assert_(not(self.__node2Down)),
                    ))

    def testChangingHandlers(self):
        root = self.loadEmptyScene()
        img = avg.ImageNode(pos=(0,0), href="rgb24-65x65.png", parent=root)
        handlerTester = NodeHandlerTester(self, img)
        
        self.start(False,
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

        root = self.loadEmptyScene()
        root.setEventHandler(avg.CURSORDOWN, avg.MOUSE, onMainMouseDown)
        self.img = avg.ImageNode(pos=(0,0), href="rgb24-65x65.png", parent=root)
        self.img.setEventHandler(avg.CURSORDOWN, avg.MOUSE, onMouseDown)

        self.start(False,
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
            Parent = img1.parent
            Parent.removeChild(Parent.indexOf(img1))
        
        Helper = player.getTestHelper()
        root = self.loadEmptyScene()
        img1 = avg.ImageNode(href="rgb24-65x65.png", parent=root)
        div = avg.DivNode(pos=(65,0), parent=root)
        img3 = avg.ImageNode(href="rgb24-65x65.png", parent=div)
        img2 = avg.ImageNode(pos=(0,65), href="rgb24-65x65.png", parent=div)
        
        img2.setEventHandler(avg.CURSOROVER, avg.MOUSE, onImg2MouseOver)
        img2.setEventHandler(avg.CURSOROUT, avg.MOUSE, onImg2MouseOut)
        div.setEventHandler(avg.CURSOROVER, avg.MOUSE, onDivMouseOver)
        div.setEventHandler(avg.CURSOROUT, avg.MOUSE, onDivMouseOut)
        root.setEventHandler(avg.CURSOROVER, avg.MOUSE, onAVGMouseOver)
        img1.setEventHandler(avg.CURSOROVER, avg.MOUSE, onImg1MouseOver)
        self.start(False,
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
                 lambda: img2.setEventCapture(),
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

        root = self.loadEmptyScene()
        root.setEventHandler(avg.CURSORDOWN, avg.MOUSE, onErrMouseOver)
        self.assertException(lambda:
                self.start((
                         lambda: Helper.fakeMouseEvent(avg.CURSORDOWN, 
                                False, False, False, 10, 10, 0),
                )))
    
    def testEventHook(self):
        def resetState():
            self.ehookMouseEvent = False
            self.ehookKeyboardEvent = False

        def cleanup():
            resetState()
            player.setEventHook(None)
            
        def handleEvent(event):
            if isinstance(event, avg.MouseEvent) and event.source == avg.MOUSE:
                if event.type == avg.CURSORDOWN:
                    self.ehookMouseEvent = True
            elif isinstance(event, avg.KeyEvent):
                self.ehookKeyboardEvent = True
            else:
                self.fail()
            
        root = self.loadEmptyScene()
        resetState()

        player.setEventHook(handleEvent)
        self.start(False,
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
        
        root = self.loadEmptyScene()
        root.appendChild(rect)
        
        self.__exceptionThrown = False
        try:
            self.start(False,
                    (lambda: Helper.fakeMouseEvent(avg.CURSORDOWN, True, False, False, 10,
                            10, 0),
                     lambda: None
                    ))
        except TestException:
            self.__exceptionThrown = True
            
        self.assert_(self.__exceptionThrown)
               
    def testContacts(self):

        def onDown(event):
            contact = event.contact
            self.assertEqual(event.cursorid, contact.id)
            self.assertEqual(contact.age, 0)
            self.assertEqual(contact.distancefromstart, 0)
            self.assertEqual(contact.motionangle, 0)
            self.assertEqual(contact.motionvec, (0,0))
            self.assertEqual(contact.distancetravelled, 0)
            self.assertEqual(contact.events[0].pos, event.pos)
            self.assertEqual(len(contact.events), 1)
            contact.connectListener(onMotion, onUp)

        def onMotion(event):
            contact = event.contact
            self.assertEqual(event.cursorid, contact.id)
            self.assertEqual(contact.age, 40)
            self.assertEqual(contact.distancefromstart, 10)
            self.assertEqual(contact.motionangle, 0)
            self.assertEqual(contact.motionvec, (10,0))
            self.assertEqual(contact.distancetravelled, 10)
            self.assertEqual(contact.events[-1].pos, event.pos)
            self.assert_(len(contact.events) > 1)
            self.numContactCallbacks += 1
 
        def onUp(event):
            contact = event.contact
            self.assertEqual(event.cursorid, contact.id)
            self.assertEqual(contact.age, 80)
            self.assertEqual(contact.distancefromstart, 0)
            self.assertEqual(contact.motionangle, 0)
            self.assertEqual(contact.motionvec, (0,0))
            self.assertEqual(contact.distancetravelled, 20)
            self.assertEqual(contact.events[-1].pos, event.pos)
            self.assert_(len(contact.events) > 1)
            self.numContactCallbacks += 1

        def onOver(event): 
            self.numOverCallbacks += 1
            self.assertEqual(event.cursorid, event.contact.id)

        def onOut(event): 
            self.numOutCallbacks += 1
            self.assertEqual(event.cursorid, event.contact.id)

        root = self.loadEmptyScene()
        root.connectEventHandler(avg.CURSORDOWN, avg.TOUCH, self, onDown)
        self.numContactCallbacks = 0
        rect = avg.RectNode(pos=(5,5), size=(10,10), parent=root)
        rect.connectEventHandler(avg.CURSOROVER, avg.TOUCH, self, onOver)
        self.numOverCallbacks = 0
        rect.connectEventHandler(avg.CURSOROUT, avg.TOUCH, self, onOut)
        self.numOutCallbacks = 0
        player.setFakeFPS(25)
        self.start(False,
                (lambda: Helper.fakeTouchEvent(1, avg.CURSORDOWN, avg.TOUCH, (10,10)),
                 lambda: Helper.fakeTouchEvent(1, avg.CURSORMOTION, avg.TOUCH, (20,10)),
                 lambda: Helper.fakeTouchEvent(1, avg.CURSORUP, avg.TOUCH, (10,10)),
                ))
        self.assertEqual(self.numContactCallbacks, 2)
        self.assertEqual(self.numOverCallbacks, 2)
        self.assertEqual(self.numOutCallbacks, 2)
        
        root = self.loadEmptyScene()
        root.connectEventHandler(avg.CURSORDOWN, avg.MOUSE, self, onDown)
        self.numContactCallbacks = 0
        self.start(False,
                (lambda: Helper.fakeMouseEvent(avg.CURSORDOWN, 1, 0, 0, 10, 10, 0),
                 lambda: Helper.fakeMouseEvent(avg.CURSORMOTION, 1, 0, 0, 20, 10, 0),
                 lambda: Helper.fakeMouseEvent(avg.CURSORUP, 0, 0, 0, 10, 10, 0),
                ))
        self.assertEqual(self.numContactCallbacks, 2)

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
       
        root = self.loadEmptyScene()
        root.connectEventHandler(avg.CURSORDOWN, avg.TOUCH, self, onDown)
        self.numMotionCallbacks = 0
        root.connectEventHandler(avg.CURSORMOTION, avg.TOUCH, self, onMotion)
        self.numContactCallbacks = 0
        player.setFakeFPS(25)
        self.start(False,
                (lambda: Helper.fakeTouchEvent(1, avg.CURSORDOWN, avg.TOUCH, (10,10)),
                 lambda: Helper.fakeTouchEvent(1, avg.CURSORMOTION, avg.TOUCH, (20,10)),
                 lambda: Helper.fakeTouchEvent(1, avg.CURSORMOTION, avg.TOUCH, (30,10)),
                 lambda: Helper.fakeTouchEvent(1, avg.CURSORMOTION, avg.TOUCH, (40,10)),
                ))
        self.assertEqual(self.numContactCallbacks, 1)
        self.assertEqual(self.numMotionCallbacks, 1)
        
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
            self.assertEqual(self.numContact1Callbacks, 0)
            self.numContact2Callbacks += 1
        
        root = self.loadEmptyScene()
        root.connectEventHandler(avg.CURSORDOWN, avg.TOUCH, self, onDown)
        player.setFakeFPS(25)
        self.numContact1Callbacks = 0
        self.numContact2Callbacks = 0
        self.start(False,
                (lambda: Helper.fakeTouchEvent(1, avg.CURSORDOWN, avg.TOUCH, (10,10)),
                 lambda: Helper.fakeTouchEvent(1, avg.CURSORMOTION, avg.TOUCH, (20,10)),
                 lambda: Helper.fakeTouchEvent(1, avg.CURSORUP, avg.TOUCH, (10,10)),
                ))
        self.assertEqual(self.numContact1Callbacks, 2)
        # The order of callbacks is unspecified, so onContact2 might be called once.
        self.assert_(self.numContact2Callbacks <= 1)


def eventTestSuite(tests):
    availableTests = (
            "testKeyEvents",
            "testSimpleEvents",
            "testTilted",
            "testDivEvents",
            "testUnlinkInHandler",
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

Helper = player.getTestHelper()
