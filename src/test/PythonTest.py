#!/usr/bin/python
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

import sys, platform

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

if platform.system() == 'Windows':
    from libavg import anim, draggable, button, textarea
else:
    import anim, draggable, button, textarea

from testcase import *

class PythonTestCase(AVGTestCase):
    def __init__(self, testFuncName):
        AVGTestCase.__init__(self, testFuncName, 24)
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
                 lambda: self.assert_(self.__anim.isDone),
                 lambda: self.compareImage(imgBaseName+"2", False),
                 lambda: self.assert_(Player.getElementByID("test").x == 100),
                 startAnim,
                 lambda: self.compareImage(imgBaseName+"1", False),
                 abortAnim,
                 lambda: self.assert_(anim.getNumRunningAnims() == 0),
                 lambda: self.compareImage(imgBaseName+"3", False),
                 lambda: self.assert_(self.__anim.isDone),
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
                 lambda: self.assert_(self.__anim.isDone)
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
                    [ anim.LinearAnim(node0, "x", 200, 0, 2),
                      anim.SplineAnim(node1, "x", 400, 0, 40, 0, 0),
                      anim.EaseInOutAnim(node2, "x", 300, 129, 99, 100, 100)
                    ], animStopped)
            self.anim.start()
        anim.init(avg)
        self.__endCalled = False
        Player.setFakeFPS(10)
        self.start("image.avg",
                (startAnim,
                 lambda: self.assert_(anim.getNumRunningAnims() == 3),
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
        def onClick(event):
            self.__clicked = True
        def createButton():
            buttonNode = Player.getElementByID("button") 
            self.button = button.Button(buttonNode, onClick)
            buttonNode.getChild(4).opacity = 0
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
        def disable():
            self.button.setDisabled(True)
            self.__clicked = False
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
                lambda: self.compareImage("testButtonUp", False),
                disable,
                lambda: self.compareImage("testButtonDisabled", False),
                down,
                lambda: self.compareImage("testButtonDisabled", False),
                upInside,
                lambda: self.assert_(not(self.__clicked)),
                lambda: self.compareImage("testButtonDisabled", False),
                out,
                lambda: self.button.setDisabled(False),
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

    def testTextArea(self):
        def setup():
            self.ta1 = textarea.TextArea(Player.getElementByID('ph1'), id='ta1')
            self.ta1.setStyle(font='Bitstream Vera Sans', variant='Roman',
                size=16, multiline=True, color='FFFFFF')
            self.ta1.setText('Lorem ipsum')
            self.ta1.setFocus(True) # TODO: REMOVE

            self.ta2 = textarea.TextArea(Player.getElementByID('ph2'), id='ta2')
            self.ta2.setStyle(font='Bitstream Vera Sans', variant='Roman',
                size=14, multiline=False, color='FFFFFF')
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

    def testFocusContext(self):
       def setup():
           textarea.init(avg)
           self.ctx1 = textarea.FocusContext()
           self.ctx2 = textarea.FocusContext()

           self.ta1 = textarea.TextArea(Player.getElementByID('ph1'),
               self.ctx1, id='ta1')
           self.ta1.setStyle(font='Bitstream Vera Sans', variant='Roman',
               size=16, multiline=True, color='FFFFFF')
           self.ta1.setText('Lorem ipsum')

           self.ta2 = textarea.TextArea(Player.getElementByID('ph2'),
               self.ctx1, id='ta2')
           self.ta2.setStyle(font='Bitstream Vera Sans', variant='Roman',
               size=14, multiline=False, color='FFFFFF')
           self.ta2.setText('dolor')

           self.ta3 = textarea.TextArea(Player.getElementByID('ph3'),
               self.ctx2, disableMouseFocus=True, id='ta3')
           self.ta3.setStyle(font='Bitstream Vera Sans', variant='Roman',
               size=14, multiline=True, color='FFFFFF')
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
           self.__sendEvent(avg.CURSORDOWN, 20, 70)
           self.__sendEvent(avg.CURSORUP, 20, 70)


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

    def __sendEvent(self, type, x, y):
        Helper = Player.getTestHelper()
        Helper.fakeMouseEvent(type, True, False, False, x, y, 1)


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
        "testCheckbox",
        "testTextArea",
        "testFocusContext",
        )
    return AVGTestSuite (availableTests, PythonTestCase, tests)

Player = avg.Player.get()
anim.init(avg)

if __name__ == '__main__':
    runStandaloneTest (pythonTestSuite)


