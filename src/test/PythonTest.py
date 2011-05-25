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

from libavg import avg, anim, draggable, textarea, ui, geom, statemachine

from testcase import *

class PythonTestCase(AVGTestCase):
    def __init__(self, testFuncName):
        AVGTestCase.__init__(self, testFuncName)

    def testAnimType(self, curAnim, imgBaseName):
        def onStop():
            self.__onStopCalled = True

        def startAnim():
            self.__onStopCalled = False
            node = Player.getElementByID("test")
            self.__anim.start()

        def startKeepAttr():
            node = Player.getElementByID("test")
            node.x = 25
            self.__anim.start(keepAttr=True)

        def abortAnim():
            self.__anim.abort()

        self.__anim = curAnim
        self.__anim.setHandler(onStop, None)
        self.__onStopCalled = False
        Player.setFakeFPS(10)
        self.start(None,
                (startAnim,
                 lambda: self.compareImage(imgBaseName+"1", False),
                 lambda: self.assert_(anim.getNumRunningAnims() == 1),
                 None,
                 None,
                 lambda: self.assert_(self.__onStopCalled),
                 lambda: self.assert_(self.__anim.isDone()),
                 lambda: self.compareImage(imgBaseName+"2", False),
                 lambda: self.assert_(Player.getElementByID("test").x == 100),
                 startAnim,
                 lambda: self.compareImage(imgBaseName+"1", False),
                 abortAnim,
                 lambda: self.assert_(anim.getNumRunningAnims() == 0),
                 lambda: self.compareImage(imgBaseName+"3", False),
                 lambda: self.assert_(self.__anim.isDone()),
                 None,
                 lambda: self.assert_(not(self.__onStopCalled)),
                 startAnim,
                 startKeepAttr,
                 lambda: self.assert_(anim.getNumRunningAnims() == 1),
                 abortAnim
                ))
        self.__anim = None

    def testLinearAnim(self):
        Player.loadFile("image.avg")
        node = Player.getElementByID("test")
        curAnim = anim.LinearAnim(node, "x", 200, 0, 100, False)
        self.testAnimType(curAnim, "testLinearAnim")

    def testLinearAnimZeroDuration(self):
        def onStop():
            self.__onStopCalled = True

        def startAnim():
            self.__onStopCalled = False
            node = Player.getElementByID("test")
            self.__anim.start()

        Player.loadFile("image.avg")
        node = Player.getElementByID("test")
        self.__anim = anim.LinearAnim(node, "x", 0, 0, 100, False)
        self.__anim.setHandler(onStop, None)
        self.__onStopCalled = False
        Player.setFakeFPS(10)
        self.start(None,
                (startAnim,
                 lambda: self.compareImage("testLinearAnimZeroDuration1", False),
                 lambda: self.assert_(anim.getNumRunningAnims() == 0),
                 lambda: self.assert_(self.__onStopCalled),
                 lambda: self.assert_(self.__anim.isDone())
                ))
        self.__anim = None

    def testEaseInOutAnim(self):
        Player.loadFile("image.avg")
        node = Player.getElementByID("test")
        curAnim = anim.EaseInOutAnim(node, "x", 400, 0, 100, 100, 100, False)
        self.testAnimType(curAnim, "testEaseInOutAnim")

    def testSplineAnim(self):
        Player.loadFile("image.avg")
        node = Player.getElementByID("test")
        curAnim = anim.SplineAnim(node, "x", 300, 0, 0, 100, 0, False)
        self.testAnimType(curAnim, "testSplineAnim")

    def testContinuousAnim(self):
        def onStart():
            Player.setTimeout(10,startAnim)
            Player.setTimeout(100,lambda:self.compareImage("testContAnim1", False))
            Player.setTimeout(200,startAnim2)
            Player.setTimeout(400,lambda:self.compareImage("testContAnim2", False))
            Player.setTimeout(450,startAnim3)
            Player.setTimeout(700,lambda:self.compareImage("testContAnim3", False))
            Player.setTimeout(800,stopAnim)
            Player.setTimeout(900,lambda:self.compareImage("testContAnim4", False))
            Player.setTimeout(1000,Player.stop)

        def startAnim():
            node=Player.getElementByID("mainimg")
            self.anim=anim.ContinuousAnim(node,"angle",0,1,0)
            self.anim.start()

        def startAnim2():
            node=Player.getElementByID("nestedimg1")
            self.anim2=anim.ContinuousAnim(node,"width",0,50,0)
            self.anim2.start()

        def startAnim3():
            node=Player.getElementByID("nestedimg2")
            self.anim3=anim.ContinuousAnim(node,"x",0,50,0)
            self.anim3.start()

        def stopAnim():
            self.anim.abort()
            self.anim2.abort()
            self.anim3.abort()
            self.anim = None
            self.anim2 = None
            self.anim3 = None

        Player.setFakeFPS(25)
        anim.init(avg)
        Player.loadFile("avg.avg")
        Player.setTimeout(1, onStart)
        Player.play()

    def testWaitAnim(self):
        def animStopped():
            self.__endCalled = True

        def startAnim():
            self.anim = anim.WaitAnim(200, animStopped, False)
            self.anim.start()

        anim.init(avg)
        Player.setFakeFPS(10)
        self.__endCalled = False
        self.start("image.avg",
                (startAnim, 
                 lambda: self.assert_(not(self.anim.isDone())),
                 None,
                 None,
                 lambda: self.assert_(self.anim.isDone()),
                 lambda: self.assert_(self.__endCalled)
                ))

    def testStateAnim(self):
        def state2Callback():
            self.__state2CallbackCalled = True

        def makeAnim():
            node = Player.getElementByID("test")
            self.anim = anim.StateAnim(
                    {"STATE1": anim.LinearAnim(node, "x", 200, 64, 128),
                     "STATE2": anim.LinearAnim(node, "x", 200, 128, 64),
                     "STATE3": anim.WaitAnim()},
                    {"STATE1": anim.AnimTransition("STATE2", state2Callback),
                     "STATE2": anim.AnimTransition("STATE3")})
        anim.init(avg)
        Player.setFakeFPS(10)
        self.__state2CallbackCalled = False
        self.start("image.avg",
                (makeAnim,
                 lambda: self.compareImage("testStateAnim1", False),
                 lambda: self.anim.setState("STATE1"),
                 None,
                 lambda: self.compareImage("testStateAnim2", False),
                 lambda: self.anim.getState() == "STATE2",
                 lambda: self.compareImage("testStateAnim3", False),
                 lambda: self.assert_(self.__state2CallbackCalled),
                 lambda: self.anim.getState() == "STATE3",
                 lambda: self.compareImage("testStateAnim4", False),
                 lambda: self.anim.setState("STATE1"),
                 lambda: self.assert_(anim.getNumRunningAnims() == 1),
                 lambda: self.compareImage("testStateAnim5", False)
                ))

    def testParallelAnim(self):
        def animStopped():
            self.__endCalled = True

        def startAnim():
            node0 = Player.getElementByID("mainimg")
            node1 = Player.getElementByID("test")
            node2 = Player.getElementByID("test1")
            self.anim = anim.ParallelAnim(
                    [ anim.SplineAnim(node1, "x", 400, 0, 40, 0, 0),
                      anim.EaseInOutAnim(node2, "x", 300, 129, 99, 100, 100)
                    ], animStopped)
            self.anim.start()
        
        anim.init(avg)
        self.__endCalled = False
        Player.setFakeFPS(10)
        self.start("image.avg",
                (startAnim,
                 lambda: self.assert_(anim.getNumRunningAnims() == 2),
                 lambda: self.assert_(not(self.anim.isDone())),
                 lambda: self.compareImage("testParallelAnims1", False),
                 None,
                 None,
                 lambda: self.compareImage("testParallelAnims2", False),
                 lambda: self.assert_(self.anim.isDone()),
                 lambda: self.assert_(self.__endCalled)
                ))

    def testDraggable(self):
        def onDragStart(event):
            self.__dragStartCalled = True
        
        def onDragEnd(event):
            self.__dragEndCalled = True
        
        def startDrag():
            Helper.fakeMouseEvent(avg.CURSORDOWN, True, False, False, 140, 40, 1)
        
        def move():
            Helper.fakeMouseEvent(avg.CURSORMOTION, True, False, False, 150, 50, 1)
        
        def stop():
            Helper.fakeMouseEvent(avg.CURSORUP, True, False, False, 140, 40, 1)
        
        self.__dragEndCalled = False
        self.__dragStartCalled = False
        Helper = Player.getTestHelper()    
        Player.loadFile("image.avg")
        draggable.init(avg)
        dragger = draggable.Draggable(Player.getElementByID("test1"),
                onDragStart, onDragEnd)
        dragger.enable()
        self.start(None,
                (startDrag,
                 lambda: self.assert_(self.__dragStartCalled),
                 move,
                 lambda: self.compareImage("testDraggable1", False),
                 stop,
                 lambda: self.assert_(self.__dragEndCalled),
                 lambda: self.compareImage("testDraggable2", False),
                 dragger.disable,
                 startDrag,
                 move,
                 lambda: self.compareImage("testDraggable2", False)
                ))

    def testButton(self):
        def onDown(event):
            self.__down = True

        def onClick(event):
            self.__clicked = True

        def reset():
            self.__down = False
            self.__clicked = False
        
        def printAddress(obj):
            print obj
        
        def setObjectActive(obj, active):
            obj.active = active
            
        self.loadEmptyScene()
        
        b = ui.Button(
                parent = Player.getRootNode(),
                upNode = avg.ImageNode(href="button_up.png"),
                downNode = avg.ImageNode(href="button_down.png"),
                disabledNode = avg.ImageNode(href="button_disabled.png"),
                pressHandler = onDown,
                clickHandler = onClick)
        
        b1 = ui.Button(parent=Player.getRootNode(),
                       active=False,
                       pressHandler=onDown,
                       clickHandler=onClick)
        
        b.pos = (0, 0)
        xOutDistance = int(b.width * 2)
        yOutDistance = int(b.height * 2)

        self.__down = False
        self.__clicked = False
        
        self.start(None,
                (# Normal click: Down & up over button
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self.__sendMouseEvent(avg.CURSORDOWN, 0, 0),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 lambda: self.__sendMouseEvent(avg.CURSORUP, 0, 0),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self.assert_(self.__clicked),
                 reset,

                 # Down, move away from button, move over button, up
                 # ==> click
                 lambda: self.__sendMouseEvent(avg.CURSORDOWN, 0, 0),
                 lambda: self.__sendMouseEvent(avg.CURSORMOTION, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self.__sendMouseEvent(avg.CURSORMOTION, 0, 0),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.__sendMouseEvent(avg.CURSORUP, 0, 0),
                 lambda: self.assert_(self.__down and self.__clicked),
                 reset,

                 # Down, move away from button, up ==> no click
                 lambda: self.__sendMouseEvent(avg.CURSORDOWN, 0, 0),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.__sendMouseEvent(avg.CURSORMOTION, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self.__sendMouseEvent(avg.CURSORUP, 0, yOutDistance),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 reset,
                 
                 # Move away from button, down, mover over button, up
                 # ==> no click
                 lambda: self.__sendMouseEvent(avg.CURSORDOWN, 0, yOutDistance),
                 lambda: self.__sendMouseEvent(avg.CURSORMOTION, 0, 0),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self.__sendMouseEvent(avg.CURSORUP, 0, 0),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self.assert_(not(self.__down) and not(self.__clicked)),
                 reset,

                 # Move away from button, down, mover over button, move away from button, up
                 # ==> no click
                 lambda: self.__sendMouseEvent(avg.CURSORDOWN, 0, yOutDistance),
                 lambda: self.__sendMouseEvent(avg.CURSORMOTION, 0, 0),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self.__sendMouseEvent(avg.CURSORMOTION, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self.__sendMouseEvent(avg.CURSORUP, 0, yOutDistance),
                 lambda: self.assert_(not(self.__down) and not(self.__clicked)),
                 reset,

                 # Check checkable button
                 # Set checkable, down, up => pressed, down, up ==> click
                 lambda: b.setCheckable(True),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self.__sendMouseEvent(avg.CURSORDOWN, 0, 0),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.__sendMouseEvent(avg.CURSORUP, 0, 0),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.assert_(self.__down and self.__clicked),
                 reset,
                 lambda: self.__sendMouseEvent(avg.CURSORDOWN, 0, 0),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.__sendMouseEvent(avg.CURSORUP, 0, 0),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self.assert_(self.__down and self.__clicked),
                 reset,

                 # not checked, down, out, in, up ==> pressed, click
                 lambda: b.setCheckable(True),
                 lambda: b.setChecked(False),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self.__sendMouseEvent(avg.CURSORDOWN, 0, 0),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.__sendMouseEvent(avg.CURSORMOTION, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self.__sendMouseEvent(avg.CURSORMOTION, 0, 0),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.__sendMouseEvent(avg.CURSORUP, 0, 0),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.assert_(self.__down and self.__clicked),
                 reset,
                 
                 # not checked, down, out, up ==> pressed, no click
                 lambda: b.setCheckable(True),
                 lambda: b.setChecked(False),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self.__sendMouseEvent(avg.CURSORDOWN, 0, 0),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.__sendMouseEvent(avg.CURSORMOTION, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self.__sendMouseEvent(avg.CURSORUP, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 reset,
                 
                 # not checked, down, out, up ==> pressed, no click
                 lambda: b.setCheckable(True),
                 lambda: b.setChecked(True),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.__sendMouseEvent(avg.CURSORDOWN, 0, 0),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.__sendMouseEvent(avg.CURSORMOTION, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.__sendMouseEvent(avg.CURSORUP, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 reset,
                 
                 # checked, down, out, in, up ==> pressed, clicked
                 lambda: b.setCheckable(True),
                 lambda: b.setChecked(True),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.__sendMouseEvent(avg.CURSORDOWN, 0, 0),
                 lambda: self.assert_(self.__down),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.__sendMouseEvent(avg.CURSORMOTION, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.__sendMouseEvent(avg.CURSORMOTION, 0, 0),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.__sendMouseEvent(avg.CURSORUP, 0, 0),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self.assert_(self.__down and self.__clicked),
                 reset,
                 
                 # Test public interface
                 lambda: b.setCheckable(False),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self.assert_(b.isEnabled()),
                 lambda: self.assert_(not(b.isCheckable())),
                 lambda: b.setCheckable(True),
                 lambda: self.assert_(b.isCheckable()),
                 lambda: b.setChecked(True),
                 lambda: self.assert_(b.isChecked()),
                 lambda: b.setEnabled(False),
                 lambda: self.assert_(not(b.isEnabled())),
                 lambda: self.assert_(not(self.__down) and not(self.__clicked)),
                 lambda: b.setEnabled(True),
                 lambda: b.setChecked(False),
                 reset,
                 
                 # Disable: Various up/down combinations have no effect
                 lambda: b.setEnabled(False),
                 lambda: self.__sendMouseEvent(avg.CURSORUP, 0, 0),
                 lambda: self.compareImage("testUIButtonDisabled", False),
                 lambda: self.assert_(not(self.__down) and not(self.__clicked)),
                 lambda: self.__sendMouseEvent(avg.CURSORDOWN, 0, 0),
                 lambda: self.compareImage("testUIButtonDisabled", False),
                 lambda: self.__sendMouseEvent(avg.CURSORUP, 0, 0),
                 lambda: self.compareImage("testUIButtonDisabled", False),
                 lambda: self.assert_(not(self.__down) and not(self.__clicked)),
                 lambda: self.__sendMouseEvent(avg.CURSORDOWN, 0, 0),
                 lambda: self.compareImage("testUIButtonDisabled", False),
                 lambda: b.setEnabled(True),
                 lambda: self.__sendMouseEvent(avg.CURSORUP, 0, 0),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self.assert_(not(self.__down)),
                 lambda: self.assert_(not(self.__clicked)),
                 reset,

                 # Checking functionality while resetting the visible nodes
                 lambda: b.setNodes(upNode = avg.ImageNode(href="button_up.png"),
                                    downNode = avg.ImageNode(href="button_down.png"),
                                    disabledNode = avg.ImageNode(href="button_disabled.png")),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self.__sendMouseEvent(avg.CURSORDOWN, 0, 0),
                 lambda: self.__sendMouseEvent(avg.CURSORUP, 0, 0),
                 lambda: self.assert_(self.__down and self.__clicked),
                 reset,
                 
                 lambda: b.setEnabled(False),
                 lambda: b.setNodes(upNode = avg.ImageNode(href="button_up.png"),
                                    downNode = avg.ImageNode(href="button_down.png"),
                                    disabledNode = avg.ImageNode(href="button_disabled.png")),
                 lambda: self.compareImage("testUIButtonDisabled", False),
                 lambda: self.__sendMouseEvent(avg.CURSORDOWN, 0, 0),
                 lambda: self.__sendMouseEvent(avg.CURSORUP, 0, 0),
                 lambda: self.assert_(not(self.__down) and not(self.__clicked)),
                 reset,
                 
                 lambda: b.setEnabled(True),
                 lambda: b.setNodes(upNode = avg.ImageNode(href="button_up.png"),
                                    downNode = avg.ImageNode(href="button_down.png"),
                                    disabledNode = None),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self.__sendMouseEvent(avg.CURSORDOWN, 0, 0),
                 lambda: self.__sendMouseEvent(avg.CURSORUP, 0, 0),
                 lambda: self.assert_(self.__down and self.__clicked),
                 reset,
                 
                 # resetting the nodes on an empty button
                 lambda: setObjectActive(b, False),
                 lambda: setObjectActive(b1, True),
                 lambda: b1.setNodes(upNode = avg.ImageNode(href="button_up.png"),
                                     downNode = avg.ImageNode(href="button_down.png"),
                                     disabledNode = avg.ImageNode(href="button_disabled.png")),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self.__sendMouseEvent(avg.CURSORDOWN, 0, 0),
                 lambda: self.__sendMouseEvent(avg.CURSORUP, 0, 0),
                 lambda: self.assert_(self.__down and self.__clicked),
                 ))
    
        
    def testMultitouchButton(self):
        def onDown(event):
            self.__down = True

        def onClick(event):
            self.__clicked = True

        def reset():
            self.__down = False
            self.__clicked = False

        self.loadEmptyScene()
        b = ui.Button(
                parent = Player.getRootNode(),
                upNode = avg.ImageNode(href="button_up.png"),
                downNode = avg.ImageNode(href="button_down.png"),
                disabledNode = avg.ImageNode(href="button_disabled.png"),
                pressHandler = onDown,
                clickHandler = onClick,
                )
        b.pos = (0,0)
        xOutDistance = b.width * 2
        yOutDistance = b.height * 2
        
        self.__down = False
        self.__clicked = False
        self.start(None,
                (# Two downs, two ups ==> click after second up.
                 lambda: self.__sendTouchEvent(1, avg.CURSORDOWN, 0, 0),
                 lambda: self.__sendTouchEvent(2, avg.CURSORDOWN, 0, 0),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 lambda: self.__sendTouchEvent(1, avg.CURSORUP, 0, 0),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 lambda: self.__sendTouchEvent(2, avg.CURSORUP, 0, 0),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self.assert_(self.__clicked),
                 reset,

                 # Two downs, one out, out up, in up ==> click
                 lambda: self.__sendTouchEvent(1, avg.CURSORDOWN, 0, 0),
                 lambda: self.__sendTouchEvent(2, avg.CURSORDOWN, 0, 0),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.__sendTouchEvent(2, avg.CURSORMOTION, 0, yOutDistance),
                 lambda: self.__sendTouchEvent(2, avg.CURSORUP, 0, 50),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 lambda: self.__sendTouchEvent(1, avg.CURSORUP, 0, 0),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self.assert_(self.__down and self.__clicked),
                 reset,
                 
                 # Two down, both out, both in, both up ==> click
                 lambda: self.__sendTouchEvent(1, avg.CURSORDOWN, 0, 0),
                 lambda: self.__sendTouchEvent(2, avg.CURSORDOWN, 0, 0),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.__sendTouchEvent(1, avg.CURSORMOTION, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.__sendTouchEvent(2, avg.CURSORMOTION, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 lambda: self.__sendTouchEvent(1, avg.CURSORMOTION, 0, 0),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.__sendTouchEvent(2, avg.CURSORMOTION, 0, 0),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.__sendTouchEvent(1, avg.CURSORUP, 0, 0),
                 lambda: self.__sendTouchEvent(2, avg.CURSORUP, 0, 0),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self.assert_(self.__down and self.__clicked),
                 reset,

                 # Two down both out, both up ==> no click
                 lambda: self.__sendTouchEvent(1, avg.CURSORDOWN, 0, 0),
                 lambda: self.__sendTouchEvent(2, avg.CURSORDOWN, 0, 0),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.__sendTouchEvent(1, avg.CURSORMOTION, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.__sendTouchEvent(2, avg.CURSORMOTION, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self.__sendTouchEvent(1, avg.CURSORUP, 0, yOutDistance),
                 lambda: self.__sendTouchEvent(2, avg.CURSORUP, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 reset,
                                                   
                 # Two downs, one out, in up, out up ==> no click
                 lambda: self.__sendTouchEvent(1, avg.CURSORDOWN, 0, 0),
                 lambda: self.__sendTouchEvent(2, avg.CURSORDOWN, 0, 0),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.__sendTouchEvent(2, avg.CURSORMOTION, 0, yOutDistance),
                 lambda: self.__sendTouchEvent(1, avg.CURSORUP, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 lambda: self.__sendTouchEvent(2, avg.CURSORUP, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 reset,
                 
                 # Check checkable multitouch button
                 # 2 down, 2 up ==> pressed, clicked, 
                 # 2 down, 2 up ==> pressed, clicked
                 lambda: b.setCheckable(True),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self.__sendTouchEvent(1, avg.CURSORDOWN, 0, 0),
                 lambda: self.__sendTouchEvent(2, avg.CURSORDOWN, 0, 0),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 lambda: self.__sendTouchEvent(1, avg.CURSORUP, 0, 0),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.__sendTouchEvent(2, avg.CURSORUP, 0, 0),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.assert_(self.__down and self.__clicked),
                 reset,
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.__sendTouchEvent(1, avg.CURSORDOWN, 0, 0),
                 lambda: self.__sendTouchEvent(2, avg.CURSORDOWN, 0, 0),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 lambda: self.__sendTouchEvent(1, avg.CURSORUP, 0, 0),
                 lambda: self.__sendTouchEvent(2, avg.CURSORUP, 0, 0),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self.assert_(self.__down and self.__clicked),
                 reset,
                 
                 # not checked, 2 down, 2 out, 2 in, first up, second up 
                 # ==> pressed, clicked
                 lambda: b.setChecked(False),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self.__sendTouchEvent(1, avg.CURSORDOWN, 0, 0),
                 lambda: self.__sendTouchEvent(2, avg.CURSORDOWN, 0, 0),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 lambda: self.__sendTouchEvent(1, avg.CURSORMOTION, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.__sendTouchEvent(2, avg.CURSORMOTION, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self.__sendTouchEvent(1, avg.CURSORMOTION, 0, 0),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.__sendTouchEvent(2, avg.CURSORMOTION, 0, 0),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.__sendTouchEvent(1, avg.CURSORUP, 0, 0),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 lambda: self.__sendTouchEvent(2, avg.CURSORUP, 0, 0),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.assert_(self.__down and self.__clicked),
                 reset,
                 
                 # checked, 2 down, 2 out, 2 in, first up, second up 
                 # ==> pressed, clicked
                 lambda: b.setCheckable(True),
                 lambda: b.setChecked(True),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.__sendTouchEvent(1, avg.CURSORDOWN, 0, 0),
                 lambda: self.__sendTouchEvent(2, avg.CURSORDOWN, 0, 0),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 lambda: self.__sendTouchEvent(1, avg.CURSORMOTION, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.__sendTouchEvent(2, avg.CURSORMOTION, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.__sendTouchEvent(1, avg.CURSORMOTION, 0, 0),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.__sendTouchEvent(2, avg.CURSORMOTION, 0, 0),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.__sendTouchEvent(1, avg.CURSORUP, 0, 0),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 lambda: self.__sendTouchEvent(2, avg.CURSORUP, 0, 0),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self.assert_(self.__down and self.__clicked),
                 reset,
                 
                 # not checked, 2 down, 2 out, first up, second up 
                 # ==> pressed, not clicked
                 lambda: b.setCheckable(True),
                 lambda: b.setChecked(False),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self.__sendTouchEvent(1, avg.CURSORDOWN, 0, 0),
                 lambda: self.__sendTouchEvent(2, avg.CURSORDOWN, 0, 0),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 lambda: self.__sendTouchEvent(1, avg.CURSORMOTION, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.__sendTouchEvent(2, avg.CURSORMOTION, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self.__sendTouchEvent(1, avg.CURSORUP, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 lambda: self.__sendTouchEvent(2, avg.CURSORUP, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 reset,
                 
                 # checked, 2 down, 2 out, first up, second up 
                 # ==> pressed, not clicked
                 lambda: b.setCheckable(True),
                 lambda: b.setChecked(True),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.__sendTouchEvent(1, avg.CURSORDOWN, 0, 0),
                 lambda: self.__sendTouchEvent(2, avg.CURSORDOWN, 0, 0),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 lambda: self.__sendTouchEvent(1, avg.CURSORMOTION, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.__sendTouchEvent(2, avg.CURSORMOTION, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.__sendTouchEvent(1, avg.CURSORUP, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 lambda: self.__sendTouchEvent(2, avg.CURSORUP, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 reset,
                ))

    def testKeyboard(self):
        def setup():
            keyDefs = [
                    [("a", "A"), ( 5, 5), (30, 30)],
                    [(1, ),      (35, 5), (30, 30)],
                    ["SHIFT",    (65, 5), (50, 30)]]
            kbNoShift = ui.Keyboard("keyboard_bg.png", "keyboard_ovl.png", keyDefs, None,
                    pos=(10, 10), parent = Player.getRootNode())
            kbNoShift.setKeyHandler(onKeyDown, onKeyUp)
            kbShift = ui.Keyboard("keyboard_bg.png", "keyboard_ovl.png", keyDefs, "SHIFT",
                    pos=(10, 60), parent = Player.getRootNode())
            kbShift.setKeyHandler(onKeyDown, onKeyUp)

        def onKeyDown(event, char, cmd):
            self.__keyDown = True
            self.__keyUp   = False
            self.__char = char
            self.__cmd = cmd

        def onKeyUp(event, char, cmd):
            self.__keyDown = False
            self.__keyUp   = True
            self.__char = char
            self.__cmd = cmd

        self.loadEmptyScene()

        kbNoShift = None
        kbShift   = None
        self.__keyDown = False
        self.__keyUp   = True
        self.__char = "foo"
        self.__cmd = "bar"
        self.start(None,
                (setup,
                 lambda: self.compareImage("testUIKeyboard", False),
                 # test character key
                 lambda: self.__sendMouseEvent(avg.CURSORDOWN, 30, 30),
                 lambda: self.assert_(self.__keyDown and not self.__keyUp),
                 lambda: self.assert_(self.__char == "a" and self.__cmd is None),
                 lambda: self.compareImage("testUIKeyboardDownA1", False),
                 lambda: self.__sendMouseEvent(avg.CURSORUP, 30, 30),
                 lambda: self.assert_(not self.__keyDown and self.__keyUp),
                 lambda: self.assert_(self.__char == "a" and self.__cmd is None),
                 lambda: self.compareImage("testUIKeyboard", False),
                 # test command key
                 lambda: self.__sendMouseEvent(avg.CURSORDOWN, 100, 30),
                 lambda: self.assert_(self.__keyDown and not self.__keyUp),
                 lambda: self.assert_(self.__char is None and self.__cmd == "SHIFT"),
                 lambda: self.compareImage("testUIKeyboardDownS1", False),
                 lambda: self.__sendMouseEvent(avg.CURSORUP, 100, 30),
                 lambda: self.assert_(not self.__keyDown and self.__keyUp),
                 lambda: self.assert_(self.__char is None and self.__cmd == "SHIFT"),
                 lambda: self.compareImage("testUIKeyboard", False),
                 # test shift key (no shift key support)
                 lambda: self.__sendTouchEvent(1, avg.CURSORDOWN, 100, 30),
                 lambda: self.__sendTouchEvent(2, avg.CURSORDOWN, 30, 30),
                 lambda: self.assert_(self.__char == "a" and self.__cmd is None),
                 lambda: self.__sendTouchEvent(3, avg.CURSORDOWN, 60, 30),
                 lambda: self.assert_(self.__char == 1 and self.__cmd is None),
                 lambda: self.compareImage("testUIKeyboardDownA111S1", False),
                 lambda: self.__sendTouchEvent(2, avg.CURSORUP, 30, 30),
                 lambda: self.assert_(self.__char == "a" and self.__cmd is None),
                 lambda: self.__sendTouchEvent(3, avg.CURSORUP, 60, 30),
                 lambda: self.assert_(self.__char == 1 and self.__cmd is None),
                 lambda: self.__sendTouchEvent(1, avg.CURSORUP, 100, 30),
                 lambda: self.compareImage("testUIKeyboard", False),
                 # test shift key (with shift key support)
                 lambda: self.__sendTouchEvent(1, avg.CURSORDOWN, 100, 80),
                 lambda: self.__sendTouchEvent(2, avg.CURSORDOWN, 30, 80),
                 lambda: self.assert_(self.__char == "A" and self.__cmd is None),
                 lambda: self.__sendTouchEvent(3, avg.CURSORDOWN, 60, 80),
                 lambda: self.assert_(self.__char == 1 and self.__cmd is None),
                 lambda: self.compareImage("testUIKeyboardDownA212S2", False),
                 lambda: self.__sendTouchEvent(2, avg.CURSORUP, 30, 80),
                 lambda: self.assert_(self.__char == "A" and self.__cmd is None),
                 lambda: self.__sendTouchEvent(3, avg.CURSORUP, 60, 80),
                 lambda: self.assert_(self.__char == 1 and self.__cmd is None),
                 lambda: self.__sendTouchEvent(1, avg.CURSORUP, 100, 80),
                 lambda: self.compareImage("testUIKeyboard", False)
                ))

    def testTextArea(self):
        def setup():
            self.ta1 = textarea.TextArea(Player.getElementByID('ph1'), id='ta1')
            self.ta1.setStyle(font='Bitstream Vera Sans', variant='Roman',
                fontsize=16, multiline=True, color='FFFFFF')
            self.ta1.setText('Lorem ipsum')
            self.ta1.setFocus(True) # TODO: REMOVE

            self.ta2 = textarea.TextArea(Player.getElementByID('ph2'), id='ta2')
            self.ta2.setStyle(font='Bitstream Vera Sans', variant='Roman',
                fontsize=14, multiline=False, color='FFFFFF')
            self.ta2.setText('sit dolor')
            self.ta2.setFocus(True) # TODO: REMOVE
            
        def setAndCheck(ta, text):
            ta.setText(text)
            self.assert_(ta.getText() == text)
        
        def clear(ta):
            ta.onKeyDown(textarea.KEYCODE_FORMFEED)
            self.assert_(ta.getText() == '')
        
        def testUnicode():
            self.ta1.setText(u'some ùnìcöde')
            self.ta1.onKeyDown(textarea.KEYCODES_BACKSPACE[0])
            self.assert_(self.ta1.getText() == u'some ùnìcöd')
            self.ta1.onKeyDown(textarea.KEYCODES_BACKSPACE[1])
            self.ta1.onKeyDown(textarea.KEYCODES_BACKSPACE[0])
            self.assert_(self.ta1.getText() == u'some ùnìc')
            self.ta1.onKeyDown(ord(u'Ä'))
            self.assert_(self.ta1.getText() == u'some ùnìcÄ')
        
        def testSpecialChars():
            clear(self.ta1)
            self.ta1.onKeyDown(ord(u'&'))
            self.ta1.onKeyDown(textarea.KEYCODES_BACKSPACE[0])
            self.assert_(self.ta1.getText() == '')
        
        def checkSingleLine():
            text = ''
            self.ta2.setText('')
            while True:
                self.assert_(len(text) < 20)
                self.ta2.onKeyDown(ord(u'A'))
                text = text + 'A'
                if text != self.ta2.getText():
                    self.assert_(len(text) == 16)
                    break
            
        Player.loadString("""
        <avg width="160" height="120">
            <div id="ph1" x="2" y="2" width="156" height="96"/>
            <div id="ph2" x="2" y="100" width="156" height="18"/>
        </avg>
        """)
        
        import time
        textarea.init(avg, False)
        self.start(None,
               (setup,
                lambda: self.assert_(self.ta1.getText() == 'Lorem ipsum'),
                lambda: setAndCheck(self.ta1, ''),
                lambda: setAndCheck(self.ta2, 'Lorem Ipsum'),
                testUnicode,
                lambda: self.compareImage("testTextArea1", True),
                testSpecialChars,
                checkSingleLine,
                lambda: self.compareImage("testTextArea2", True),
               ))

    def testDragProcessor(self):
        def onDragStart(event):
            self.__dragStartCalled = True

        def onDrag(event, offset):
            if self.friction == -1:
                self.assert_(offset == (40,40))
            self.__dragMoveCalled = True

        def onDragUp(event, offset):
            if self.friction == -1:
                self.assert_(offset == (10,-10))
            self.__dragUpCalled = True

        def onDragStop():
            self.__dragStopCalled = True

        def disable():
            dragProcessor.enable(False)
            initState()

        def initState():
            self.__dragStartCalled = False
            self.__dragMoveCalled = False
            self.__dragUpCalled = False
            self.__dragStopCalled = False

        def assertDragEvents(start, move, up, stop):
            self.assert_(self.__dragStartCalled == start and
                self.__dragMoveCalled == move and
                self.__dragUpCalled == up and
                self.__dragStopCalled == stop)

        Player.setFakeFPS(100)
        for self.friction in (-1, 100):
            self.loadEmptyScene()
            image = avg.ImageNode(parent=Player.getRootNode(), href="rgb24-64x64.png")
            dragProcessor = ui.DragProcessor(image, 
                    startHandler=onDragStart, moveHandler=onDrag, upHandler=onDragUp, 
                    stopHandler=onDragStop, friction=self.friction)
            initState()
            self.start(None,
                    (lambda: self.__sendMouseEvent(avg.CURSORDOWN, 30, 30),
                     lambda: assertDragEvents(True, False, False, False),
                     lambda: self.__sendMouseEvent(avg.CURSORMOTION, 70, 70, 40, 40),
                     lambda: assertDragEvents(True, True, False, False),
                     lambda: self.__sendMouseEvent(avg.CURSORUP, 40, 20, -30, -50),
                     lambda: assertDragEvents(True, True, True, True),
                     disable,
                     lambda: self.__sendMouseEvent(avg.CURSORDOWN, 30, 30),
                     lambda: assertDragEvents(False, False, False, False),
                     lambda: dragProcessor.enable(True),
                     lambda: self.__sendMouseEvent(avg.CURSORUP, 30, 30),
                     lambda: self.__sendMouseEvent(avg.CURSORDOWN, 30, 30),
                     lambda: assertDragEvents(True, False, False, False),
                    ))
        Player.setFakeFPS(-1)


    def testDragProcessorInitialEvent(self):
        def onDown(event):
            dragProcessor = ui.DragProcessor(self.image, 
                    startHandler=onDragStart, initialEvent=event)
           
        def onDragStart(event):
            self.__dragStartCalled = True

        self.loadEmptyScene()
        self.image = avg.ImageNode(parent=Player.getRootNode(), href="rgb24-64x64.png")
        self.image.connectEventHandler(avg.CURSORDOWN, avg.MOUSE, self, onDown)
        self.__dragStartCalled = False
        self.start(None,
                (lambda: self.__sendMouseEvent(avg.CURSORDOWN, 30, 30),
                ))
        assert(self.__dragStartCalled)
            

    def testHoldProcessor(self):
      
        def onStart(pos):
            self.__startCalled = True
            self.assert_(self.__holdProcessor.getLastEvent().pos == pos)
            return True

        def onHold(time):
            self.__holdCalled = True
            self.assert_(time >= 0 and time <= 1)

        def onActivate():
            self.__activateCalled = True

        def onStop():
            self.__stopCalled = True

        def initState():
            self.__startCalled = False
            self.__holdCalled = False
            self.__activateCalled = False
            self.__stopCalled = False

        def assertEvents(start, hold, activate, stop):
#            print (self.__startCalled, self.__holdCalled, self.__activateCalled,
#                    self.__stopCalled)
            self.assert_(self.__startCalled == start and
                self.__holdCalled == hold and
                self.__activateCalled == activate and
                self.__stopCalled == stop)

        Player.setFakeFPS(2)
        self.loadEmptyScene()
        image = avg.ImageNode(parent=Player.getRootNode(), href="rgb24-64x64.png")
        self.__holdProcessor = ui.HoldProcessor(image,
            holdDelay=1000,
            activateDelay=2000, 
            startHandler=onStart, 
            holdHandler=onHold, 
            activateHandler=onActivate, 
            stopHandler=onStop)
        initState()
        self.start(None,
                (# Standard down-hold-up sequence.
                 lambda: self.__sendMouseEvent(avg.CURSORDOWN, 30, 30),
                 lambda: assertEvents(False, False, False, False),
                 None,
                 None,
                 lambda: assertEvents(True, True, False, False),
                 None,
                 None,
                 lambda: assertEvents(True, True, True, False),
                 lambda: self.__sendMouseEvent(avg.CURSORUP, 30, 30),
                 lambda: assertEvents(True, True, True, True),
                 
                 # down-up sequence, hold not long enough.
                 initState,
                 lambda: self.__sendMouseEvent(avg.CURSORDOWN, 30, 30),
                 None,
                 None,
                 lambda: self.__sendMouseEvent(avg.CURSORUP, 30, 30),
                 lambda: assertEvents(True, True, False, True),

                 # down-move-up sequence, should abort. 
                 initState,
                 lambda: self.__sendMouseEvent(avg.CURSORDOWN, 30, 30),
                 None,
                 None,
                 lambda: self.__sendMouseEvent(avg.CURSORMOTION, 40, 40),
                 lambda: assertEvents(True, True, False, True),
                 lambda: self.__sendMouseEvent(avg.CURSORMOTION, 50, 50),
                 initState,
                 lambda: self.__sendMouseEvent(avg.CURSORUP, 40, 40),
                 lambda: assertEvents(False, False, False, False),
                ))

        Player.setFakeFPS(-1)

    def testFocusContext(self):
        def setup():
            textarea.init(avg)
            self.ctx1 = textarea.FocusContext()
            self.ctx2 = textarea.FocusContext()
            
            self.ta1 = textarea.TextArea(Player.getElementByID('ph1'),
                self.ctx1, id='ta1')
            self.ta1.setStyle(font='Bitstream Vera Sans', variant='Roman',
                fontsize=16, multiline=True, color='FFFFFF')
            self.ta1.setText('Lorem ipsum')
            
            self.ta2 = textarea.TextArea(Player.getElementByID('ph2'),
                self.ctx1, id='ta2')
            self.ta2.setStyle(font='Bitstream Vera Sans', variant='Roman',
                fontsize=14, multiline=False, color='FFFFFF')
            self.ta2.setText('dolor')
            
            self.ta3 = textarea.TextArea(Player.getElementByID('ph3'),
                self.ctx2, disableMouseFocus=True, id='ta3')
            self.ta3.setStyle(font='Bitstream Vera Sans', variant='Roman',
                fontsize=14, multiline=True, color='FFFFFF')
            self.ta3.setText('dolor sit amet')
            
            textarea.setActiveFocusContext(self.ctx1)

        def writeChar():
            helper = Player.getTestHelper()
            helper.fakeKeyEvent(avg.KEYDOWN, 65, 65, "A", 65, 0)
            helper.fakeKeyEvent(avg.KEYUP, 65, 65, "A", 65, 0)
            helper.fakeKeyEvent(avg.KEYDOWN, 66, 66, "B", 66, 0)
            helper.fakeKeyEvent(avg.KEYUP, 66, 66, "B", 66, 0)
            helper.fakeKeyEvent(avg.KEYDOWN, 67, 67, "C", 67, 0)
            helper.fakeKeyEvent(avg.KEYUP, 67, 67, "C", 67, 0)

        def switchFocus():
            self.ctx1.cycleFocus()

        def clearFocused():
            self.ctx1.clear()

        def clickForFocus():
            self.__sendMouseEvent(avg.CURSORDOWN, 20, 70)
            self.__sendMouseEvent(avg.CURSORUP, 20, 70)

        Player.loadString("""
       <avg width="160" height="120">
           <div id="ph1" x="2" y="2" width="156" height="54"/>
           <div id="ph2" x="2" y="58" width="76" height="54"/>
           <div id="ph3" x="80" y="58" width="76" height="54">
               <image href="1x1_white.png" width="76" height="54"/>
           </div>
       </avg>
       """)
        self.start(None,
                (setup,
                 lambda: self.compareImage("testFocusContext1", True),
                 writeChar,
                 lambda: self.compareImage("testFocusContext2", True),
                 switchFocus,
                 writeChar,
                 lambda: self.compareImage("testFocusContext3", True),
                 switchFocus,
                 clearFocused,
                 lambda: self.compareImage("testFocusContext4", True),
                 clickForFocus,
                 clearFocused,
                 lambda: self.compareImage("testFocusContext5", True),
               ))

    def testRoundedRect(self):
        def setPos():
            self.rect.pos = (20.5, 3.5)

        def setSize():
            self.rect.size = (40, 20)

        def setRadius(r):
            self.rect.radius = r

        def setFill():
            self.rect.fillcolor = "0000FF"
            self.rect.fillopacity = 0.5

        self.loadEmptyScene()
        self.rect = geom.RoundedRect(parent=Player.getRootNode(), pos=(2.5,2.5), 
                size=(64,64), radius=5, color="FF0000")
        self.start(None,
                (lambda: self.compareImage("testRoundedRect1", True),
                 setPos,
                 lambda: self.compareImage("testRoundedRect2", True),
                 setSize,
                 lambda: self.compareImage("testRoundedRect3", True),
                 lambda: self.assertException(setRadius(15)),
                 lambda: setRadius(10),
                 lambda: self.compareImage("testRoundedRect4", True),
                 setFill,
                 lambda: self.compareImage("testRoundedRect5", True),
                ))

    def testPieSlice(self):
        def changeAttrs():
            self.pieSlice.startangle = -1
            self.pieSlice.endangle = 3.14
            self.pieSlice.radius = 50
            self.pieSlice.pos = (80.5, 60.5)
            self.pieSlice.fillcolor = "00FFFF"
            self.pieSlice.fillopacity = 0.5

        def makeSmall():
            self.pieSlice.radius = 0.6

        self.loadEmptyScene()
        self.pieSlice = geom.PieSlice(parent=Player.getRootNode(), pos=(20.5,20.5), 
                radius=40, startangle=0, endangle=1.57, color="FF0000")
       
        self.start(None,
                (lambda: self.compareImage("testPieSlice1", True),
                 changeAttrs,
                 lambda: self.compareImage("testPieSlice2", True),
                 makeSmall,
                 lambda: self.compareImage("testPieSlice3", True),
                ))

    def testArc(self):
        def changeAttrs():
            self.arc.startangle = -1
            self.arc.endangle = 3.14
            self.arc.radius = 50
            self.arc.pos = (80.5, 60.5)

        self.loadEmptyScene()
        self.arc = geom.Arc(parent=Player.getRootNode(), pos=(20.5,20.5), 
                radius=40, startangle=0, endangle=1.57, color="FF0000")
       
        self.start(None,
                (lambda: self.compareImage("testArc1", True),
                 changeAttrs,
                 lambda: self.compareImage("testArc2", True),
                ))

    def testStateMachine(self):
        def atob(oldState, newState):
            self.atobCalled = True

        def btoc(oldState, newState):
            self.btocCalled = True

        def btoa(oldState, newState):
            self.btoaCalled = True

        self.atobCalled = False
        self.btocCalled = False
        self.btoaCalled = False
        machine = statemachine.StateMachine('A')
        machine.addState('A', {'B': atob, 'nostate': atob})
        machine.addState('B', {'C': btoc, 'A': btoa})
        machine.addState('C', {})
        self.assertException(lambda: machine.changeState('C'))
        self.assertException(lambda: machine.changeState('nostate'))
        machine.changeState('B')
        self.assert_(self.atobCalled)
        machine.changeState('A')
        self.assert_(self.btoaCalled)
        machine.changeState('B')
        machine.changeState('C')
        self.assert_(self.btocCalled)


    def __sendMouseEvent(self, type, x, y, sx=0, sy=0):
        Helper = Player.getTestHelper()
        Helper.fakeMouseEvent(type, True, False, False, x, y, 1, avg.Point2D(sx, sy))

    def __sendTouchEvent(self, id, type, x, y, sx=0, sy=0):
        Helper = Player.getTestHelper()
        Helper.fakeTouchEvent(id, type, avg.TOUCH, avg.Point2D(x, y), avg.Point2D(0, 0), 
                avg.Point2D(sx,sy))
       

def pythonTestSuite (tests):
    availableTests = (
        "testLinearAnim",
        "testLinearAnimZeroDuration",
        "testEaseInOutAnim",
        "testSplineAnim",
        "testContinuousAnim",
        "testWaitAnim",
        "testParallelAnim",
        "testStateAnim",
        "testDraggable",
        "testButton",
        "testMultitouchButton",
        "testKeyboard",
        "testTextArea",
        "testDragProcessor",
        "testDragProcessorInitialEvent",
        "testHoldProcessor",
        "testFocusContext",
        "testRoundedRect",
        "testPieSlice",
        "testArc",
        "testStateMachine",
        )
    
    return createAVGTestSuite(availableTests, PythonTestCase, tests)

Player = avg.Player.get()
anim.init(avg)
