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
    from libavg import anim, draggable, button, textarea
else:
    import anim, draggable, button, textarea

from testcase import *

class PythonTestCase(AVGTestCase):
    def __init__(self, testFuncName):
        AVGTestCase.__init__(self, testFuncName, 24)
    def testAnimType(self, createAnimFunc, imgBaseName):
        def onStop():
            self.__onStopCalled = True
        def startAnim():
            self.__onStopCalled = False
            node = Player.getElementByID("test")
            self.__runningAnim = self.__createAnimFunc(node, onStop)
        def abortAnim():
            self.__runningAnim.abort()
        self.__createAnimFunc = createAnimFunc
        self.__onStopCalled = False
        anim.init(avg)
        Player.setFakeFPS(10)
        self.start("image.avg",
                (startAnim,
                 lambda: self.compareImage(imgBaseName+"1", False),
                 lambda: self.assert_(anim.getNumRunningAnims() == 1),
                 None,
                 None,
                 lambda: self.assert_(self.__onStopCalled),
                 lambda: self.assert_(self.__runningAnim.isDone),
                 lambda: self.compareImage(imgBaseName+"2", False),
                 lambda: self.assert_(Player.getElementByID("test").x == 100),
                 startAnim,
                 lambda: self.compareImage(imgBaseName+"1", False),
                 abortAnim,
                 lambda: self.assert_(anim.getNumRunningAnims() == 0),
                 lambda: self.compareImage(imgBaseName+"3", False),
                 lambda: self.assert_(self.__runningAnim.isDone),
                 None,
                 lambda: self.assert_(not(self.__onStopCalled)),
                 startAnim,
                 startAnim,
                 lambda: self.assert_(anim.getNumRunningAnims() == 1)
                ))

    def testLinearAnim(self):
        def createAnim(node, onStop):
            return anim.LinearAnim(node, "x", 200, 0, 100, False, onStop)
        self.testAnimType(createAnim, "testLinearAnim")

    def testEaseInOutAnim(self):
        def createAnim(node, onStop):
            return anim.EaseInOutAnim(node, "x", 400, 0, 100, 100, 100,
                    False, onStop)
        self.testAnimType(createAnim, "testEaseInOutAnim")

    def testSplineAnim(self):
        def createAnim(node, onStop):
            return anim.SplineAnim(node, "x", 300, 0, 0, 100, 0,
                    False, onStop)
        self.testAnimType(createAnim, "testSplineAnim")

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

    def testTextarea(self):
        def createTextareaSet():
            self.ctx1 = textarea.FocusContext()
            self.ctx2 = textarea.FocusContext()
            
            self.ta1 = textarea.Textarea(Player.getElementByID('placeholder'), self.ctx1)
            self.ta1.setStyle(font='Arial', size=44, multiline=True)
            self.ta1.setText("Lorem ipsum")

            self.ta2 = textarea.Textarea(Player.getElementByID('placeholder_2'), self.ctx1)
            self.ta2.setStyle(font='Verdana', size=12, multiline=False)

            self.ta3 = textarea.Textarea(Player.getElementByID('placeholder_3'), self.ctx2, '1x1_white.png', True)
            self.ta3.setStyle(font='Eurostile', size=18, multiline=True)
            
            textarea.setActiveFocusContext(self.ctx1)
        def setAndCheck(ta, text):
            ta.setText(text)
            self.assert_(ta.getText() == text)
        def checkSingleLine():
            text = ''
            self.ta2.setText('')
            while True:
                print text, self.ta2.getText()
                self.assert_(len(text) < 60)
                self.ctx1.keyCharPressed('X')
                text = text + 'X'
                if text != self.ta2.getText():
                    break
        def clearPage():
            self.ctx1.keyCodePressed(12)
            self.assert_(self.ta2.getText() == '')
        def longText():
            textarea.setActiveFocusContext(self.ctx2)
            text = '''
Lorem ipsum dolor sit amet, consectetuer adipiscing elit. Donec massa nunc, pretium sed,
sagittis mollis, dignissim vitae, erat. Vestibulum mattis, erat nec pulvinar lacinia,
velit turpis molestie nulla, non dictum nisl nibh ac magna.
Vestibulum ac dui non sapien luctus imperdiet.
Morbi eros enim, laoreet non, tincidunt eu, varius eget, magna. Nulla facilisi.
Aenean id nulla.
Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia Curae;
Nam sodales. Cum sociis natoque penatibus et magnis dis parturient montes,
nascetur ridiculus mus. Curabitur auctor sollicitudin tortor. 
            '''
            self.ta3.setText(text)
            self.assert_(self.ta3.getText() == text)
        def clickFocus():
            textarea.setActiveFocusContext(self.ctx1)
            self.ctx1.cycleFocus()
            self.__sendEvent(avg.CURSORDOWN, 20, 20)
            self.__sendEvent(avg.CURSORUP, 20, 20)
        def testClickFocus():
            self.ctx1.keyCodePressed(12)
            self.ctx1.keyCharPressed('X')
            self.assert_(self.ta1.getText() == 'X')
        
        textarea.init(avg, False)
        self.start("TextareaTest.avg",
               (createTextareaSet,
               lambda: self.assert_(self.ta1.getText() != 'Lorem Ipsum'),
               lambda: setAndCheck(self.ta1, ''),
               lambda: setAndCheck(self.ta2, 'Lorem Ipsum'),
               checkSingleLine,
               clearPage,
               longText,
               clickFocus,
               testClickFocus
               ))

    def __sendEvent(self, type, x, y):
        Helper = Player.getTestHelper()
        Helper.fakeMouseEvent(type, True, False, False, x, y, 1)


def pythonTestSuite():
    suite = unittest.TestSuite()
    suite.addTest(PythonTestCase("testLinearAnim"))
    suite.addTest(PythonTestCase("testEaseInOutAnim"))
    suite.addTest(PythonTestCase("testSplineAnim"))
    suite.addTest(PythonTestCase("testContinuousAnim"))
    suite.addTest(PythonTestCase("testDraggable"))
    suite.addTest(PythonTestCase("testButton"))
    suite.addTest(PythonTestCase("testCheckbox"))
    suite.addTest(PythonTestCase("testTextarea"))
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

