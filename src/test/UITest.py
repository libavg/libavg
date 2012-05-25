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
        self.start(False,
                (setup,
                 lambda: self.compareImage("testUIKeyboard"),
                 # test character key
                 lambda: self._sendMouseEvent(avg.CURSORDOWN, 30, 30),
                 lambda: self.assert_(self.__keyDown and not self.__keyUp),
                 lambda: self.assert_(self.__char == "a" and self.__cmd is None),
                 lambda: self.compareImage("testUIKeyboardDownA1"),
                 lambda: self._sendMouseEvent(avg.CURSORUP, 30, 30),
                 lambda: self.assert_(not self.__keyDown and self.__keyUp),
                 lambda: self.assert_(self.__char == "a" and self.__cmd is None),
                 lambda: self.compareImage("testUIKeyboard"),
                 # test command key
                 lambda: self._sendMouseEvent(avg.CURSORDOWN, 100, 30),
                 lambda: self.assert_(self.__keyDown and not self.__keyUp),
                 lambda: self.assert_(self.__char is None and self.__cmd == "SHIFT"),
                 lambda: self.compareImage("testUIKeyboardDownS1"),
                 lambda: self._sendMouseEvent(avg.CURSORUP, 100, 30),
                 lambda: self.assert_(not self.__keyDown and self.__keyUp),
                 lambda: self.assert_(self.__char is None and self.__cmd == "SHIFT"),
                 lambda: self.compareImage("testUIKeyboard"),
                 # test shift key (no shift key support)
                 lambda: self._sendTouchEvent(1, avg.CURSORDOWN, 100, 30),
                 lambda: self._sendTouchEvent(2, avg.CURSORDOWN, 30, 30),
                 lambda: self.assert_(self.__char == "a" and self.__cmd is None),
                 lambda: self._sendTouchEvent(3, avg.CURSORDOWN, 60, 30),
                 lambda: self.assert_(self.__char == 1 and self.__cmd is None),
                 lambda: self.compareImage("testUIKeyboardDownA111S1"),
                 lambda: self._sendTouchEvent(2, avg.CURSORUP, 30, 30),
                 lambda: self.assert_(self.__char == "a" and self.__cmd is None),
                 lambda: self._sendTouchEvent(3, avg.CURSORUP, 60, 30),
                 lambda: self.assert_(self.__char == 1 and self.__cmd is None),
                 lambda: self._sendTouchEvent(1, avg.CURSORUP, 100, 30),
                 lambda: self.compareImage("testUIKeyboard"),
                 # test shift key (with shift key support)
                 lambda: self._sendTouchEvent(1, avg.CURSORDOWN, 100, 80),
                 lambda: self._sendTouchEvent(2, avg.CURSORDOWN, 30, 80),
                 lambda: self.assert_(self.__char == "A" and self.__cmd is None),
                 lambda: self._sendTouchEvent(3, avg.CURSORDOWN, 60, 80),
                 lambda: self.assert_(self.__char == 1 and self.__cmd is None),
                 lambda: self.compareImage("testUIKeyboardDownA212S2"),
                 lambda: self._sendTouchEvent(2, avg.CURSORUP, 30, 80),
                 lambda: self.assert_(self.__char == "A" and self.__cmd is None),
                 lambda: self._sendTouchEvent(3, avg.CURSORUP, 60, 80),
                 lambda: self.assert_(self.__char == 1 and self.__cmd is None),
                 lambda: self._sendTouchEvent(1, avg.CURSORUP, 100, 80),
                 lambda: self.compareImage("testUIKeyboard"),
                 # test drag over keys 
                 lambda: self._sendTouchEvent(1, avg.CURSORDOWN, 60, 80),
                 lambda: self.assert_(self.__char == 1 and self.__cmd is None),
                 lambda: self.compareImage("testUIKeyboardDown11"),
                 lambda: self._sendTouchEvent(1, avg.CURSORMOTION, 60, 50),
                 lambda: self.assert_(self.__char == 1 and self.__cmd is None),
                 lambda: self.compareImage("testUIKeyboard"),
                 lambda: self._sendTouchEvent(1, avg.CURSORMOTION, 100, 80),
                 lambda: self.assert_(self.__char is None and self.__cmd == "SHIFT"),
                 lambda: self.compareImage("testUIKeyboardDownA2S1"),
                 lambda: self._sendTouchEvent(1, avg.CURSORMOTION, 60, 80),
                 lambda: self.compareImage("testUIKeyboardDown11"),
                 lambda: self._sendTouchEvent(1, avg.CURSORUP, 60, 80),
                 lambda: self.assert_(not self.__keyDown and self.__keyUp),
                ))

    def testTextArea(self):
        def setup():
            self.ta1 = textarea.TextArea( pos=(2,2), size=(156, 96), parent=root)
            self.ta1.setStyle(font='Bitstream Vera Sans', variant='Roman',
                fontsize=16, multiline=True, color='FFFFFF')
            self.ta1.setText('Lorem ipsum')
            self.ta1.setFocus(True) # TODO: REMOVE

            self.ta2 = textarea.TextArea(pos=(2,100), size=(156, 18), parent=root)
            self.ta2.setStyle(font='Bitstream Vera Sans', variant='Roman',
                fontsize=14, multiline=False, color='4b94ef', cursorColor='FF0000', flashingCursor=False)
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
                    self.assertEqual(len(text), 16)
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
                 lambda: self._sendTouchEvent(1, avg.CURSORDOWN, 30, 100),
                 lambda: self._sendTouchEvent(1, avg.CURSORUP, 30, 100),
                 lambda: self.compareImage("testTextArea3"),
                 lambda: self._sendTouchEvent(2, avg.CURSORDOWN, 130, 100),
                 lambda: self.delay(1100),
                 lambda: self.compareImage("testTextArea4"),
                 lambda: self._sendTouchEvent(2, avg.CURSORMOTION, 30, 100),
                 lambda: self.compareImage("testTextArea5"),
                 lambda: self._sendTouchEvent(2, avg.CURSORUP, 30, 100),
                 lambda: self.compareImage("testTextArea3"),
                ))
        player.setFakeFPS(-1)

    def testFocusContext(self):
        def setup():
            textarea.init(avg)
            self.ctx1 = textarea.FocusContext()
            self.ctx2 = textarea.FocusContext()

            self.ta1 = textarea.TextArea(self.ctx1, pos=(2,2), size=(156,54), parent=root)
            self.ta1.setStyle(font='Bitstream Vera Sans', variant='Roman',
                fontsize=16, multiline=True, color='FFFFFF')
            self.ta1.setText('Lorem ipsum')

            self.ta2 = textarea.TextArea(self.ctx1, pos=(2,58), size=(76,54), parent=root)
            self.ta2.setStyle(font='Bitstream Vera Sans', variant='Roman',
                fontsize=14, multiline=False, color='FFFFFF')
            self.ta2.setText('dolor')

            self.bgImage = avg.ImageNode(href="1x1_white.png", size=(76,54))
            self.ta3 = textarea.TextArea(self.ctx2, disableMouseFocus=True, pos=(80,58),
                size=(76,54), textBackgroundNode=self.bgImage, parent=root)
            self.ta3.setStyle(font='Bitstream Vera Sans', variant='Roman',
                fontsize=14, multiline=True, color='FFFFFF')
            self.ta3.setText('dolor sit amet')

            textarea.setActiveFocusContext(self.ctx1)

        def writeChar():
            helper = player.getTestHelper()
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
            self._sendMouseEvent(avg.CURSORDOWN, 20, 70)
            self._sendMouseEvent(avg.CURSORUP, 20, 70)

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
        def onDown(event):
            self.__down = True

        def onClick(event):
            self.__clicked = True

        def reset():
            self.__down = False
            self.__clicked = False

        def setObjectActive(obj, active):
            obj.active = active

        root = self.loadEmptyScene()

        b = ui.Button(
                parent = root,
                upNode = avg.ImageNode(href="button_up.png"),
                downNode = avg.ImageNode(href="button_down.png"),
                disabledNode = avg.ImageNode(href="button_disabled.png"),
                pressHandler = onDown,
                clickHandler = onClick)

        b1 = ui.Button(parent=root,
                       active=False,
                       pressHandler=onDown,
                       clickHandler=onClick)

        b.pos = (0, 0)
        yOutDistance = int(b.height * 2)

        self.__down = False
        self.__clicked = False

        self.start(False,
                (# Normal click: Down & up over button
                 lambda: self.compareImage("testUIButtonUp"),
                 lambda: self._sendMouseEvent(avg.CURSORDOWN, 0, 0),
                 lambda: self.compareImage("testUIButtonDown"),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 lambda: self._sendMouseEvent(avg.CURSORUP, 0, 0),
                 lambda: self.compareImage("testUIButtonUp"),
                 lambda: self.assert_(self.__clicked),
                 reset,

                 # Down, move away from button, move over button, up
                 # ==> click
                 lambda: self._sendMouseEvent(avg.CURSORDOWN, 0, 0),
                 lambda: self._sendMouseEvent(avg.CURSORMOTION, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonUp"),
                 lambda: self._sendMouseEvent(avg.CURSORMOTION, 0, 0),
                 lambda: self.compareImage("testUIButtonDown"),
                 lambda: self._sendMouseEvent(avg.CURSORUP, 0, 0),
                 lambda: self.assert_(self.__down and self.__clicked),
                 reset,

                 # Down, move away from button, up ==> no click
                 lambda: self._sendMouseEvent(avg.CURSORDOWN, 0, 0),
                 lambda: self.compareImage("testUIButtonDown"),
                 lambda: self._sendMouseEvent(avg.CURSORMOTION, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonUp"),
                 lambda: self._sendMouseEvent(avg.CURSORUP, 0, yOutDistance),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 reset,

                 # Move away from button, down, mover over button, up
                 # ==> no click
                 lambda: self._sendMouseEvent(avg.CURSORDOWN, 0, yOutDistance),
                 lambda: self._sendMouseEvent(avg.CURSORMOTION, 0, 0),
                 lambda: self.compareImage("testUIButtonUp"),
                 lambda: self._sendMouseEvent(avg.CURSORUP, 0, 0),
                 lambda: self.compareImage("testUIButtonUp"),
                 lambda: self.assert_(not(self.__down) and not(self.__clicked)),
                 reset,

                 # Move away from button, down, mover over button, move away from button, up
                 # ==> no click
                 lambda: self._sendMouseEvent(avg.CURSORDOWN, 0, yOutDistance),
                 lambda: self._sendMouseEvent(avg.CURSORMOTION, 0, 0),
                 lambda: self.compareImage("testUIButtonUp"),
                 lambda: self._sendMouseEvent(avg.CURSORMOTION, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonUp"),
                 lambda: self._sendMouseEvent(avg.CURSORUP, 0, yOutDistance),
                 lambda: self.assert_(not(self.__down) and not(self.__clicked)),
                 reset,

                 # Check checkable button
                 # Set checkable, down, up => pressed, down, up ==> click
                 lambda: b.setCheckable(True),
                 lambda: self.compareImage("testUIButtonUp"),
                 lambda: self._sendMouseEvent(avg.CURSORDOWN, 0, 0),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 lambda: self.compareImage("testUIButtonDown"),
                 lambda: self._sendMouseEvent(avg.CURSORUP, 0, 0),
                 lambda: self.compareImage("testUIButtonDown"),
                 lambda: self.assert_(self.__down and self.__clicked),
                 reset,
                 lambda: self._sendMouseEvent(avg.CURSORDOWN, 0, 0),
                 lambda: self.compareImage("testUIButtonDown"),
                 lambda: self._sendMouseEvent(avg.CURSORUP, 0, 0),
                 lambda: self.compareImage("testUIButtonUp"),
                 lambda: self.assert_(self.__down and self.__clicked),
                 reset,

                 # not checked, down, out, in, up ==> pressed, click
                 lambda: b.setCheckable(True),
                 lambda: b.setChecked(False),
                 lambda: self.compareImage("testUIButtonUp"),
                 lambda: self._sendMouseEvent(avg.CURSORDOWN, 0, 0),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 lambda: self.compareImage("testUIButtonDown"),
                 lambda: self._sendMouseEvent(avg.CURSORMOTION, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonUp"),
                 lambda: self._sendMouseEvent(avg.CURSORMOTION, 0, 0),
                 lambda: self.compareImage("testUIButtonDown"),
                 lambda: self._sendMouseEvent(avg.CURSORUP, 0, 0),
                 lambda: self.compareImage("testUIButtonDown"),
                 lambda: self.assert_(self.__down and self.__clicked),
                 reset,

                 # not checked, down, out, up ==> pressed, no click
                 lambda: b.setCheckable(True),
                 lambda: b.setChecked(False),
                 lambda: self.compareImage("testUIButtonUp"),
                 lambda: self._sendMouseEvent(avg.CURSORDOWN, 0, 0),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 lambda: self.compareImage("testUIButtonDown"),
                 lambda: self._sendMouseEvent(avg.CURSORMOTION, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonUp"),
                 lambda: self._sendMouseEvent(avg.CURSORUP, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonUp"),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 reset,

                 # not checked, down, out, up ==> pressed, no click
                 lambda: b.setCheckable(True),
                 lambda: b.setChecked(True),
                 lambda: self.compareImage("testUIButtonDown"),
                 lambda: self._sendMouseEvent(avg.CURSORDOWN, 0, 0),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 lambda: self.compareImage("testUIButtonDown"),
                 lambda: self._sendMouseEvent(avg.CURSORMOTION, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonDown"),
                 lambda: self._sendMouseEvent(avg.CURSORUP, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonDown"),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 reset,

                 # checked, down, out, in, up ==> pressed, clicked
                 lambda: b.setCheckable(True),
                 lambda: b.setChecked(True),
                 lambda: self.compareImage("testUIButtonDown"),
                 lambda: self._sendMouseEvent(avg.CURSORDOWN, 0, 0),
                 lambda: self.assert_(self.__down),
                 lambda: self.compareImage("testUIButtonDown"),
                 lambda: self._sendMouseEvent(avg.CURSORMOTION, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonDown"),
                 lambda: self._sendMouseEvent(avg.CURSORMOTION, 0, 0),
                 lambda: self.compareImage("testUIButtonDown"),
                 lambda: self._sendMouseEvent(avg.CURSORUP, 0, 0),
                 lambda: self.compareImage("testUIButtonUp"),
                 lambda: self.assert_(self.__down and self.__clicked),
                 reset,

                 # Test public interface
                 lambda: b.setCheckable(False),
                 lambda: self.compareImage("testUIButtonUp"),
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
                 lambda: self._sendMouseEvent(avg.CURSORDOWN, 0, 0),
                 lambda: self._sendMouseEvent(avg.CURSORUP, 0, 0),
                 lambda: self.compareImage("testUIButtonDisabled"),
                 lambda: self.assert_(not(self.__down) and not(self.__clicked)),
                 lambda: self._sendMouseEvent(avg.CURSORDOWN, 0, 0),
                 lambda: self.compareImage("testUIButtonDisabled"),
                 lambda: self._sendMouseEvent(avg.CURSORUP, 0, 0),
                 lambda: self.compareImage("testUIButtonDisabled"),
                 lambda: self.assert_(not(self.__down) and not(self.__clicked)),
                 lambda: self._sendMouseEvent(avg.CURSORDOWN, 0, 0),
                 lambda: self.compareImage("testUIButtonDisabled"),
                 lambda: b.setEnabled(True),
                 lambda: self._sendMouseEvent(avg.CURSORUP, 0, 0),
                 lambda: self.compareImage("testUIButtonUp"),
                 lambda: self.assert_(not(self.__down)),
                 lambda: self.assert_(not(self.__clicked)),
                 reset,

                 # Checking functionality while resetting the visible nodes
                 lambda: b.setNodes(upNode = avg.ImageNode(href="button_up.png"),
                                    downNode = avg.ImageNode(href="button_down.png"),
                                    disabledNode = avg.ImageNode(href="button_disabled.png")),
                 lambda: self.compareImage("testUIButtonUp"),
                 lambda: self._sendMouseEvent(avg.CURSORDOWN, 0, 0),
                 lambda: self._sendMouseEvent(avg.CURSORUP, 0, 0),
                 lambda: self.assert_(self.__down and self.__clicked),
                 reset,

                 lambda: b.setEnabled(False),
                 lambda: b.setNodes(upNode = avg.ImageNode(href="button_up.png"),
                                    downNode = avg.ImageNode(href="button_down.png"),
                                    disabledNode = avg.ImageNode(href="button_disabled.png")),
                 lambda: self.compareImage("testUIButtonDisabled"),
                 lambda: self._sendMouseEvent(avg.CURSORDOWN, 0, 0),
                 lambda: self._sendMouseEvent(avg.CURSORUP, 0, 0),
                 lambda: self.assert_(not(self.__down) and not(self.__clicked)),
                 reset,

                 lambda: b.setEnabled(True),
                 lambda: b.setNodes(upNode = avg.ImageNode(href="button_up.png"),
                                    downNode = avg.ImageNode(href="button_down.png"),
                                    disabledNode = None),
                 lambda: self.compareImage("testUIButtonUp"),
                 lambda: self._sendMouseEvent(avg.CURSORDOWN, 0, 0),
                 lambda: self._sendMouseEvent(avg.CURSORUP, 0, 0),
                 lambda: self.assert_(self.__down and self.__clicked),
                 reset,

                 # resetting the nodes on an empty button
                 lambda: setObjectActive(b, False),
                 lambda: setObjectActive(b1, True),
                 lambda: b1.setNodes(upNode = avg.ImageNode(href="button_up.png"),
                                     downNode = avg.ImageNode(href="button_down.png"),
                                     disabledNode = avg.ImageNode(href="button_disabled.png")),
                 lambda: self.compareImage("testUIButtonUp"),
                 lambda: self._sendMouseEvent(avg.CURSORDOWN, 0, 0),
                 lambda: self._sendMouseEvent(avg.CURSORUP, 0, 0),
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

        root = self.loadEmptyScene()
        b = ui.Button(
                parent = root,
                upNode = avg.ImageNode(href="button_up.png"),
                downNode = avg.ImageNode(href="button_down.png"),
                disabledNode = avg.ImageNode(href="button_disabled.png"),
                pressHandler = onDown,
                clickHandler = onClick,
                )
        b.pos = (0,0)
        yOutDistance = b.height * 2

        self.__down = False
        self.__clicked = False
        self.start(False,
                (# Two downs, two ups ==> click after second up.
                 lambda: self._sendTouchEvent(1, avg.CURSORDOWN, 0, 0),
                 lambda: self._sendTouchEvent(2, avg.CURSORDOWN, 0, 0),
                 lambda: self.compareImage("testUIButtonDown"),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 lambda: self._sendTouchEvent(1, avg.CURSORUP, 0, 0),
                 lambda: self.compareImage("testUIButtonDown"),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 lambda: self._sendTouchEvent(2, avg.CURSORUP, 0, 0),
                 lambda: self.compareImage("testUIButtonUp"),
                 lambda: self.assert_(self.__clicked),
                 reset,

                 # Two downs, one out, out up, in up ==> click
                 lambda: self._sendTouchEvent(1, avg.CURSORDOWN, 0, 0),
                 lambda: self._sendTouchEvent(2, avg.CURSORDOWN, 0, 0),
                 lambda: self.compareImage("testUIButtonDown"),
                 lambda: self._sendTouchEvent(2, avg.CURSORMOTION, 0, yOutDistance),
                 lambda: self._sendTouchEvent(2, avg.CURSORUP, 0, 50),
                 lambda: self.compareImage("testUIButtonDown"),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 lambda: self._sendTouchEvent(1, avg.CURSORUP, 0, 0),
                 lambda: self.compareImage("testUIButtonUp"),
                 lambda: self.assert_(self.__down and self.__clicked),
                 reset,

                 # Two down, both out, both in, both up ==> click
                 lambda: self._sendTouchEvent(1, avg.CURSORDOWN, 0, 0),
                 lambda: self._sendTouchEvent(2, avg.CURSORDOWN, 0, 0),
                 lambda: self.compareImage("testUIButtonDown"),
                 lambda: self._sendTouchEvent(1, avg.CURSORMOTION, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonDown"),
                 lambda: self._sendTouchEvent(2, avg.CURSORMOTION, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonUp"),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 lambda: self._sendTouchEvent(1, avg.CURSORMOTION, 0, 0),
                 lambda: self.compareImage("testUIButtonDown"),
                 lambda: self._sendTouchEvent(2, avg.CURSORMOTION, 0, 0),
                 lambda: self.compareImage("testUIButtonDown"),
                 lambda: self._sendTouchEvent(1, avg.CURSORUP, 0, 0),
                 lambda: self._sendTouchEvent(2, avg.CURSORUP, 0, 0),
                 lambda: self.compareImage("testUIButtonUp"),
                 lambda: self.assert_(self.__down and self.__clicked),
                 reset,

                 # Two down both out, both up ==> no click
                 lambda: self._sendTouchEvent(1, avg.CURSORDOWN, 0, 0),
                 lambda: self._sendTouchEvent(2, avg.CURSORDOWN, 0, 0),
                 lambda: self.compareImage("testUIButtonDown"),
                 lambda: self._sendTouchEvent(1, avg.CURSORMOTION, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonDown"),
                 lambda: self._sendTouchEvent(2, avg.CURSORMOTION, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonUp"),
                 lambda: self._sendTouchEvent(1, avg.CURSORUP, 0, yOutDistance),
                 lambda: self._sendTouchEvent(2, avg.CURSORUP, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonUp"),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 reset,

                 # Two downs, one out, in up, out up ==> no click
                 lambda: self._sendTouchEvent(1, avg.CURSORDOWN, 0, 0),
                 lambda: self._sendTouchEvent(2, avg.CURSORDOWN, 0, 0),
                 lambda: self.compareImage("testUIButtonDown"),
                 lambda: self._sendTouchEvent(2, avg.CURSORMOTION, 0, yOutDistance),
                 lambda: self._sendTouchEvent(1, avg.CURSORUP, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonUp"),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 lambda: self._sendTouchEvent(2, avg.CURSORUP, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonUp"),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 reset,

                 # Check checkable multitouch button
                 # 2 down, 2 up ==> pressed, clicked, 
                 # 2 down, 2 up ==> pressed, clicked
                 lambda: b.setCheckable(True),
                 lambda: self.compareImage("testUIButtonUp"),
                 lambda: self._sendTouchEvent(1, avg.CURSORDOWN, 0, 0),
                 lambda: self._sendTouchEvent(2, avg.CURSORDOWN, 0, 0),
                 lambda: self.compareImage("testUIButtonDown"),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 lambda: self._sendTouchEvent(1, avg.CURSORUP, 0, 0),
                 lambda: self.compareImage("testUIButtonDown"),
                 lambda: self._sendTouchEvent(2, avg.CURSORUP, 0, 0),
                 lambda: self.compareImage("testUIButtonDown"),
                 lambda: self.assert_(self.__down and self.__clicked),
                 reset,
                 lambda: self.compareImage("testUIButtonDown"),
                 lambda: self._sendTouchEvent(1, avg.CURSORDOWN, 0, 0),
                 lambda: self._sendTouchEvent(2, avg.CURSORDOWN, 0, 0),
                 lambda: self.compareImage("testUIButtonDown"),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 lambda: self._sendTouchEvent(1, avg.CURSORUP, 0, 0),
                 lambda: self._sendTouchEvent(2, avg.CURSORUP, 0, 0),
                 lambda: self.compareImage("testUIButtonUp"),
                 lambda: self.assert_(self.__down and self.__clicked),
                 reset,

                 # not checked, 2 down, 2 out, 2 in, first up, second up 
                 # ==> pressed, clicked
                 lambda: b.setChecked(False),
                 lambda: self.compareImage("testUIButtonUp"),
                 lambda: self._sendTouchEvent(1, avg.CURSORDOWN, 0, 0),
                 lambda: self._sendTouchEvent(2, avg.CURSORDOWN, 0, 0),
                 lambda: self.compareImage("testUIButtonDown"),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 lambda: self._sendTouchEvent(1, avg.CURSORMOTION, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonDown"),
                 lambda: self._sendTouchEvent(2, avg.CURSORMOTION, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonUp"),
                 lambda: self._sendTouchEvent(1, avg.CURSORMOTION, 0, 0),
                 lambda: self.compareImage("testUIButtonDown"),
                 lambda: self._sendTouchEvent(2, avg.CURSORMOTION, 0, 0),
                 lambda: self.compareImage("testUIButtonDown"),
                 lambda: self._sendTouchEvent(1, avg.CURSORUP, 0, 0),
                 lambda: self.compareImage("testUIButtonDown"),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 lambda: self._sendTouchEvent(2, avg.CURSORUP, 0, 0),
                 lambda: self.compareImage("testUIButtonDown"),
                 lambda: self.assert_(self.__down and self.__clicked),
                 reset,

                 # checked, 2 down, 2 out, 2 in, first up, second up 
                 # ==> pressed, clicked
                 lambda: b.setCheckable(True),
                 lambda: b.setChecked(True),
                 lambda: self.compareImage("testUIButtonDown"),
                 lambda: self._sendTouchEvent(1, avg.CURSORDOWN, 0, 0),
                 lambda: self._sendTouchEvent(2, avg.CURSORDOWN, 0, 0),
                 lambda: self.compareImage("testUIButtonDown"),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 lambda: self._sendTouchEvent(1, avg.CURSORMOTION, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonDown"),
                 lambda: self._sendTouchEvent(2, avg.CURSORMOTION, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonDown"),
                 lambda: self._sendTouchEvent(1, avg.CURSORMOTION, 0, 0),
                 lambda: self.compareImage("testUIButtonDown"),
                 lambda: self._sendTouchEvent(2, avg.CURSORMOTION, 0, 0),
                 lambda: self.compareImage("testUIButtonDown"),
                 lambda: self._sendTouchEvent(1, avg.CURSORUP, 0, 0),
                 lambda: self.compareImage("testUIButtonDown"),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 lambda: self._sendTouchEvent(2, avg.CURSORUP, 0, 0),
                 lambda: self.compareImage("testUIButtonUp"),
                 lambda: self.assert_(self.__down and self.__clicked),
                 reset,

                 # not checked, 2 down, 2 out, first up, second up 
                 # ==> pressed, not clicked
                 lambda: b.setCheckable(True),
                 lambda: b.setChecked(False),
                 lambda: self.compareImage("testUIButtonUp"),
                 lambda: self._sendTouchEvent(1, avg.CURSORDOWN, 0, 0),
                 lambda: self._sendTouchEvent(2, avg.CURSORDOWN, 0, 0),
                 lambda: self.compareImage("testUIButtonDown"),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 lambda: self._sendTouchEvent(1, avg.CURSORMOTION, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonDown"),
                 lambda: self._sendTouchEvent(2, avg.CURSORMOTION, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonUp"),
                 lambda: self._sendTouchEvent(1, avg.CURSORUP, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonUp"),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 lambda: self._sendTouchEvent(2, avg.CURSORUP, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonUp"),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 reset,

                 # checked, 2 down, 2 out, first up, second up 
                 # ==> pressed, not clicked
                 lambda: b.setCheckable(True),
                 lambda: b.setChecked(True),
                 lambda: self.compareImage("testUIButtonDown"),
                 lambda: self._sendTouchEvent(1, avg.CURSORDOWN, 0, 0),
                 lambda: self._sendTouchEvent(2, avg.CURSORDOWN, 0, 0),
                 lambda: self.compareImage("testUIButtonDown"),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 lambda: self._sendTouchEvent(1, avg.CURSORMOTION, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonDown"),
                 lambda: self._sendTouchEvent(2, avg.CURSORMOTION, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonDown"),
                 lambda: self._sendTouchEvent(1, avg.CURSORUP, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonDown"),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 lambda: self._sendTouchEvent(2, avg.CURSORUP, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonDown"),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 reset,
                ))


    def testTouchButton(self):

        def onClick(event):
            self.clicked = True

        def reset():
            self.clicked = False

        def enable(enabled):
            button.enabled = enabled

        def createScene(**kwargs):
            root = self.loadEmptyScene()
            return ui.TouchButton(
                    parent = root,
                    upNode = avg.ImageNode(href="button_up.png"),
                    downNode = avg.ImageNode(href="button_down.png"),
                    disabledNode = avg.ImageNode(href="button_disabled.png"),
                    clickHandler = onClick,
                    **kwargs
                    )

        def runTest():
            self.clicked = False
            self.start(False,
                    (# Standard down->up
                     lambda: self._sendTouchEvent(1, avg.CURSORDOWN, 0, 0),
                     lambda: self.assert_(not(self.clicked)),
                     lambda: self.compareImage("testUIButtonDown"),
                     lambda: self._sendTouchEvent(1, avg.CURSORUP, 0, 0),
                     lambda: self.assert_(self.clicked),
                     lambda: self.compareImage("testUIButtonUp"),

                     # Disable, down, up -> no click
                     reset,
                     lambda: self.assert_(button.enabled),
                     lambda: enable(False),
                     lambda: self.assert_(not(button.enabled)),
                     lambda: self.compareImage("testUIButtonDisabled"),
                     lambda: self._sendTouchEvent(2, avg.CURSORDOWN, 0, 0),
                     lambda: self._sendTouchEvent(2, avg.CURSORUP, 0, 0),
                     lambda: self.assert_(not(self.clicked)),
                     lambda: enable(True),
                     lambda: self.assert_(button.enabled),

                     # Down, up further away -> no click
                     reset,
                     lambda: self._sendTouchEvent(3, avg.CURSORDOWN, 0, 0),
                     lambda: self._sendTouchEvent(3, avg.CURSORUP, 100, 0),
                     lambda: self.assert_(not(self.clicked)),
                     lambda: self.compareImage("testUIButtonUp"),

                     # Down, move further away, up -> no click
                     reset,
                     lambda: self._sendTouchEvent(3, avg.CURSORDOWN, 0, 0),
                     lambda: self._sendTouchEvent(3, avg.CURSORMOTION, 100, 0),
                     lambda: self._sendTouchEvent(3, avg.CURSORUP, 100, 0),
                     lambda: self.assert_(not(self.clicked)),
                     lambda: self.compareImage("testUIButtonUp"),

                     # Test if button still reacts after abort
                     lambda: self._sendTouchEvent(4, avg.CURSORDOWN, 0, 0),
                     lambda: self.assert_(not(self.clicked)),
                     lambda: self.compareImage("testUIButtonDown"),
                     lambda: self._sendTouchEvent(4, avg.CURSORUP, 0, 0),
                     lambda: self.assert_(self.clicked),
                     lambda: self.compareImage("testUIButtonUp"),
                    ))

        button = createScene()
        runTest()

        button = createScene(activeAreaNode = avg.CircleNode(r=5, opacity=0))
        runTest()

        button = createScene(fatFingerEnlarge = True)
        runTest()

        root = self.loadEmptyScene()
        button = ui.TouchButton.fromSrc(
                parent = root,
                upSrc = "button_up.png",
                downSrc = "button_down.png",
                disabledSrc = "button_disabled.png",
                clickHandler = onClick
                )
        runTest()
        
    def testToggleButton(self):

        def onCheck(event):
            self.checked = True
        
        def onUncheck(event):
            self.unchecked = True

        def reset():
            self.checked = False
            self.unchecked = False

        def createScene(**kwargs):
            root = self.loadEmptyScene()
            return ui.ToggleButton(
                    uncheckedUpNode = avg.ImageNode(href="toggle_unchecked_Up.png"),
                    uncheckedDownNode = avg.ImageNode(href="toggle_unchecked_Down.png"),
                    checkedUpNode = avg.ImageNode(href="toggle_checked_Up.png"),
                    checkedDownNode = avg.ImageNode(href="toggle_checked_Down.png"),
                    uncheckedDisabledNode =
                            avg.ImageNode(href="toggle_unchecked_Disabled.png"),
                    checkedDisabledNode =
                            avg.ImageNode(href="toggle_checked_Disabled.png"),
                    checkHandler = onCheck, uncheckHandler = onUncheck, parent = root,
                    **kwargs
                    )

        def testToggle():
            self.start(False,
                    (reset,
                     lambda: self.compareImage("testUIToggleUnchecked_Up"),
                     lambda: self._sendTouchEvent(1, avg.CURSORDOWN, 0, 0),
                     lambda: self.assert_(not(self.checked)),
                     lambda: self.assert_(not(self.unchecked)),
                     lambda: self.compareImage("testUIToggleUnchecked_Down"),
                     lambda: self._sendTouchEvent(1, avg.CURSORUP, 0, 0),
                     lambda: self.assert_(self.checked),
                     lambda: self.compareImage("testUIToggleChecked_Up"),
                     lambda: self._sendTouchEvent(2, avg.CURSORDOWN, 0, 0),
                     lambda: self.compareImage("testUIToggleChecked_Down"),
                     lambda: self._sendTouchEvent(2, avg.CURSORUP, 0, 0),
                     lambda: self.assert_(self.unchecked),
                     lambda: self.compareImage("testUIToggleUnchecked_Up"),
                    ))

        def testToggleAbort():
            self.start(False,
                    (reset,
                     lambda: self.compareImage("testUIToggleUnchecked_Up"),
                     lambda: self._sendTouchEvent(1, avg.CURSORDOWN, 0, 0),
                     lambda: self.compareImage("testUIToggleUnchecked_Down"),
                     lambda: self._sendTouchEvent(1, avg.CURSORUP, 100, 0),
                     lambda: self.assert_(not (self.checked)),
                     lambda: self.compareImage("testUIToggleUnchecked_Up"),
                     lambda: button.setChecked(True),
                     lambda: self.compareImage("testUIToggleChecked_Up"),
                     lambda: self._sendTouchEvent(2, avg.CURSORDOWN, 0, 0),
                     lambda: self.compareImage("testUIToggleChecked_Down"),
                     lambda: self._sendTouchEvent(2, avg.CURSORUP, 100, 0),
                     lambda: self.assert_(not(self.unchecked)),
                     lambda: self.compareImage("testUIToggleChecked_Up"),
                    ))

        def testToggleDisable():
            self.start(False,
                    (reset,
                     lambda: self.compareImage("testUIToggleUnchecked_Up"),
                     lambda: button.setEnabled(False),
                     lambda: self.compareImage("testUIToggleUnchecked_Disabled"),
                     lambda: self._sendTouchEvent(1, avg.CURSORDOWN, 0, 0),
                     lambda: self._sendTouchEvent(1, avg.CURSORUP, 0, 0),
                     lambda: self.compareImage("testUIToggleUnchecked_Disabled"),
                     lambda: button.setEnabled(True),
                     lambda: self.compareImage("testUIToggleUnchecked_Up"),
                     lambda: self._sendTouchEvent(2, avg.CURSORDOWN, 0, 0),
                     lambda: button.setEnabled(False),
                     lambda: self.assert_(not (self.checked)),
                     lambda: self.compareImage("testUIToggleUnchecked_Disabled"),
                     
                     lambda: button.setEnabled(True),
                     reset,
                     lambda: self.compareImage("testUIToggleUnchecked_Up"),
                     lambda: button.setChecked(True),
                     lambda: self.compareImage("testUIToggleChecked_Up"),
                     lambda: button.setEnabled(False),
                     lambda: self.compareImage("testUIToggleChecked_Disabled"),
                     lambda: self._sendTouchEvent(3, avg.CURSORDOWN, 0, 0),
                     lambda: self._sendTouchEvent(3, avg.CURSORUP, 0, 0),
                     lambda: self.compareImage("testUIToggleChecked_Disabled"),
                     lambda: button.setEnabled(True),
                     lambda: self.compareImage("testUIToggleChecked_Up"),
                     lambda: self._sendTouchEvent(4, avg.CURSORDOWN, 0, 0),
                     lambda: button.setEnabled(False),
                     lambda: self.assert_(not (self.unchecked)),
                     lambda: self.compareImage("testUIToggleChecked_Disabled"),
                    ))
        
        button = createScene()
        testToggle()
        
        button = createScene()
        testToggleAbort()

        button = createScene()
        testToggleDisable()

    def testScrollPane(self):
        def scrollLarge():
            scrollPane.contentpos = (-34, -34)
            self.assertEqual(scrollPane.contentpos, (-32,-32))

        def initSmallContent():
            scrollPane.size = (64, 64)
            contentArea.size = (32, 32)
            image.size = (32, 32)
            scrollPane.contentpos = (0, 0)
            self.assertEqual(scrollPane.getMaxContentPos(), (32,32))

        def scrollSmall():
            scrollPane.contentpos = (32, 32)

        root = self.loadEmptyScene()
        contentArea = avg.DivNode(size=(64,64))
        image = avg.ImageNode(href="rgb24-64x64.png", parent=contentArea)
        scrollPane = ui.ScrollPane(contentDiv=contentArea, size=(32,32), parent=root)

        self.start(False,
                (lambda: self.compareImage("testScrollPane1"),
                 scrollLarge,
                 lambda: self.compareImage("testScrollPane2"),
                 initSmallContent,
                 lambda: self.compareImage("testScrollPane3"),
                 scrollSmall,
                 lambda: self.compareImage("testScrollPane4"),
                ))


def uiTestSuite(tests):
    availableTests = (
        "testKeyboard",
        "testTextArea",
        "testFocusContext",
        "testButton",
        "testMultitouchButton",
        "testTouchButton",
        "testToggleButton",
        "testScrollPane"
        )

    return createAVGTestSuite(availableTests, UITestCase, tests)
