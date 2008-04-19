#!/usr/bin/python
# -*- coding: utf-8 -*-
import unittest

import sys, time, os, platform

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

SrcDir = os.getenv("srcdir",".")
os.chdir(SrcDir)
if platform.system() == 'Windows':
    from libavg import anim, draggable, button
else:    
    import anim, draggable, button

from testcase import *

class PythonTestCase(AVGTestCase):
    def __init__(self, testFuncName):
        AVGTestCase.__init__(self, testFuncName, 24)
    def testAnim(self):
        def onStart():
            Player.setTimeout(10, startAnim)
            Player.setTimeout(380, startSplineAnim)
            Player.setTimeout(800, lambda: self.compareImage("testAnim3", False))
            Player.setTimeout(850, Player.stop)
        def startAnim():
            def onStop():
                self.__animStopped = True
            self.compareImage("testAnim1", False)
            anim.fadeOut(Player.getElementByID("nestedimg2"), 200)
            Player.getElementByID("nestedimg1").opacity = 0
            anim.fadeIn(Player.getElementByID("nestedimg1"), 200, 1)
            anim.LinearAnim(Player.getElementByID("nestedimg1"), "x", 
                    200, 0, 100, 0, onStop)
        def startSplineAnim():
            self.assert_(self.__animStopped)
            self.compareImage("testAnim2", False)
            anim.SplineAnim(Player.getElementByID("mainimg"), "x", 
                    200, 100, -400, 10, 0, 0, None)
            anim.SplineAnim(Player.getElementByID("mainimg"), "y", 
                    200, 100, 0, 10, -400, 1, None)
        self.__animStopped = False
        Player.setFakeFPS(60)
        anim.init(avg)
        Player.loadFile("avg.avg")
        Player.setTimeout(1, onStart)
        Player.setFramerate(60)
        Player.play()

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
        def startAnim2():
            node=Player.getElementByID("nestedimg1")
            self.anim2=anim.ContinuousAnim(node,"width",0,50,0)
        def startAnim3():
            node=Player.getElementByID("nestedimg2")
            self.anim3=anim.ContinuousAnim(node,"x",0,50,0)
        def stopAnim():
            self.anim.abort()
            self.anim2.abort()
            self.anim3.abort()

        Player.setFakeFPS(25)
        anim.init(avg)
        Player.loadFile("avg.avg")
        Player.setTimeout(1, onStart)
        Player.play()

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
        dragger = draggable.Draggable(Player.getElementByID("testhue"),
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
        def onClick(event):
            self.__clicked = True
        def createButton():
            self.button = button.Button(Player.getElementByID("button"), onClick)
        def down():
            self.__sendEvent(avg.CURSORDOWN, 0, 0)
        def out():
            self.__sendEvent(avg.CURSORMOTION, 0, 50)
        def upOutside():
            self.__sendEvent(avg.CURSORUP, 0, 50)
        def over():
            self.__sendEvent(avg.CURSORMOTION, 0, 0)
        def upInside():
            self.__sendEvent(avg.CURSORUP, 0, 0)
        self.__clicked = False
        button.init(avg)
        self.start("ButtonTest.avg",
                (createButton,
                lambda: self.compareImage("testButtonUp", False),
                down,
                lambda: self.compareImage("testButtonDown", False),
                out,
                lambda: self.compareImage("testButtonUp", False),
                upOutside,
                lambda: self.assert_(not(self.__clicked)),
                down,
                lambda: self.compareImage("testButtonDown", False),
                out,
                lambda: self.compareImage("testButtonUp", False),
                over,
                lambda: self.compareImage("testButtonDown", False),
                upInside,
                lambda: self.assert_(self.__clicked),
                lambda: self.compareImage("testButtonOver", False),
                out,
                lambda: self.compareImage("testButtonUp", False)
               ))

    def testCheckbox(self):
        def createCheckbox():
            self.checkbox = button.Checkbox(Player.getElementByID("button"))
        def down():
            self.__sendEvent(avg.CURSORDOWN, 0, 0)
        def up():
            self.__sendEvent(avg.CURSORUP, 0, 0)
        def out():
            self.__sendEvent(avg.CURSORMOTION, 0, 50)
        button.init(avg)
        self.start("ButtonTest.avg",
                (createCheckbox,
                lambda: self.compareImage("testCheckboxUp", False),
                down,
                lambda: self.compareImage("testCheckboxDown", False),
                up,
                lambda: self.assert_(self.checkbox.getState() == True),
                lambda: self.compareImage("testCheckboxClickedOver", False),
                out,
                lambda: self.compareImage("testCheckboxClickedOut", False),
                down,
                lambda: self.compareImage("testCheckboxClickedDown", False),
                up,
                lambda: self.compareImage("testCheckboxOver", False)
               ))
    def __sendEvent(self, type, x, y):
        Helper = Player.getTestHelper()
        Helper.fakeMouseEvent(type, True, False, False, x, y, 1)


def pythonTestSuite():
    suite = unittest.TestSuite()
    suite.addTest(PythonTestCase("testAnim"))
    suite.addTest(PythonTestCase("testContinuousAnim"))
    suite.addTest(PythonTestCase("testDraggable"))
    suite.addTest(PythonTestCase("testButton"))
    suite.addTest(PythonTestCase("testCheckbox"))
    return suite
    
Log = avg.Logger.get()
Log.setCategories(Log.APP |
        Log.WARNING
#         Log.PROFILE |
#         Log.PROFILE_LATEFRAMES |
#         Log.CONFIG |
#         Log.MEMORY |
#         Log.BLTS    |
#         Log.EVENTS |
#         Log.EVENTS2
              )

if os.getenv("AVG_CONSOLE_TEST"):
    sys.exit(0)
else:
    Player = avg.Player()
    runner = unittest.TextTestRunner()
    rc = runner.run(pythonTestSuite())
    
    if rc.wasSuccessful():
        sys.exit(0)
    else:
        sys.exit(1)

