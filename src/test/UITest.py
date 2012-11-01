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

from libavg import avg, textarea, ui, player

from testcase import *

class UITestCase(AVGTestCase):
    def __init__(self, testFuncName):
        AVGTestCase.__init__(self, testFuncName)

    def testKeyboard(self):
        def setup():
            keyDefs = [
                    [("a", "A"),True, False, ( 5, 5), (30, 30)],
                    [(1, ),     True, False, (35, 5), (30, 30)],
                    ["SHIFT",   False, False, (65, 5), (50, 30)]]
            kbNoShift = ui.Keyboard("keyboard_bg.png", "keyboard_ovl.png", keyDefs, None,
                    pos=(10, 10), parent = root)
            kbNoShift.setKeyHandler(onKeyDown, onKeyUp)
            kbShift = ui.Keyboard("keyboard_bg.png", "keyboard_ovl.png", keyDefs, "SHIFT",
                    pos=(10, 60), selHref="keyboard_sel.png", parent = root)
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

        root = self.loadEmptyScene()

        self.__keyDown = False
        self.__keyUp   = True
        self.__char = "foo"
        self.__cmd = "bar"
        setup()
        self.start(False,
                (lambda: self.compareImage("testUIKeyboard"),
                 # test character key
                 lambda: self._sendMouseEvent(avg.Event.CURSOR_DOWN, 30, 30),
                 lambda: self.assert_(self.__keyDown and not self.__keyUp),
                 lambda: self.assert_(self.__char == "a" and self.__cmd is None),
                 lambda: self.compareImage("testUIKeyboardDownA1"),
                 lambda: self._sendMouseEvent(avg.Event.CURSOR_UP, 30, 30),
                 lambda: self.assert_(not self.__keyDown and self.__keyUp),
                 lambda: self.assert_(self.__char == "a" and self.__cmd is None),
                 lambda: self.compareImage("testUIKeyboard"),
                 # test command key
                 lambda: self._sendMouseEvent(avg.Event.CURSOR_DOWN, 100, 30),
                 lambda: self.assert_(self.__keyDown and not self.__keyUp),
                 lambda: self.assert_(self.__char is None and self.__cmd == "SHIFT"),
                 lambda: self.compareImage("testUIKeyboardDownS1"),
                 lambda: self._sendMouseEvent(avg.Event.CURSOR_UP, 100, 30),
                 lambda: self.assert_(not self.__keyDown and self.__keyUp),
                 lambda: self.assert_(self.__char is None and self.__cmd == "SHIFT"),
                 lambda: self.compareImage("testUIKeyboard"),
                 # test shift key (no shift key support)
                 lambda: self._sendTouchEvent(1, avg.Event.CURSOR_DOWN, 100, 30),
                 lambda: self._sendTouchEvent(2, avg.Event.CURSOR_DOWN, 30, 30),
                 lambda: self.assert_(self.__char == "a" and self.__cmd is None),
                 lambda: self._sendTouchEvent(3, avg.Event.CURSOR_DOWN, 60, 30),
                 lambda: self.assert_(self.__char == 1 and self.__cmd is None),
                 lambda: self.compareImage("testUIKeyboardDownA111S1"),
                 lambda: self._sendTouchEvent(2, avg.Event.CURSOR_UP, 30, 30),
                 lambda: self.assert_(self.__char == "a" and self.__cmd is None),
                 lambda: self._sendTouchEvent(3, avg.Event.CURSOR_UP, 60, 30),
                 lambda: self.assert_(self.__char == 1 and self.__cmd is None),
                 lambda: self._sendTouchEvent(1, avg.Event.CURSOR_UP, 100, 30),
                 lambda: self.compareImage("testUIKeyboard"),
                 # test shift key (with shift key support)
                 lambda: self._sendTouchEvent(1, avg.Event.CURSOR_DOWN, 100, 80),
                 lambda: self._sendTouchEvent(2, avg.Event.CURSOR_DOWN, 30, 80),
                 lambda: self.assert_(self.__char == "A" and self.__cmd is None),
                 lambda: self._sendTouchEvent(3, avg.Event.CURSOR_DOWN, 60, 80),
                 lambda: self.assert_(self.__char == 1 and self.__cmd is None),
                 lambda: self.compareImage("testUIKeyboardDownA212S2"),
                 lambda: self._sendTouchEvent(2, avg.Event.CURSOR_UP, 30, 80),
                 lambda: self.assert_(self.__char == "A" and self.__cmd is None),
                 lambda: self._sendTouchEvent(3, avg.Event.CURSOR_UP, 60, 80),
                 lambda: self.assert_(self.__char == 1 and self.__cmd is None),
                 lambda: self._sendTouchEvent(1, avg.Event.CURSOR_UP, 100, 80),
                 lambda: self.compareImage("testUIKeyboard"),
                 # test drag over keys 
                 lambda: self._sendTouchEvent(1, avg.Event.CURSOR_DOWN, 60, 80),
                 lambda: self.assert_(self.__char == 1 and self.__cmd is None),
                 lambda: self.compareImage("testUIKeyboardDown11"),
                 lambda: self._sendTouchEvent(1, avg.Event.CURSOR_MOTION, 60, 50),
                 lambda: self.assert_(self.__char == 1 and self.__cmd is None),
                 lambda: self.compareImage("testUIKeyboard"),
                 lambda: self._sendTouchEvent(1, avg.Event.CURSOR_MOTION, 100, 80),
                 lambda: self.assert_(self.__char is None and self.__cmd == "SHIFT"),
                 lambda: self.compareImage("testUIKeyboardDownA2S1"),
                 lambda: self._sendTouchEvent(1, avg.Event.CURSOR_MOTION, 60, 80),
                 lambda: self.compareImage("testUIKeyboardDown11"),
                 lambda: self._sendTouchEvent(1, avg.Event.CURSOR_UP, 60, 80),
                 lambda: self.assert_(not self.__keyDown and self.__keyUp),
                ))

    def testTextArea(self):
        def setup():
            self.ta1 = textarea.TextArea(pos=(2,2), size=(156, 96), parent=root)
            self.ta1.setStyle(fontsize=16, multiline=True, color='FFFFFF')
            self.ta1.setText('Lorem ipsum')
            self.ta1.setFocus(True) # TODO: REMOVE

            self.ta2 = textarea.TextArea(pos=(2,100), size=(156, 18), parent=root)
            self.ta2.setStyle(fontsize=14, multiline=False, color='4b94ef', 
                    cursorColor='FF0000', flashingCursor=False)
            self.ta2.setText('sit dolor')
            self.ta2.showCursor(False)
            self.ta2.setFocus(True) # TODO: REMOVE

        def setAndCheck(ta, text):
            ta.setText(text)
            self.assertEqual(ta.getText(), text)

        def clear(ta):
            ta.onKeyDown(textarea.KEYCODE_FORMFEED)
            self.assertEqual(ta.getText(), '')

        def testUnicode():
            self.ta1.setText(u'some ùnìcöde')
            self.ta1.onKeyDown(textarea.KEYCODES_BACKSPACE[0])
            self.assertEqual(self.ta1.getText(), u'some ùnìcöd')
            self.ta1.onKeyDown(textarea.KEYCODES_BACKSPACE[1])
            self.ta1.onKeyDown(textarea.KEYCODES_BACKSPACE[0])
            self.assertEqual(self.ta1.getText(), u'some ùnìc')
            self.ta1.onKeyDown(ord(u'Ä'))
            self.assertEqual(self.ta1.getText(), u'some ùnìcÄ')

        def testSpecialChars():
            clear(self.ta1)
            self.ta1.onKeyDown(ord(u'&'))
            self.ta1.onKeyDown(textarea.KEYCODES_BACKSPACE[0])
            self.assertEqual(self.ta1.getText(), '')

        def checkSingleLine():
            text = ''
            self.ta2.setText('')
            while True:
                self.assert_(len(text) < 20)
                self.ta2.onKeyDown(ord(u'A'))
                text = text + 'A'
                if text != self.ta2.getText():
                    break

        root = self.loadEmptyScene()

        player.setFakeFPS(20)
        textarea.init(avg, False)
        self.start(True,
                (setup,
                 lambda: self.delay(200),
                 lambda: self.assertEqual(self.ta1.getText(), 'Lorem ipsum'),
                 lambda: setAndCheck(self.ta1, ''),
                 lambda: setAndCheck(self.ta2, 'Lorem Ipsum'),
                 testUnicode,
                 lambda: self.compareImage("testTextArea1"),
                 testSpecialChars,
                 checkSingleLine,
                 lambda: self.compareImage("testTextArea2"),
                 lambda: self.ta2.showCursor(True),
                 lambda: self.delay(200),
                 lambda: self._sendTouchEvent(1, avg.Event.CURSOR_DOWN, 30, 100),
                 lambda: self._sendTouchEvent(1, avg.Event.CURSOR_UP, 30, 100),
                 lambda: self.compareImage("testTextArea3"),
                 lambda: self._sendTouchEvent(2, avg.Event.CURSOR_DOWN, 130, 100),
                 lambda: self.delay(1100),
                 lambda: self.compareImage("testTextArea4"),
                 lambda: self._sendTouchEvent(2, avg.Event.CURSOR_MOTION, 30, 100),
                 lambda: self.compareImage("testTextArea5"),
                 lambda: self._sendTouchEvent(2, avg.Event.CURSOR_UP, 30, 100),
                 lambda: self.compareImage("testTextArea3"),
                ))
        player.setFakeFPS(-1)

    def testFocusContext(self):
        def setup():
            textarea.init(avg)
            self.ctx1 = textarea.FocusContext()
            self.ctx2 = textarea.FocusContext()

            self.ta1 = textarea.TextArea(self.ctx1, pos=(2,2), size=(156,54), parent=root)
            self.ta1.setStyle(fontsize=16, multiline=True, color='FFFFFF')
            self.ta1.setText('Lorem ipsum')

            self.ta2 = textarea.TextArea(self.ctx1, pos=(2,58), size=(76,54), parent=root)
            self.ta2.setStyle(fontsize=14, multiline=False, color='FFFFFF')
            self.ta2.setText('dolor')

            self.bgImage = avg.ImageNode(href="1x1_white.png", size=(76,54))
            self.ta3 = textarea.TextArea(self.ctx2, disableMouseFocus=True, pos=(80,58),
                size=(76,54), textBackgroundNode=self.bgImage, parent=root)
            self.ta3.setStyle(fontsize=14, multiline=True, color='FFFFFF')
            self.ta3.setText('dolor sit amet')

            textarea.setActiveFocusContext(self.ctx1)

        def writeChar():
            helper = player.getTestHelper()
            helper.fakeKeyEvent(avg.Event.KEY_DOWN, 65, 65, "A", 65, 0)
            helper.fakeKeyEvent(avg.Event.KEY_UP, 65, 65, "A", 65, 0)
            helper.fakeKeyEvent(avg.Event.KEY_DOWN, 66, 66, "B", 66, 0)
            helper.fakeKeyEvent(avg.Event.KEY_UP, 66, 66, "B", 66, 0)
            helper.fakeKeyEvent(avg.Event.KEY_DOWN, 67, 67, "C", 67, 0)
            helper.fakeKeyEvent(avg.Event.KEY_UP, 67, 67, "C", 67, 0)

        def switchFocus():
            self.ctx1.cycleFocus()

        def clearFocused():
            self.ctx1.clear()

        def clickForFocus():
            self._sendMouseEvent(avg.Event.CURSOR_DOWN, 20, 70)
            self._sendMouseEvent(avg.Event.CURSOR_UP, 20, 70)

        root = self.loadEmptyScene()
        self.start(True,
                (setup,
                 lambda: self.compareImage("testFocusContext1"),
                 writeChar,
                 lambda: self.compareImage("testFocusContext2"),
                 switchFocus,
                 writeChar,
                 lambda: self.compareImage("testFocusContext3"),
                 switchFocus,
                 clearFocused,
                 lambda: self.compareImage("testFocusContext4"),
                 clickForFocus,
                 clearFocused,
                 lambda: self.compareImage("testFocusContext5"),
               ))


    def testButton(self):

        def enable(enabled):
            button.enabled = enabled

        def createScene(**kwargs):
            root = self.loadEmptyScene()
            button = ui.Button(
                    parent = root,
                    upNode = avg.ImageNode(href="button_up.png"),
                    downNode = avg.ImageNode(href="button_down.png"),
                    disabledNode = avg.ImageNode(href="button_disabled.png"),
                    **kwargs
                    )
            self.messageTester = MessageTester(button, 
                    [ui.Button.CLICKED, ui.Button.PRESSED, ui.Button.RELEASED],
                    self)
            return button

        def runTest():
            self.start(False,
                    (# Standard down->up
                     self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 0, 0,
                            [ui.Button.PRESSED]),
                     lambda: self.compareImage("testUIButtonDown"),
                     self._genMouseEventFrames(avg.Event.CURSOR_UP, 0, 0,
                            [ui.Button.CLICKED, ui.Button.RELEASED]),
                     lambda: self.compareImage("testUIButtonUp"),

                     # Disable, down, up -> no click
                     lambda: self.assert_(button.enabled),
                     lambda: enable(False),
                     lambda: self.assert_(not(button.enabled)),
                     lambda: self.compareImage("testUIButtonDisabled"),
                     lambda: self._sendMouseEvent(avg.Event.CURSOR_DOWN, 0, 0),
                     self._genMouseEventFrames(avg.Event.CURSOR_UP, 0, 0, []),
                     lambda: enable(True),
                     lambda: self.assert_(button.enabled),

                     # Down, up further away -> no click
                     lambda: self._sendMouseEvent(avg.Event.CURSOR_DOWN, 0, 0),
                     self._genMouseEventFrames(avg.Event.CURSOR_UP, 100, 0,
                            [ui.Button.PRESSED, ui.Button.RELEASED]),
                     lambda: self.compareImage("testUIButtonUp"),

                     # Down, move further away, up -> no click
                     lambda: self._sendMouseEvent(avg.Event.CURSOR_DOWN, 0, 0),
                     lambda: self._sendMouseEvent(avg.Event.CURSOR_MOTION, 100, 0),
                     self._genMouseEventFrames(avg.Event.CURSOR_UP, 100, 0,
                            [ui.Button.PRESSED, ui.Button.RELEASED]),
                     lambda: self.compareImage("testUIButtonUp"),

                     # Test if button still reacts after abort
                     self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 0, 0,
                            [ui.Button.PRESSED]),
                     lambda: self.compareImage("testUIButtonDown"),
                     self._genMouseEventFrames(avg.Event.CURSOR_UP, 0, 0,
                            [ui.Button.CLICKED, ui.Button.RELEASED]),
                     lambda: self.compareImage("testUIButtonUp"),
                    ))

        button = createScene()
        runTest()

        button = createScene(activeAreaNode=avg.CircleNode(r=5, opacity=0))
        runTest()

        button = createScene(fatFingerEnlarge=True)
        runTest()

        root = self.loadEmptyScene()
        button = ui.BmpButton(
                parent = root,
                upSrc = "button_up.png",
                downSrc = "button_down.png",
                disabledSrc = "button_disabled.png",
                )
        self.messageTester = MessageTester(button, 
                [ui.Button.CLICKED, ui.Button.PRESSED, ui.Button.RELEASED],
                self)
        runTest()
       
        button = createScene(enabled=False)
        self.start(False,
                (lambda: self.compareImage("testUIButtonDisabled"),
                ))

    def testTextButton(self):
        root = self.loadEmptyScene()
        button = ui.TextButton("text", parent=root, size=(50,42))
        self.messageTester = MessageTester(button, 
                [ui.Button.CLICKED, ui.Button.PRESSED, ui.Button.RELEASED],
                self)
        self.start(True,
                (# Standard down->up
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 0, 0,
                        [ui.Button.PRESSED]),
                 lambda: self.compareImage("testTextButtonDown"),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 0, 0,
                        [ui.Button.CLICKED, ui.Button.RELEASED]),
                 lambda: self.compareImage("testTextButtonUp"),

                 # Check disabled graphics
                 lambda: self.assert_(button.enabled),
                 lambda: button.setEnabled(False),
                 lambda: self.assert_(not(button.enabled)),
                 lambda: self.compareImage("testTextButtonDisabled"),
                 lambda: button.setEnabled(True),

                 # Change text
                 lambda: button.setText("newText"),
                 lambda: self.compareImage("testTextButtonUpNewText"),
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 0, 0,
                        [ui.Button.PRESSED]),
                 lambda: self.compareImage("testTextButtonDownNewText"),
                ))


    def testToggleButton(self):

        def onToggled(isToggled):
            self.messageTester.setMessageReceived(ui.ToggleButton.TOGGLED)
            self.toggled = isToggled
        
        def reset():
            self.messageTester.reset()
            self.toggled = False

        def createScene(**kwargs):
            root = self.loadEmptyScene()
            button = ui.ToggleButton(
                    uncheckedUpNode = avg.ImageNode(href="toggle_unchecked_Up.png"),
                    uncheckedDownNode = avg.ImageNode(href="toggle_unchecked_Down.png"),
                    checkedUpNode = avg.ImageNode(href="toggle_checked_Up.png"),
                    checkedDownNode = avg.ImageNode(href="toggle_checked_Down.png"),
                    uncheckedDisabledNode =
                            avg.ImageNode(href="toggle_unchecked_Disabled.png"),
                    checkedDisabledNode =
                            avg.ImageNode(href="toggle_checked_Disabled.png"),
                    parent=root,
                    **kwargs
                   )
            self.messageTester = MessageTester(button, 
                    [ui.ToggleButton.PRESSED, ui.ToggleButton.RELEASED], self)

            button.subscribe(ui.ToggleButton.TOGGLED, onToggled)
            return button

        def testToggle():
            self.start(False,
                    (reset,
                     lambda: self.compareImage("testUIToggleUnchecked_Up"),
                     self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 0, 0,
                             [ui.ToggleButton.PRESSED]),
                     lambda: self.assert_(not self.toggled),
                     lambda: self.compareImage("testUIToggleUnchecked_Down"),
                     self._genMouseEventFrames(avg.Event.CURSOR_UP, 0, 0,
                            [ui.ToggleButton.RELEASED, ui.ToggleButton.TOGGLED]),
                     lambda: self.assert_(self.toggled),
                     lambda: self.compareImage("testUIToggleChecked_Up"),
                     lambda: self._sendMouseEvent(avg.Event.CURSOR_DOWN, 0, 0),
                     lambda: self.compareImage("testUIToggleChecked_Down"),
                     lambda: self._sendMouseEvent(avg.Event.CURSOR_UP, 0, 0),
                     lambda: self.assert_(not(self.toggled)),
                     lambda: self.compareImage("testUIToggleUnchecked_Up"),
                    ))

        def testToggleAbort():
            self.start(False,
                    (reset,
                     lambda: self.compareImage("testUIToggleUnchecked_Up"),
                     lambda: self._sendMouseEvent(avg.Event.CURSOR_DOWN, 0, 0),
                     lambda: self.compareImage("testUIToggleUnchecked_Down"),
                     self._genMouseEventFrames(avg.Event.CURSOR_UP, 100, 0,
                             [ui.ToggleButton.PRESSED, ui.ToggleButton.RELEASED]),
                     lambda: self.assert_(not self.toggled),
                     lambda: self.compareImage("testUIToggleUnchecked_Up"),
                     lambda: button.setChecked(True),
                     lambda: self.compareImage("testUIToggleChecked_Up"),
                     lambda: self._sendMouseEvent(avg.Event.CURSOR_DOWN, 0, 0),
                     lambda: self.compareImage("testUIToggleChecked_Down"),
                     lambda: self._sendMouseEvent(avg.Event.CURSOR_UP, 100, 0),
                     lambda: self.assert_(not(self.toggled)), 
                     lambda: self.compareImage("testUIToggleChecked_Up"),
                    ))

        def testToggleDisable():
            self.start(False,
                    (reset,
                     lambda: self.compareImage("testUIToggleUnchecked_Disabled"),
                     lambda: self._sendTouchEvent(1, avg.Event.CURSOR_DOWN, 0, 0),
                     lambda: self._sendTouchEvent(1, avg.Event.CURSOR_UP, 0, 0),
                     lambda: self.compareImage("testUIToggleUnchecked_Disabled"),
                     lambda: button.setEnabled(True),
                     lambda: self.compareImage("testUIToggleUnchecked_Up"),
                     lambda: self._sendTouchEvent(2, avg.Event.CURSOR_DOWN, 0, 0),
                     lambda: button.setEnabled(False),
                     lambda: self._sendTouchEvent(2, avg.Event.CURSOR_UP, 0, 0),
                     lambda: self.assert_(not(self.toggled)),
                     lambda: self.compareImage("testUIToggleUnchecked_Disabled"),
                     
                     lambda: button.setEnabled(True),
                     reset,
                     lambda: self.compareImage("testUIToggleUnchecked_Up"),
                     lambda: button.setChecked(True),
                     lambda: self.compareImage("testUIToggleChecked_Up"),
                     lambda: button.setEnabled(False),
                     lambda: self.compareImage("testUIToggleChecked_Disabled"),
                     lambda: self._sendTouchEvent(3, avg.Event.CURSOR_DOWN, 0, 0),
                     lambda: self._sendTouchEvent(3, avg.Event.CURSOR_UP, 0, 0),
                     lambda: self.compareImage("testUIToggleChecked_Disabled"),
                     lambda: button.setEnabled(True),
                     lambda: self.compareImage("testUIToggleChecked_Up"),
                     lambda: self._sendTouchEvent(4, avg.Event.CURSOR_DOWN, 0, 0),
                     lambda: button.setEnabled(False),
                     lambda: self._sendTouchEvent(4, avg.Event.CURSOR_UP, 0, 0),
                     lambda: self.assert_(not(self.toggled)),
                     lambda: self.compareImage("testUIToggleChecked_Disabled"),
                    ))
       
        def testFromSrc():
            root = self.loadEmptyScene()
            button = ui.BmpToggleButton(
                    uncheckedUpSrc="toggle_unchecked_Up.png",
                    uncheckedDownSrc="toggle_unchecked_Down.png",
                    checkedUpSrc="toggle_checked_Up.png",
                    checkedDownSrc="toggle_checked_Down.png",
                    uncheckedDisabledSrc="toggle_unchecked_Disabled.png",
                    checkedDisabledSrc="toggle_checked_Disabled.png",
                    parent=root)
            button.subscribe(ui.ToggleButton.TOGGLED, onToggled)
            self.start(False,
                    (lambda: self.compareImage("testUIToggleUnchecked_Up"),
                     lambda: button.setChecked(True),
                     lambda: self.compareImage("testUIToggleChecked_Up"),
                     lambda: button.setChecked(False),
                     lambda: button.setEnabled(False),
                     lambda: self.compareImage("testUIToggleUnchecked_Disabled"),
                     lambda: button.setChecked(True),
                     lambda: self.compareImage("testUIToggleChecked_Disabled"),
                    ))
 
        button = createScene()
        testToggle()
        
        button = createScene()
        testToggleAbort()
        
        button = createScene(enabled = False)
        testToggleDisable()

        button = createScene(activeAreaNode = avg.CircleNode(r=5, opacity=0))
        testToggle()
        
        button = createScene(fatFingerEnlarge = True)
        testToggle()

        testFromSrc()

    def testScrollPane(self):
        def scrollLarge():
            scrollPane.contentpos = (34, 34)
            self.assertEqual(scrollPane.contentpos, (32,32))

        def initSmallContent():
            scrollPane.size = (64, 64)
            contentArea.size = (32, 32)
            image.size = (32, 32)
            scrollPane.contentpos = (0, 0)
            self.assertEqual(scrollPane.getMaxContentPos(), (0,0))

        def scrollSmall():
            scrollPane.contentpos = (32, 32)

        root = self.loadEmptyScene()
        contentArea = avg.DivNode(size=(64,64))
        image = avg.ImageNode(href="rgb24-64x64.png", parent=contentArea)
        scrollPane = ui.ScrollPane(contentNode=contentArea, size=(32,32), parent=root)

        self.start(False,
                (lambda: self.compareImage("testScrollPane1"),
                 scrollLarge,
                 lambda: self.compareImage("testScrollPane2"),
                 initSmallContent,
                 lambda: self.compareImage("testScrollPane3"),
                 scrollSmall,
                 lambda: self.compareImage("testScrollPane3"),
                ))

    def testStretchNode(self):
            
        def changeExtent():
            if orientation == ui.Orientation.HORIZONTAL:
                self.node.width = 100
            else:
                self.node.height = 100

        def minExtent():
            if orientation == ui.Orientation.HORIZONTAL:
                self.node.width = 3
                self.assert_(self.node.width == 31)
            else:
                self.node.height = 3
                self.assert_(self.node.height == 31)

        for orientation, orName in (
                (ui.Orientation.HORIZONTAL,"Horiz"),
                (ui.Orientation.VERTICAL, "Vert")):
            root = self.loadEmptyScene()
            if orientation == ui.Orientation.HORIZONTAL:
                self.node = ui.HStretchNode(src="media/rgb24-32x32.png", endsExtent=15, 
                        size=(31,31), parent=root)
            else:
                self.node = ui.VStretchNode(src="media/rgb24-32x32.png", endsExtent=15, 
                        size=(31,31), parent=root)
            self.start(False,
                    (lambda: self.compareImage("testStretchNode"+orName+"1"),
                     changeExtent,
                     lambda: self.compareImage("testStretchNode"+orName+"2"),
                     minExtent,
                     lambda: self.compareImage("testStretchNode"+orName+"1"),
                    ))


    def testHVStretchNode(self):

        def changeSize():
            self.node.size = (64, 64)
            self.assert_(self.node.size == (64,64))

        root = self.loadEmptyScene()
        self.node = ui.HVStretchNode(src="media/rgb24-32x32.png", endsExtent=(5,5), 
                size=(31,31), parent=root)
        self.start(False,
                (lambda: self.compareImage("testHVStretchNode1"),
                 changeSize,
                 lambda: self.compareImage("testHVStretchNode2"),
                )) 


    def testSlider(self):
        def onThumbPosChanged(pos):
            self.thumbpos = pos

        def setSize():
            if orientation == ui.Orientation.HORIZONTAL:
                self.node.width=140
            else:
                self.node.height=60

        for orientation, orName in (
                (ui.Orientation.HORIZONTAL, "Horiz"),
                (ui.Orientation.VERTICAL, "Vert")):
            root = self.loadEmptyScene()
            self.node = ui.Slider(orientation=orientation, pos=(20,20), 
                    width=100, height=100, parent=root)
            self.start(False,
                    (lambda: self.compareImage("testSlider"+orName+"1"),
                     lambda: self.node.setThumbPos(0.25),
                     lambda: self.compareImage("testSlider"+orName+"2"),
                     lambda: self.node.setThumbPos(1),
                     lambda: self.compareImage("testSlider"+orName+"3"),
                     lambda: self.node.setRange((1,10)),
                     lambda: self.compareImage("testSlider"+orName+"1"),
                     lambda: self.node.setRange((10,1)),
                     lambda: self.compareImage("testSlider"+orName+"3"),
                     setSize,
                     lambda: self.compareImage("testSlider"+orName+"4"),
                     lambda: self.node.setEnabled(False),
                     lambda: self.compareImage("testSlider"+orName+"5"),
                     lambda: self.node.setEnabled(True),
                     lambda: self.compareImage("testSlider"+orName+"4"),
                    ))


    def testScrollBar(self):
        
        def onThumbPosChanged(pos):
            self.thumbpos = pos

        for orientation, orName in (
                (ui.Orientation.HORIZONTAL, "Horiz"),
                (ui.Orientation.VERTICAL, "Vert")):
            root = self.loadEmptyScene()
            self.node = ui.ScrollBar(orientation=orientation, pos=(20,20), 
                    width=100, height=100, parent=root)
            self.start(False,
                    (lambda: self.compareImage("testScrollBar"+orName+"1"),
                     lambda: self.node.setThumbExtent(0.5),
                     lambda: self.compareImage("testScrollBar"+orName+"2"),
                     lambda: self.node.setThumbPos(0.25),
                     lambda: self.compareImage("testScrollBar"+orName+"3"),
                     lambda: self.node.setThumbPos(5),
                     lambda: self.compareImage("testScrollBar"+orName+"4"),
                     lambda: self.node.setRange((0,10)),
                     lambda: self.node.setThumbPos(4.75),
                     lambda: self.compareImage("testScrollBar"+orName+"5"),
                     lambda: self.node.setRange((10,20)),
                     lambda: self.node.setThumbPos(14.75),
                     lambda: self.compareImage("testScrollBar"+orName+"5"),
                     lambda: self.node.setThumbExtent(10),
                     lambda: self.compareImage("testScrollBar"+orName+"6"),
                     lambda: self.node.setRange((10,0)),
                     lambda: self.node.setThumbExtent(0.5),
                     lambda: self.node.setThumbPos(4.75),
                     lambda: self.compareImage("testScrollBar"+orName+"5"),
                    ))

        # Horizontal
        root = self.loadEmptyScene()
        self.node = ui.ScrollBar(orientation=ui.Orientation.HORIZONTAL, pos=(20,5), 
                width=100, parent=root)
        self.messageTester = MessageTester(self.node, 
                [ui.Slider.PRESSED, ui.Slider.RELEASED], self)
        self.start(False,
                (lambda: self.node.setThumbExtent(0.5),
                 # User input
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 25, 10,
                        [ui.Slider.PRESSED]),
                 lambda: self.compareImage("testScrollBarHoriz7"),
                 lambda: self._sendMouseEvent(avg.Event.CURSOR_MOTION, 50, 10),
                 lambda: self.compareImage("testScrollBarHoriz8"),
                 lambda: self.assertAlmostEqual(self.node.getThumbPos(), 0.25),
                 lambda: self._sendMouseEvent(avg.Event.CURSOR_MOTION, 25, 10),
                 lambda: self.compareImage("testScrollBarHoriz9"),
                 lambda: self.assertAlmostEqual(self.node.getThumbPos(), 0),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 0, 10,
                        [ui.Slider.RELEASED]),
                 lambda: self.compareImage("testScrollBarHoriz10"),
                 lambda: self.assertAlmostEqual(self.node.getThumbPos(), 0),

                 # Publish/Subscribe interface
                 lambda: self.node.subscribe(ui.ScrollBar.THUMB_POS_CHANGED, 
                        onThumbPosChanged),
                 lambda: self._sendMouseEvent(avg.Event.CURSOR_DOWN, 25, 10),
                 lambda: self._sendMouseEvent(avg.Event.CURSOR_UP, 50, 10),
                 lambda: self.assertAlmostEqual(self.thumbpos, 0.25),

                 # Enable/disable
                 self.messageTester.reset,
                 lambda: self.node.setEnabled(False),
                 lambda: self.compareImage("testScrollBarHoriz11"),
                 lambda: self._sendMouseEvent(avg.Event.CURSOR_DOWN, 50, 10),
                 lambda: self._sendMouseEvent(avg.Event.CURSOR_UP, 25, 10),
                 lambda: self.messageTester.assertState([]),
                 lambda: self.assertAlmostEqual(self.thumbpos, 0.25),
                 lambda: self.node.setEnabled(True),
                 lambda: self.compareImage("testScrollBarHoriz12"),

                 # Disable after down: Drag aborted
                 lambda: self._sendMouseEvent(avg.Event.CURSOR_DOWN, 50, 10),
                 lambda: self.node.setEnabled(False),
                 lambda: self._sendMouseEvent(avg.Event.CURSOR_UP, 25, 10),
                 lambda: self.assertAlmostEqual(self.thumbpos, 0.25),
                 lambda: self.node.setEnabled(True),
                 lambda: self.compareImage("testScrollBarHoriz12"),
                ))

        # Vertical: Don't need to test everything again, just make sure coords are 
        # calculated correctly.
        root = self.loadEmptyScene()
        self.node = ui.ScrollBar(orientation=ui.Orientation.VERTICAL, pos=(5,5), 
                height=100, parent=root)
        self.start(False,
                (lambda: self.node.setThumbExtent(0.5),
                 lambda: self._sendMouseEvent(avg.Event.CURSOR_DOWN, 10, 25),
                 lambda: self.compareImage("testScrollBarVert7"),
                 lambda: self._sendMouseEvent(avg.Event.CURSOR_MOTION, 10, 50),
                 lambda: self.compareImage("testScrollBarVert8"),
                 lambda: self.assertAlmostEqual(self.node.getThumbPos(), 0.25),
                 lambda: self._sendMouseEvent(avg.Event.CURSOR_UP, 10, 0),
                 lambda: self.compareImage("testScrollBarVert9"),
                 lambda: self.assertAlmostEqual(self.node.getThumbPos(), 0),
                ))

    def testScrollArea(self):
        def setSize(size):
            self.node.size = size

        root = self.loadEmptyScene()
        image = avg.ImageNode(href="rgb24-64x64.png", size=(200,400))
        self.node = ui.ScrollArea(contentNode=image, size=(80,80), friction=-1, 
                parent=root)
        self.start(False,
                (lambda: self.compareImage("testScrollArea1"),
                 lambda: setSize((120,80)),
                 lambda: self.compareImage("testScrollArea2"),
                ))

#    def testSimpleScrollArea(self):
#
#        def onContentPosChanged(newPos):
#            self.messageTester.setMessageReceived(self.node.CONTENT_POS_CHANGED)
#
#        root = self.loadEmptyScene()
#        image = avg.ImageNode(href="rgb24-64x64.png", size=(200,400))
#        self.node = simple.ScrollArea(contentNode=image, size=(115,115),
#                friction=-1, parent=root)
#        self.messageTester = MessageTester(self.node, [self.node.PRESSED, 
#                self.node.RELEASED], self)
#        self.node.subscribe(self.node.CONTENT_POS_CHANGED, onContentPosChanged)
#        player.setFakeFPS(10)
#
#        self.start(False,
#                (lambda: self.compareImage("testSimpleScrollArea1"),
#                 lambda: self.node.setContentSize((400,200)),
#                 lambda: self.compareImage("testSimpleScrollArea2"),
#                 lambda: self.node.setContentPos((200,100)),
#                 lambda: self.messageTester.assertState([self.node.CONTENT_POS_CHANGED]),
#                 lambda: self.compareImage("testSimpleScrollArea3"),
#                 lambda: self.node.setContentPos((0,0)),
#                 # Scroll via gesture
#                 self.messageTester.reset,
#                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 90, 90,
#                        [self.node.PRESSED,]),
#                 self._genMouseEventFrames(avg.Event.CURSOR_MOTION, 10, 90,
#                        [self.node.CONTENT_POS_CHANGED]),
#                 lambda: self.compareImage("testSimpleScrollArea4"),
#                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 10, 10,
#                        [self.node.RELEASED,self.node.CONTENT_POS_CHANGED]),
#                 lambda: self.compareImage("testSimpleScrollArea5"),
#                 lambda: self.delay(1000), # Wait for end of inertia.
#                 lambda: self.node.setContentPos((0,0)), 
#                 # Scroll using scroll bars
#                 self.messageTester.reset,
#                 lambda: self.compareImage("testSimpleScrollArea2"),
#                 lambda: self._sendMouseEvent(avg.Event.CURSOR_DOWN, 110, 0),
#                 lambda: self._sendMouseEvent(avg.Event.CURSOR_UP, 110, 50),
#                 lambda: self.messageTester.assertState([self.node.CONTENT_POS_CHANGED]),
#                 lambda: self.compareImage("testSimpleScrollArea6"),
#                 lambda: self._sendMouseEvent(avg.Event.CURSOR_DOWN, 0, 110),
#                 lambda: self._sendMouseEvent(avg.Event.CURSOR_UP, 50, 110),
#                 lambda: self.messageTester.assertState([self.node.CONTENT_POS_CHANGED]),
#                 lambda: self.compareImage("testSimpleScrollArea7"),
#                ))
#        player.setFakeFPS(-1)
#
#    def testSimpleCheckBox(self):
#        def onToggled(isChecked):
#            self.assert_(self.expectedChecked == isChecked)
#            self.assert_(checkBox.checked == isChecked)
#            self.messageTester.setMessageReceived(simple.CheckBox.TOGGLED)
#
#        def setExpectedChecked(isChecked):
#            self.expectedChecked = isChecked
#
#        root = self.loadEmptyScene()
#        checkBox = simple.CheckBox(text="text", parent=root)
#        self.messageTester = MessageTester(checkBox, [checkBox.PRESSED, 
#                checkBox.RELEASED], self)
#        checkBox.subscribe(checkBox.TOGGLED, onToggled)
#        setExpectedChecked(False)
#        self.start(False,
#                (self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 10, 10,
#                        [checkBox.PRESSED,]),
#                 lambda: setExpectedChecked(True),
#                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 10, 10,
#                        [checkBox.RELEASED, checkBox.TOGGLED,]),
#                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 10, 10,
#                        [checkBox.PRESSED,]),
#                 lambda: setExpectedChecked(False),
#                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 10, 10,
#                        [checkBox.RELEASED, checkBox.TOGGLED,]),
#                 
#                 # Disabled node: No events.
#                 lambda: checkBox.setEnabled(False),
#                 lambda: self.assert_(not(checkBox.enabled)),
#                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 10, 10, []),
#                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 10, 10, []),
#
#                 lambda: checkBox.setEnabled(True),
#                 lambda: self.assert_(checkBox.enabled),
#                ))

def uiTestSuite(tests):
    availableTests = (
        "testKeyboard",
        "testTextArea",
        "testFocusContext",
        "testButton",
        "testTextButton",
        "testToggleButton",
        "testScrollPane",
        "testStretchNode",
        "testHVStretchNode",
        "testSlider",
        "testScrollBar",
        "testScrollArea",
#        "testSimpleScrollBar",
#        "testSimpleSlider",
#        "testSimpleScrollArea",
#        "testSimpleCheckBox"
        )

    return createAVGTestSuite(availableTests, UITestCase, tests)
