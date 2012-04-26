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

from libavg import avg, textarea, ui

import math
from sets import Set
from testcase import *

class UITestCase(AVGTestCase):
    def __init__(self, testFuncName):
        AVGTestCase.__init__(self, testFuncName)

    def testKeyboard(self):
        def setup():
            keyDefs = [
                    [("a", "A"), ( 5, 5), (30, 30)],
                    [(1, ),      (35, 5), (30, 30)],
                    ["SHIFT",    (65, 5), (50, 30)]]
            kbNoShift = ui.Keyboard("keyboard_bg.png", "keyboard_ovl.png", keyDefs, None,
                    pos=(10, 10), parent = root)
            kbNoShift.setKeyHandler(onKeyDown, onKeyUp)
            kbShift = ui.Keyboard("keyboard_bg.png", "keyboard_ovl.png", keyDefs, "SHIFT",
                    pos=(10, 60), parent = root)
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
        self.start((
                 setup,
                 lambda: self.compareImage("testUIKeyboard", False),
                 # test character key
                 lambda: self._sendMouseEvent(avg.CURSORDOWN, 30, 30),
                 lambda: self.assert_(self.__keyDown and not self.__keyUp),
                 lambda: self.assert_(self.__char == "a" and self.__cmd is None),
                 lambda: self.compareImage("testUIKeyboardDownA1", False),
                 lambda: self._sendMouseEvent(avg.CURSORUP, 30, 30),
                 lambda: self.assert_(not self.__keyDown and self.__keyUp),
                 lambda: self.assert_(self.__char == "a" and self.__cmd is None),
                 lambda: self.compareImage("testUIKeyboard", False),
                 # test command key
                 lambda: self._sendMouseEvent(avg.CURSORDOWN, 100, 30),
                 lambda: self.assert_(self.__keyDown and not self.__keyUp),
                 lambda: self.assert_(self.__char is None and self.__cmd == "SHIFT"),
                 lambda: self.compareImage("testUIKeyboardDownS1", False),
                 lambda: self._sendMouseEvent(avg.CURSORUP, 100, 30),
                 lambda: self.assert_(not self.__keyDown and self.__keyUp),
                 lambda: self.assert_(self.__char is None and self.__cmd == "SHIFT"),
                 lambda: self.compareImage("testUIKeyboard", False),
                 # test shift key (no shift key support)
                 lambda: self._sendTouchEvent(1, avg.CURSORDOWN, 100, 30),
                 lambda: self._sendTouchEvent(2, avg.CURSORDOWN, 30, 30),
                 lambda: self.assert_(self.__char == "a" and self.__cmd is None),
                 lambda: self._sendTouchEvent(3, avg.CURSORDOWN, 60, 30),
                 lambda: self.assert_(self.__char == 1 and self.__cmd is None),
                 lambda: self.compareImage("testUIKeyboardDownA111S1", False),
                 lambda: self._sendTouchEvent(2, avg.CURSORUP, 30, 30),
                 lambda: self.assert_(self.__char == "a" and self.__cmd is None),
                 lambda: self._sendTouchEvent(3, avg.CURSORUP, 60, 30),
                 lambda: self.assert_(self.__char == 1 and self.__cmd is None),
                 lambda: self._sendTouchEvent(1, avg.CURSORUP, 100, 30),
                 lambda: self.compareImage("testUIKeyboard", False),
                 # test shift key (with shift key support)
                 lambda: self._sendTouchEvent(1, avg.CURSORDOWN, 100, 80),
                 lambda: self._sendTouchEvent(2, avg.CURSORDOWN, 30, 80),
                 lambda: self.assert_(self.__char == "A" and self.__cmd is None),
                 lambda: self._sendTouchEvent(3, avg.CURSORDOWN, 60, 80),
                 lambda: self.assert_(self.__char == 1 and self.__cmd is None),
                 lambda: self.compareImage("testUIKeyboardDownA212S2", False),
                 lambda: self._sendTouchEvent(2, avg.CURSORUP, 30, 80),
                 lambda: self.assert_(self.__char == "A" and self.__cmd is None),
                 lambda: self._sendTouchEvent(3, avg.CURSORUP, 60, 80),
                 lambda: self.assert_(self.__char == 1 and self.__cmd is None),
                 lambda: self._sendTouchEvent(1, avg.CURSORUP, 100, 80),
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
        avg.DivNode(id="ph1", pos=(2,2), size=(156, 96), parent=root)
        avg.DivNode(id="ph2", pos=(2,100), size=(156, 18), parent=root)

        textarea.init(avg, False)
        self.start((
                setup,
                lambda: self.assertEqual(self.ta1.getText(), 'Lorem ipsum'),
                lambda: setAndCheck(self.ta1, ''),
                lambda: setAndCheck(self.ta2, 'Lorem Ipsum'),
                testUnicode,
                lambda: self.compareImage("testTextArea1", True),
                testSpecialChars,
                checkSingleLine,
                lambda: self.compareImage("testTextArea2", True),
               ))


    def testTapRecognizer(self):

        def onPossible(event):
            self.__possible = True

        def onDetected(event):
            self.__detected = True

        def onFail(event):
            self.__failed = True

        def initState():
            self.__possible = False
            self.__detected = False
            self.__failed = False

        def abort():
            self.__tapRecognizer.abort()
            initState()

        def enable(isEnabled):
            self.__tapRecognizer.enable(isEnabled)
            initState()

        EVENT_POSSIBLE = 1
        EVENT_DETECTED = 2
        EVENT_FAILED = 3
        def assertEvents(flags):
            self.assert_((EVENT_POSSIBLE in flags) == self.__possible)
            self.assert_((EVENT_DETECTED in flags) == self.__detected)
            self.assert_((EVENT_FAILED in flags) == self.__failed)

        root = self.loadEmptyScene()
        image = avg.ImageNode(parent=root, href="rgb24-64x64.png")
        self.__tapRecognizer = ui.TapRecognizer(image,
                possibleHandler=onPossible,
                detectedHandler=onDetected,
                failHandler=onFail)
        initState()
        Player.setFakeFPS(10)
        self.start((
                 # Down-up: recognized as tap.
                 lambda: self._sendMouseEvent(avg.CURSORDOWN, 30, 30),
                 lambda: assertEvents(Set([EVENT_POSSIBLE])),
                 lambda: self._sendMouseEvent(avg.CURSORUP, 30, 30),
                 lambda: assertEvents(Set([EVENT_POSSIBLE, EVENT_DETECTED])),
                 # Down-small move-up: recognized as tap.
                 initState,
                 lambda: self._sendMouseEvent(avg.CURSORDOWN, 30, 30),
                 lambda: self._sendMouseEvent(avg.CURSORMOTION, 31, 30),
                 lambda: assertEvents(Set([EVENT_POSSIBLE])),
                 lambda: self._sendMouseEvent(avg.CURSORUP, 30, 30),
                 lambda: assertEvents(Set([EVENT_POSSIBLE, EVENT_DETECTED])),
                 # Down-big move-up: fail
                 initState,
                 lambda: self._sendMouseEvent(avg.CURSORDOWN, 1, 1),
                 lambda: self._sendMouseEvent(avg.CURSORMOTION, 150, 50),
                 lambda: assertEvents(Set([EVENT_POSSIBLE, EVENT_FAILED])),
                 lambda: self._sendMouseEvent(avg.CURSORUP, 1, 1),
                 lambda: assertEvents(Set([EVENT_POSSIBLE, EVENT_FAILED])),
                 # Down-delay: fail
                 lambda: self._sendMouseEvent(avg.CURSORDOWN, 30, 30),
                 lambda: self.delay(1000),
                 lambda: assertEvents(Set([EVENT_POSSIBLE, EVENT_FAILED])),
                 lambda: self._sendMouseEvent(avg.CURSORUP, 30, 30),
                 lambda: assertEvents(Set([EVENT_POSSIBLE, EVENT_FAILED])),
                 # Down-Abort-Up: not recognized as tap
                 initState,
                 lambda: self._sendMouseEvent(avg.CURSORDOWN, 30, 30),
                 lambda: assertEvents(Set([EVENT_POSSIBLE])),
                 abort,
                 lambda: self._sendMouseEvent(avg.CURSORUP, 30, 30),
                 lambda: assertEvents(Set([])),
                 # Abort-Down-Up: recognized as tap
                 initState,
                 abort,
                 lambda: self._sendMouseEvent(avg.CURSORDOWN, 30, 30),
                 lambda: assertEvents(Set([EVENT_POSSIBLE])),
                 lambda: self._sendMouseEvent(avg.CURSORUP, 30, 30),
                 lambda: assertEvents(Set([EVENT_POSSIBLE, EVENT_DETECTED])),
                 # Down-Abort-Up-Down-Up: recognized as tap
                 initState,
                 lambda: self._sendMouseEvent(avg.CURSORDOWN, 0, 0),
                 lambda: assertEvents(Set([EVENT_POSSIBLE])),
                 abort,
                 lambda: self._sendMouseEvent(avg.CURSORUP, 30, 30),
                 lambda: assertEvents(Set([])),
                 lambda: self._sendMouseEvent(avg.CURSORDOWN, 30, 30),
                 lambda: assertEvents(Set([EVENT_POSSIBLE])),
                 lambda: self._sendMouseEvent(avg.CURSORUP, 30, 30),
                 lambda: assertEvents(Set([EVENT_POSSIBLE, EVENT_DETECTED])),
                 # Disable-Down-Up-Enable: not recognized as tap
                 initState,
                 lambda: enable(False),
                 lambda: self._sendMouseEvent(avg.CURSORDOWN, 30, 30),
                 lambda: assertEvents(Set([])),
                 lambda: self._sendMouseEvent(avg.CURSORUP, 30, 30),
                 lambda: assertEvents(Set([])),
                 lambda: enable(True),
                 # Down-Disable-Up-Enable: not recognized as tap
                 initState,
                 lambda: self._sendMouseEvent(avg.CURSORDOWN, 30, 30),
                 lambda: assertEvents(Set([EVENT_POSSIBLE])),
                 lambda: enable(False),
                 lambda: self._sendMouseEvent(avg.CURSORUP, 30, 30),
                 lambda: assertEvents(Set([])),
                 lambda: enable(True),
                 # Down-Disable-Enable-Up: not recognized as tap
                 initState,
                 lambda: self._sendMouseEvent(avg.CURSORDOWN, 30, 30),
                 lambda: assertEvents(Set([EVENT_POSSIBLE])),
                 lambda: enable(False),
                 lambda: enable(True),
                 lambda: self._sendMouseEvent(avg.CURSORUP, 30, 30),
                 lambda: assertEvents(Set([])),
                 # Down-Disable-Up-Enable-Down-Up: recognized as tap
                 initState,
                 lambda: self._sendMouseEvent(avg.CURSORDOWN, 30, 30),
                 lambda: assertEvents(Set([EVENT_POSSIBLE])), 
                 lambda: enable(False),
                 lambda: self._sendMouseEvent(avg.CURSORUP, 30, 30),
                 lambda: assertEvents(Set([])),
                 lambda: enable(True),
                 lambda: self._sendMouseEvent(avg.CURSORDOWN, 30, 30),
                 lambda: assertEvents(Set([EVENT_POSSIBLE])),
                 lambda: self._sendMouseEvent(avg.CURSORUP, 30, 30),
                 lambda: assertEvents(Set([EVENT_POSSIBLE, EVENT_DETECTED])),
                 # Down-Abort-Disable-Up-Enable: not recognized as tap
                 initState,
                 lambda: self._sendMouseEvent(avg.CURSORDOWN, 30, 30),
                 lambda: assertEvents(Set([EVENT_POSSIBLE])),
                 abort,
                 lambda: enable(False),
                 lambda: self._sendMouseEvent(avg.CURSORUP, 30, 30),
                 lambda: assertEvents(Set([])),
                 lambda: enable(True),
                 # Abort-Disable-Abort-Enable-Abort-Down-Up: recognized as tap
                 initState,
                 abort,
                 lambda: enable(False),
                 abort,
                 lambda: enable(True),
                 abort,
                 lambda: self._sendMouseEvent(avg.CURSORDOWN, 30, 30),
                 lambda: assertEvents(Set([EVENT_POSSIBLE])),
                 lambda: self._sendMouseEvent(avg.CURSORUP, 30, 30),
                 lambda: assertEvents(Set([EVENT_POSSIBLE, EVENT_DETECTED])),
                ))


    def testHoldRecognizer(self):

        def onPossible(event):
            self.__possible = True

        def onDetected(event):
            self.__detected = True

        def onFail(event):
            self.__failed = True

        def onStop(event):
            self.__stopped = True

        def initState():
            self.__possible = False
            self.__detected = False
            self.__failed = False
            self.__stopped = False

        def abort():
            self.__holdRecognizer.abort()
            initState()

        def enable(isEnabled):
            self.__holdRecognizer.enable(isEnabled)
            initState()

        EVENT_POSSIBLE = 1
        EVENT_DETECTED = 2
        EVENT_FAILED = 3
        EVENT_STOPPED = 4
        def assertEvents(flags):
            self.assert_((EVENT_POSSIBLE in flags) == self.__possible)
            self.assert_((EVENT_DETECTED in flags) == self.__detected)
            self.assert_((EVENT_FAILED in flags) == self.__failed)
            self.assert_((EVENT_STOPPED in flags) == self.__stopped)

        Player.setFakeFPS(2)
        root = self.loadEmptyScene()
        image = avg.ImageNode(parent=root, href="rgb24-64x64.png")
        self.__holdRecognizer = ui.HoldRecognizer(image,
                delay=1000,
                possibleHandler=onPossible, 
                detectedHandler=onDetected, 
                failHandler=onFail, 
                stopHandler=onStop)
        initState()
        self.start((
                 # Standard down-hold-up sequence.
                 lambda: self._sendMouseEvent(avg.CURSORDOWN, 30, 30),
                 lambda: assertEvents(Set([EVENT_POSSIBLE])),
                 None,
                 lambda: assertEvents(Set([EVENT_POSSIBLE, EVENT_DETECTED])),
                 None,
                 lambda: self._sendMouseEvent(avg.CURSORUP, 30, 30),
                 lambda: assertEvents(Set([EVENT_POSSIBLE, EVENT_DETECTED,
                         EVENT_STOPPED])),

                 # down-up sequence, hold not long enough.
                 initState,
                 lambda: self._sendMouseEvent(avg.CURSORDOWN, 30, 30),
                 lambda: self._sendMouseEvent(avg.CURSORUP, 30, 30),
                 lambda: assertEvents(Set([EVENT_POSSIBLE, EVENT_FAILED])),

                 # down-move-up sequence, should stop. 
                 initState,
                 lambda: self._sendMouseEvent(avg.CURSORDOWN, 1, 1),
                 lambda: self._sendMouseEvent(avg.CURSORMOTION, 150, 50),
                 lambda: assertEvents(Set([EVENT_POSSIBLE, EVENT_FAILED])),
                 lambda: self._sendMouseEvent(avg.CURSORUP, 150, 50),
                 lambda: assertEvents(Set([EVENT_POSSIBLE, EVENT_FAILED])),

                 # down-hold-abort-up, should not recognized
                 initState,
                 lambda: self._sendMouseEvent(avg.CURSORDOWN, 30, 30),
                 lambda: assertEvents(Set([EVENT_POSSIBLE])),
                 None,
                 lambda: assertEvents(Set([EVENT_POSSIBLE, EVENT_DETECTED])),
                 None,
                 abort,
                 lambda: self._sendMouseEvent(avg.CURSORUP, 30, 30),
                 lambda: assertEvents(Set([])),

                 # down-abort-hold-up, should not recognized
                 initState,
                 lambda: self._sendMouseEvent(avg.CURSORDOWN, 30, 30),
                 lambda: assertEvents(Set([EVENT_POSSIBLE])),
                 abort,
                 None,
                 lambda: assertEvents(Set([])),
                 None,
                 lambda: self._sendMouseEvent(avg.CURSORUP, 30, 30),
                 lambda: assertEvents(Set([])),

                 # down-hold-disabled-up-enabled, should not recognized
                 initState,
                 lambda: self._sendMouseEvent(avg.CURSORDOWN, 30, 30),
                 lambda: assertEvents(Set([EVENT_POSSIBLE])),
                 None,
                 lambda: assertEvents(Set([EVENT_POSSIBLE, EVENT_DETECTED])),
                 None,
                 lambda: enable(False),
                 lambda: self._sendMouseEvent(avg.CURSORUP, 30, 30),
                 lambda: assertEvents(Set([])),
                 lambda: enable(True),

                 # down-disabled-enabled-hold-up, should not recognized
                 initState,
                 lambda: self._sendMouseEvent(avg.CURSORDOWN, 30, 30),
                 lambda: assertEvents(Set([EVENT_POSSIBLE])),
                 lambda: enable(False),
                 lambda: enable(True),
                 None,
                 lambda: assertEvents(Set([])),
                 None,
                 lambda: self._sendMouseEvent(avg.CURSORUP, 30, 30),
                 lambda: assertEvents(Set([])),
                ))
        Player.setFakeFPS(-1)


    def testDoubletapRecognizer(self):

        EVENT_POSSIBLE = 1
        EVENT_DETECTED = 2
        EVENT_FAILED = 3
        def onPossible(event):
            self.__flags.add(EVENT_POSSIBLE)

        def onDetected(event):
            self.__flags.add(EVENT_DETECTED)

        def onFail(event):
            self.__flags.add(EVENT_FAILED)

        def initState():
            self.__flags = Set()

        def assertEvents(flags):
            wantedFlags = Set(flags)
            if wantedFlags != self.__flags:
                print "State expected: ", wantedFlags
                print "Actual state: ", self.__flags
                self.assert_(False)

        def checkMouseEvent(type, x, y, eventFlags):
            return [
                     lambda: self._sendMouseEvent(type, x, y),
                     lambda: assertEvents(eventFlags)
                    ]

        def abort():
            self.__tapRecognizer.abort()
            initState()

        def enable(isEnabled):
            self.__tapRecognizer.enable(isEnabled)
            initState()

        root = self.loadEmptyScene()
        image = avg.ImageNode(parent=root, href="rgb24-64x64.png", size=(128,128))
        self.__tapRecognizer = ui.DoubletapRecognizer(image,
                possibleHandler=onPossible,
                detectedHandler=onDetected,
                failHandler=onFail)
        initState()
        Player.setFakeFPS(20)
        self.start((
                 # Down, up, down, up: click
                 checkMouseEvent(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                 checkMouseEvent(avg.CURSORUP, 30, 30, [EVENT_POSSIBLE]),
                 checkMouseEvent(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                 checkMouseEvent(avg.CURSORUP, 30, 30, [EVENT_POSSIBLE, EVENT_DETECTED]),
                 # Down, move: stop
                 initState,
                 checkMouseEvent(avg.CURSORDOWN, 0, 30, [EVENT_POSSIBLE]),
                 checkMouseEvent(avg.CURSORMOTION, 80, 30, 
                        [EVENT_POSSIBLE, EVENT_FAILED]),
                 checkMouseEvent(avg.CURSORUP, 0, 30, [EVENT_POSSIBLE, EVENT_FAILED]),
                 # Down, up, move: stop
                 initState,
                 checkMouseEvent(avg.CURSORDOWN, 0, 30, [EVENT_POSSIBLE]),
                 checkMouseEvent(avg.CURSORUP, 0, 30, [EVENT_POSSIBLE]),
                 checkMouseEvent(avg.CURSORMOTION, 80, 30, [EVENT_POSSIBLE]),
                 initState,
                 checkMouseEvent(avg.CURSORDOWN, 80, 30, [EVENT_FAILED]),
                 initState,
                 checkMouseEvent(avg.CURSORUP, 0, 30, []),
                 # Down, up, down, move: stop
                 initState,
                 checkMouseEvent(avg.CURSORDOWN, 0, 30, [EVENT_POSSIBLE]),
                 checkMouseEvent(avg.CURSORUP, 0, 30, [EVENT_POSSIBLE]),
                 checkMouseEvent(avg.CURSORDOWN, 0, 30, [EVENT_POSSIBLE]),
                 checkMouseEvent(avg.CURSORMOTION, 80, 30, 
                        [EVENT_POSSIBLE, EVENT_FAILED]),
                 checkMouseEvent(avg.CURSORUP, 0, 30, [EVENT_POSSIBLE, EVENT_FAILED]),
                 # Down,delay: stop
                 initState,
                 checkMouseEvent(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                 lambda: self.delay(600),
                 lambda: assertEvents(Set([EVENT_POSSIBLE, EVENT_FAILED])),
                 checkMouseEvent(avg.CURSORUP, 30, 30, [EVENT_POSSIBLE, EVENT_FAILED]),
                 # Down, up, delay: stop
                 initState,
                 checkMouseEvent(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                 checkMouseEvent(avg.CURSORUP, 30, 30, [EVENT_POSSIBLE]),
                 lambda: self.delay(600),
                 lambda: assertEvents(Set([EVENT_POSSIBLE, EVENT_FAILED])),
                 # Down, up, down, delay: stop
                 initState,
                 checkMouseEvent(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                 checkMouseEvent(avg.CURSORUP, 30, 30, [EVENT_POSSIBLE]),
                 checkMouseEvent(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                 lambda: self.delay(600),
                 lambda: assertEvents(Set([EVENT_POSSIBLE, EVENT_FAILED])),
                 checkMouseEvent(avg.CURSORUP, 30, 30, [EVENT_POSSIBLE, EVENT_FAILED]),
                 # Down, abort, up, down, up, delay: just one click
                 initState,
                 checkMouseEvent(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                 abort,
                 checkMouseEvent(avg.CURSORUP, 30, 30, []),
                 checkMouseEvent(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                 checkMouseEvent(avg.CURSORUP, 30, 30, [EVENT_POSSIBLE]),
                 lambda: self.delay(600),
                 lambda: assertEvents(Set([EVENT_POSSIBLE, EVENT_FAILED])),
                 # Down, up, abort, down, up, delay: two clicks but no double-click
                 initState,
                 checkMouseEvent(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                 checkMouseEvent(avg.CURSORUP, 30, 30, [EVENT_POSSIBLE]),
                 abort,
                 checkMouseEvent(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                 checkMouseEvent(avg.CURSORUP, 30, 30, [EVENT_POSSIBLE]),
                 lambda: self.delay(600),
                 lambda: assertEvents(Set([EVENT_POSSIBLE, EVENT_FAILED])),
                 # Down, up, down, abort, up: just one click
                 initState,
                 checkMouseEvent(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                 checkMouseEvent(avg.CURSORUP, 30, 30, [EVENT_POSSIBLE]),
                 checkMouseEvent(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                 abort,
                 checkMouseEvent(avg.CURSORUP, 30, 30, []),
                 # Down, abort, up, down, up, down up: first aborted then recognized
                 initState,
                 checkMouseEvent(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                 abort,
                 checkMouseEvent(avg.CURSORUP, 30, 30, []),
                 checkMouseEvent(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                 checkMouseEvent(avg.CURSORUP, 30, 30, [EVENT_POSSIBLE]),
                 checkMouseEvent(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                 checkMouseEvent(avg.CURSORUP, 30, 30, [EVENT_POSSIBLE, EVENT_DETECTED]),
                 # Disabled, down, up, down, up, enabled: nothing
                 initState,
                 lambda: enable(False),
                 checkMouseEvent(avg.CURSORDOWN, 30, 30, []),
                 checkMouseEvent(avg.CURSORUP, 30, 30, []),
                 checkMouseEvent(avg.CURSORDOWN, 30, 30, []),
                 checkMouseEvent(avg.CURSORUP, 30, 30, []),
                 lambda: enable(True),                 
                 # Down, disabled up, down, up, enabled: just one down
                 initState,
                 checkMouseEvent(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                 lambda: enable(False),
                 checkMouseEvent(avg.CURSORUP, 30, 30, []),
                 checkMouseEvent(avg.CURSORDOWN, 30, 30, []),
                 checkMouseEvent(avg.CURSORUP, 30, 30, []),
                 lambda: enable(True),
                 # Down, up, disabled, down, up, enabled: just one click
                 initState,
                 checkMouseEvent(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                 checkMouseEvent(avg.CURSORUP, 30, 30, [EVENT_POSSIBLE]),
                 lambda: enable(False),
                 checkMouseEvent(avg.CURSORDOWN, 30, 30, []),
                 checkMouseEvent(avg.CURSORUP, 30, 30, []),
                 lambda: enable(True),
                 # Down, up, down, disabled, up, enabled: just one click
                 initState,
                 checkMouseEvent(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                 checkMouseEvent(avg.CURSORUP, 30, 30, [EVENT_POSSIBLE]),
                 checkMouseEvent(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                 lambda: enable(False),
                 checkMouseEvent(avg.CURSORUP, 30, 30, []),
                 lambda: enable(True),
                 # Down, disabled, enabled, up, down, up: just one click
                 initState,
                 checkMouseEvent(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                 lambda: enable(False),
                 lambda: enable(True),
                 checkMouseEvent(avg.CURSORUP, 30, 30, []),
                 checkMouseEvent(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                 checkMouseEvent(avg.CURSORUP, 30, 30, [EVENT_POSSIBLE]),
                 # Down, disabled, enabled, up, down, up, down, up: recognized
                 checkMouseEvent(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                 lambda: enable(False),
                 lambda: enable(True),
                 checkMouseEvent(avg.CURSORUP, 30, 30, []),
                 checkMouseEvent(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                 checkMouseEvent(avg.CURSORUP, 30, 30, [EVENT_POSSIBLE]),
                 checkMouseEvent(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                 checkMouseEvent(avg.CURSORUP, 30, 30, [EVENT_POSSIBLE, EVENT_DETECTED]),
                ))


    def testDragRecognizer(self):

        EVENT_DETECTED = 1
        EVENT_MOVED = 2
        EVENT_UP = 3
        EVENT_ENDED = 4
        EVENT_POSSIBLE = 5
        EVENT_FAILED = 6
        def onDetected(event):
            self.__flags.add(EVENT_DETECTED)

        def onMove(event, offset):
            if self.friction == -1:
                self.assertEqual(offset, (40,40))
            self.__flags.add(EVENT_MOVED)

        def onUp(event, offset):
            if self.friction == -1:
                self.assertEqual(offset, (10,-10))
            self.__flags.add(EVENT_UP)

        def onEnd(event):
            self.__flags.add(EVENT_ENDED)

        def enable(isEnabled):
            dragRecognizer.enable(isEnabled)
            initState()

        def abort():
            dragRecognizer.abort()
            initState()

        def initState():
            self.__flags = Set()

        def assertDragEvents(flags):
            wantedFlags = Set(flags)
            if wantedFlags != self.__flags:
                print "State expected: ", wantedFlags
                print "Actual state: ", self.__flags
                self.assert_(False)

        def checkMouseEvent(type, x, y, eventFlags):
            return [
                     lambda: self._sendMouseEvent(type, x, y),
                     lambda: assertDragEvents(eventFlags)
                    ]

        Player.setFakeFPS(100)
        for self.friction in (-1, 100):
            root = self.loadEmptyScene()
            image = avg.ImageNode(parent=root, href="rgb24-64x64.png")
            dragRecognizer = ui.DragRecognizer(image, 
                    detectedHandler=onDetected, moveHandler=onMove, upHandler=onUp, 
                    endHandler=onEnd, friction=self.friction)
            initState()
            self.start((
                     checkMouseEvent(avg.CURSORDOWN, 30, 30, [EVENT_DETECTED]),
                     checkMouseEvent(avg.CURSORMOTION, 70, 70, 
                            [EVENT_DETECTED, EVENT_MOVED]),
                     checkMouseEvent(avg.CURSORUP, 40, 20, 
                            [EVENT_DETECTED, EVENT_MOVED, EVENT_UP, EVENT_ENDED]),
                     lambda: enable(False),
                     checkMouseEvent(avg.CURSORDOWN, 30, 30, []),
                     lambda: dragRecognizer.enable(True),
                     checkMouseEvent(avg.CURSORUP, 30, 30, []),
                     checkMouseEvent(avg.CURSORDOWN, 30, 30, [EVENT_DETECTED]),
                    ))

        # Test with constraint.
        def onPossible(event):
            self.__flags.add(EVENT_POSSIBLE)

        def onFail(event):
            self.__flags.add(EVENT_FAILED)

        def onVertMove(event, offset):
            if self.friction == -1:
                self.assertEqual(offset, (0,40))
            self.__flags.add(EVENT_MOVED)

        for self.friction in (-1, 100):
            root = self.loadEmptyScene()
            image = avg.ImageNode(parent=root, href="rgb24-64x64.png")
            dragRecognizer = ui.DragRecognizer(image, 
                    possibleHandler=onPossible, failHandler=onFail, 
                    detectedHandler=onDetected, 
                    moveHandler=onVertMove, upHandler=onUp, endHandler=onEnd, 
                    friction=self.friction, direction=ui.DragRecognizer.VERTICAL)
            initState()
            self.start((
                     checkMouseEvent(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                     checkMouseEvent(avg.CURSORMOTION, 35, 30, [EVENT_POSSIBLE]),
                     checkMouseEvent(avg.CURSORMOTION, 30, 70, 
                            [EVENT_DETECTED, EVENT_MOVED, EVENT_POSSIBLE]),
                     checkMouseEvent(avg.CURSORUP, 40, 20, 
                            [EVENT_DETECTED, EVENT_MOVED, EVENT_UP,
                                    EVENT_ENDED, EVENT_POSSIBLE]),
                     initState,
                     # Wrong direction -> stop.
                     checkMouseEvent(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                     checkMouseEvent(avg.CURSORMOTION, 70, 30, 
                            [EVENT_POSSIBLE, EVENT_FAILED]),
                     checkMouseEvent(avg.CURSORUP, 70, 30, 
                            [EVENT_POSSIBLE, EVENT_FAILED]),

                     # No movement -> stop.
                     initState,
                     checkMouseEvent(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                     checkMouseEvent(avg.CURSORUP, 30, 30, 
                            [EVENT_POSSIBLE, EVENT_FAILED]),

                     # Down, Abort, Motion, Motion, Up -> not recognized
                     initState,
                     checkMouseEvent(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                     abort,
                     checkMouseEvent(avg.CURSORMOTION, 35, 30, []),
                     checkMouseEvent(avg.CURSORMOTION, 30, 70, []),
                     checkMouseEvent(avg.CURSORUP, 40, 20, []),

                     # Down, Motion, Abort, Motion, Up -> not Recognized
                     initState,
                     checkMouseEvent(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                     checkMouseEvent(avg.CURSORMOTION, 35, 30, [EVENT_POSSIBLE]),
                     abort,
                     checkMouseEvent(avg.CURSORMOTION, 30, 70, []),
                     checkMouseEvent(avg.CURSORUP, 40, 20, []),

                     # Down, Motion, Motion, Abort, Up -> not recognized
                     initState,
                     checkMouseEvent(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                     checkMouseEvent(avg.CURSORMOTION, 35, 30, [EVENT_POSSIBLE]),
                     checkMouseEvent(avg.CURSORMOTION, 30, 70,
                            [EVENT_DETECTED, EVENT_MOVED, EVENT_POSSIBLE]),
                     abort,
                     checkMouseEvent(avg.CURSORUP, 40, 20, []),

                     # Down, Motion, Abort, Up, Down, Motion, Motion, Up -> Recognized
                     initState,
                     checkMouseEvent(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                     checkMouseEvent(avg.CURSORMOTION, 35, 30, [EVENT_POSSIBLE]),
                     abort,
                     checkMouseEvent(avg.CURSORUP, 40, 20, []),
                     
                     checkMouseEvent(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                     checkMouseEvent(avg.CURSORMOTION, 35, 30, [EVENT_POSSIBLE]),
                     checkMouseEvent(avg.CURSORMOTION, 30, 70,
                            [EVENT_DETECTED, EVENT_MOVED, EVENT_POSSIBLE]),
                     checkMouseEvent(avg.CURSORUP, 40, 20, 
                            [EVENT_DETECTED, EVENT_MOVED, EVENT_UP, EVENT_ENDED,
                             EVENT_POSSIBLE]),
                    ))

        # Test second down during inertia.
        root = self.loadEmptyScene()
        image = avg.ImageNode(parent=root, href="rgb24-64x64.png")
        dragRecognizer = ui.DragRecognizer(image, 
                possibleHandler=onPossible, failHandler=onFail, 
                detectedHandler=onDetected, 
                moveHandler=onMove, upHandler=onUp, endHandler=onEnd, 
                friction=0.01)
        initState()
        self.start((
                 lambda: self._sendMouseEvent(avg.CURSORDOWN, 30, 30),
                 lambda: self._sendMouseEvent(avg.CURSORUP, 40, 20),
                 initState,
                 checkMouseEvent(avg.CURSORDOWN, 40, 20, 
                            [EVENT_ENDED, EVENT_DETECTED, EVENT_MOVED]),
                 ))

        # Test second down during inertia, constrained recognizer
        root = self.loadEmptyScene()
        image = avg.ImageNode(parent=root, href="rgb24-64x64.png")
        dragRecognizer = ui.DragRecognizer(image, 
                possibleHandler=onPossible, failHandler=onFail, 
                detectedHandler=onDetected, 
                moveHandler=onMove, upHandler=onUp, endHandler=onEnd, 
                friction=0.01, direction=ui.DragRecognizer.VERTICAL)
        initState()
        self.start((
                 lambda: self._sendMouseEvent(avg.CURSORDOWN, 30, 30),
                 checkMouseEvent(avg.CURSORMOTION, 30, 70,
                        [EVENT_DETECTED, EVENT_MOVED, EVENT_POSSIBLE]),
                 checkMouseEvent(avg.CURSORUP, 30, 70,
                        [EVENT_DETECTED, EVENT_MOVED, EVENT_UP, EVENT_POSSIBLE]),
                 initState,
                 checkMouseEvent(avg.CURSORDOWN, 30, 30, 
                        [EVENT_MOVED, EVENT_ENDED, EVENT_POSSIBLE]),
                 initState,
                 checkMouseEvent(avg.CURSORMOTION, 30, 70, 
                        [EVENT_DETECTED, EVENT_MOVED]),
                 ))

        Player.setFakeFPS(-1)


    def testDragRecognizerRelCoords(self):

        def onDrag(event, offset):
            self.assertAlmostEqual(offset, (-40,-40))

        Player.setFakeFPS(100)
        for self.friction in (-1, 100):
            root = self.loadEmptyScene()
            div = avg.DivNode(pos=(64,64), angle=math.pi, parent=root)
            image = avg.ImageNode(parent=div, href="rgb24-64x64.png")
            ui.DragRecognizer(image, moveHandler=onDrag, friction=self.friction)
            self.start((
                     lambda: self._sendMouseEvent(avg.CURSORDOWN, 30, 30),
                     lambda: self._sendMouseEvent(avg.CURSORMOTION, 70, 70),
                    ))
        Player.setFakeFPS(-1)


    def testDragRecognizerInitialEvent(self):

        def onMotion(event):
            ui.DragRecognizer(self.image, 
                    detectedHandler=onDragStart, moveHandler=onDrag, initialEvent=event)
            self.image.disconnectEventHandler(self)

        def onDragStart(event):
            self.__dragStartCalled = True

        def onDrag(event, offset):
            self.assertEqual(offset, (10,0))

        root = self.loadEmptyScene()
        self.image = avg.ImageNode(parent=root, href="rgb24-64x64.png")
        self.image.connectEventHandler(avg.CURSORMOTION, avg.MOUSE, self, onMotion)
        self.__dragStartCalled = False
        self.start((
                 lambda: self._sendMouseEvent(avg.CURSORDOWN, 30, 30),
                 lambda: self._sendMouseEvent(avg.CURSORMOTION, 40, 30),
                 lambda: self._sendMouseEvent(avg.CURSORMOTION, 50, 30),
                ))
        assert(self.__dragStartCalled)


    def testDragRecognizerCoordSysNode(self):

        def onDrag(event, offset):
            self.assertEqual(offset, (40,40))

        root = self.loadEmptyScene()
        div = avg.DivNode(pos=(64,64), angle=math.pi, parent=root)
        image = avg.ImageNode(parent=div, href="rgb24-64x64.png")
        ui.DragRecognizer(image, moveHandler=onDrag, coordSysNode=div, friction=-1)
        self.start((
                 lambda: self._sendMouseEvent(avg.CURSORDOWN, 30, 30),
                 lambda: self._sendMouseEvent(avg.CURSORMOTION, 70, 70),
                ))


    def testTransformRecognizer(self):

        def onDetected(event):
            pass

        def onMove(transform):
            self.transform = transform

        def onUp(transform):
            self.transform = transform

        def checkTransform(expectedTransform):
#            print self.transform
#            print expectedTransform
#            print
            self.assertAlmostEqual(self.transform.trans, expectedTransform.trans)
            self.assertAlmostEqual(self.transform.rot, expectedTransform.rot)
            self.assertAlmostEqual(self.transform.scale, expectedTransform.scale)
            if expectedTransform.rot != 0 or expectedTransform.scale != 1:
                self.assertAlmostEqual(self.transform.pivot, expectedTransform.pivot)

        def createTransTestFrames():
            return (
                    lambda: self._sendTouchEvent(1, avg.CURSORDOWN, 10, 10),
                    lambda: self._sendTouchEvent(1, avg.CURSORUP, 20, 10),
                    lambda: checkTransform(ui.Transform((10,0))),
                )

        def createRotTestFrames(expectedTransform):
            return (
                    lambda: self._sendTouchEvents((
                            (1, avg.CURSORDOWN, 0, 10),
                            (2, avg.CURSORDOWN, 0, 20))),
                    lambda: self._sendTouchEvents((
                            (1, avg.CURSORMOTION, 0, 20),
                            (2, avg.CURSORMOTION, 0, 10))),
                    lambda: checkTransform(expectedTransform),
                    lambda: self._sendTouchEvents((
                            (1, avg.CURSORUP, 0, 20),
                            (2, avg.CURSORUP, 0, 10))),
                )

        def createScaleTestFrames(expectedTransform):
            return (
                 lambda: self._sendTouchEvent(1, avg.CURSORDOWN, 0, 10),
                 lambda: self._sendTouchEvent(2, avg.CURSORDOWN, 0, 20),
                 lambda: self._sendTouchEvent(1, avg.CURSORMOTION, 0, 10),
                 lambda: self._sendTouchEvent(2, avg.CURSORMOTION, 0, 30),
                 lambda: checkTransform(expectedTransform),
                 lambda: self._sendTouchEvent(1, avg.CURSORUP, 0, 10),
                 lambda: self._sendTouchEvent(2, avg.CURSORUP, 0, 30),
                )

        root = self.loadEmptyScene()
        image = avg.ImageNode(parent=root, href="rgb24-64x64.png")
        self.__transformRecognizer = ui.TransformRecognizer(image, 
                detectedHandler=onDetected, moveHandler=onMove, upHandler=onUp)
        self.start((
                 # Check up/down handling
                 lambda: self._sendTouchEvent(1, avg.CURSORDOWN, 10, 10),
                 lambda: checkTransform(ui.Transform((0,0))),
                 lambda: self._sendTouchEvent(1, avg.CURSORMOTION, 20, 10),
                 lambda: checkTransform(ui.Transform((10,0))),
                 lambda: self._sendTouchEvent(2, avg.CURSORDOWN, 20, 20),
                 lambda: checkTransform(ui.Transform((0,0))),
                 lambda: self._sendTouchEvents((
                        (1, avg.CURSORMOTION, 30, 10),
                        (2, avg.CURSORMOTION, 30, 20))),
                 lambda: checkTransform(ui.Transform((10,0))),
                 lambda: self._sendTouchEvent(2, avg.CURSORUP, 30, 20),
                 lambda: checkTransform(ui.Transform((0,0))),
                 lambda: self._sendTouchEvent(1, avg.CURSORMOTION, 40, 10),
                 lambda: checkTransform(ui.Transform((10,0))),
                 lambda: self._sendTouchEvent(1, avg.CURSORUP, 50, 10),
                 lambda: checkTransform(ui.Transform((10,0))),

                 createRotTestFrames(ui.Transform((0,0), math.pi, 1, (0,15))),

                 createScaleTestFrames(ui.Transform((0,5), 0, 2, (0,20)))
                ))

        # Test rel. coords.
        root = self.loadEmptyScene()
        div = avg.DivNode(parent=root, pos=(0,10))
        image = avg.ImageNode(parent=div, href="rgb24-64x64.png")
        self.__transformRecognizer = ui.TransformRecognizer(image, 
                detectedHandler=onDetected, moveHandler=onMove, upHandler=onUp)
        self.start((
            createTransTestFrames(),
            createRotTestFrames(ui.Transform((0,0), math.pi, 1, (0,5))),
            createScaleTestFrames(ui.Transform((0,5), 0, 2, (0,10))),
            ))

        # Test coordSysNode.
        root = self.loadEmptyScene()
        div = avg.DivNode(parent=root, pos=(0,10))
        image = avg.ImageNode(parent=div, href="rgb24-64x64.png")
        self.__transformRecognizer = ui.TransformRecognizer(image, coordSysNode=div,
                detectedHandler=onDetected, moveHandler=onMove, upHandler=onUp)
        self.start((
            createTransTestFrames(),
            createRotTestFrames(ui.Transform((0,0), math.pi, 1, (0,15))),
            createScaleTestFrames(ui.Transform((0,5), 0, 2, (0,20))),
            ))


    def testKMeans(self):
        pts = [avg.Point2D(0,0), avg.Point2D(0,1)]
        means = ui.calcKMeans(pts)
        self.assertEqual(means, ([0], [1]))

        pts.append (avg.Point2D(0,4))
        means = ui.calcKMeans(pts)
        self.assertEqual(means, ([0,1], [2]))


    def testMat3x3(self):
        t = ui.Mat3x3.translate([1,0,1])
        v = [1,0,1]
        self.assertEqual(t.applyVec(v), [2,0,1])
        r = ui.Mat3x3.rotate(math.pi/2)
        self.assertAlmostEqual(r.applyVec(v), [0,1,1])
        t2 = t.applyMat(t)
        self.assertAlmostEqual(t.applyMat(t).m, ui.Mat3x3.translate([2,0,1]).m)
        self.assertAlmostEqual(t.applyMat(r).m, ui.Mat3x3([0,-1,1],[1,0,0]).m)
        self.assertAlmostEqual(r.applyMat(t).m, ui.Mat3x3([0,-1,0],[1,0,1]).m)
        self.assertAlmostEqual(ui.Mat3x3().m, ui.Mat3x3().inverse().m)
        m = ui.Mat3x3([-1,  3, -3], 
                      [ 0, -6,  5],
                      [-5, -3,  1])
        im = ui.Mat3x3([3./2,      1., -1./2],
                       [-25./6, -8./3,  5./6],
                       [-5.,      -3.,    1.])
        self.assertAlmostEqual(m.inverse().m, im.m)

        image = avg.ImageNode(pos=(10,20), size=(30,40), angle=1.57, 
            href="rgb24alpha-64x64.png")
        mat = ui.Mat3x3.fromNode(image)
        mat.setNodeTransform(image)
        self.assertAlmostEqual(image.pos, (10,20))
        self.assertAlmostEqual(image.size, (30,40))
        self.assertAlmostEqual(image.angle, 1.57)


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
            self._sendMouseEvent(avg.CURSORDOWN, 20, 70)
            self._sendMouseEvent(avg.CURSORUP, 20, 70)

        root = self.loadEmptyScene()
        avg.DivNode(id="ph1", pos=(2,2), size=(156,54), parent=root)
        avg.DivNode(id="ph2", pos=(2,58), size=(76,54), parent=root)
        div3 = avg.DivNode(id="ph3", pos=(80,58), size=(76,54), parent=root)
        avg.ImageNode(href="1x1_white.png", size=(76,54), parent=div3)
        self.start((
                 setup,
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

        self.start((
                 # Normal click: Down & up over button
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self._sendMouseEvent(avg.CURSORDOWN, 0, 0),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 lambda: self._sendMouseEvent(avg.CURSORUP, 0, 0),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self.assert_(self.__clicked),
                 reset,

                 # Down, move away from button, move over button, up
                 # ==> click
                 lambda: self._sendMouseEvent(avg.CURSORDOWN, 0, 0),
                 lambda: self._sendMouseEvent(avg.CURSORMOTION, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self._sendMouseEvent(avg.CURSORMOTION, 0, 0),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self._sendMouseEvent(avg.CURSORUP, 0, 0),
                 lambda: self.assert_(self.__down and self.__clicked),
                 reset,

                 # Down, move away from button, up ==> no click
                 lambda: self._sendMouseEvent(avg.CURSORDOWN, 0, 0),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self._sendMouseEvent(avg.CURSORMOTION, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self._sendMouseEvent(avg.CURSORUP, 0, yOutDistance),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 reset,

                 # Move away from button, down, mover over button, up
                 # ==> no click
                 lambda: self._sendMouseEvent(avg.CURSORDOWN, 0, yOutDistance),
                 lambda: self._sendMouseEvent(avg.CURSORMOTION, 0, 0),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self._sendMouseEvent(avg.CURSORUP, 0, 0),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self.assert_(not(self.__down) and not(self.__clicked)),
                 reset,

                 # Move away from button, down, mover over button, move away from button, up
                 # ==> no click
                 lambda: self._sendMouseEvent(avg.CURSORDOWN, 0, yOutDistance),
                 lambda: self._sendMouseEvent(avg.CURSORMOTION, 0, 0),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self._sendMouseEvent(avg.CURSORMOTION, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self._sendMouseEvent(avg.CURSORUP, 0, yOutDistance),
                 lambda: self.assert_(not(self.__down) and not(self.__clicked)),
                 reset,

                 # Check checkable button
                 # Set checkable, down, up => pressed, down, up ==> click
                 lambda: b.setCheckable(True),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self._sendMouseEvent(avg.CURSORDOWN, 0, 0),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self._sendMouseEvent(avg.CURSORUP, 0, 0),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.assert_(self.__down and self.__clicked),
                 reset,
                 lambda: self._sendMouseEvent(avg.CURSORDOWN, 0, 0),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self._sendMouseEvent(avg.CURSORUP, 0, 0),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self.assert_(self.__down and self.__clicked),
                 reset,

                 # not checked, down, out, in, up ==> pressed, click
                 lambda: b.setCheckable(True),
                 lambda: b.setChecked(False),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self._sendMouseEvent(avg.CURSORDOWN, 0, 0),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self._sendMouseEvent(avg.CURSORMOTION, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self._sendMouseEvent(avg.CURSORMOTION, 0, 0),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self._sendMouseEvent(avg.CURSORUP, 0, 0),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.assert_(self.__down and self.__clicked),
                 reset,

                 # not checked, down, out, up ==> pressed, no click
                 lambda: b.setCheckable(True),
                 lambda: b.setChecked(False),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self._sendMouseEvent(avg.CURSORDOWN, 0, 0),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self._sendMouseEvent(avg.CURSORMOTION, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self._sendMouseEvent(avg.CURSORUP, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 reset,

                 # not checked, down, out, up ==> pressed, no click
                 lambda: b.setCheckable(True),
                 lambda: b.setChecked(True),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self._sendMouseEvent(avg.CURSORDOWN, 0, 0),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self._sendMouseEvent(avg.CURSORMOTION, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self._sendMouseEvent(avg.CURSORUP, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 reset,

                 # checked, down, out, in, up ==> pressed, clicked
                 lambda: b.setCheckable(True),
                 lambda: b.setChecked(True),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self._sendMouseEvent(avg.CURSORDOWN, 0, 0),
                 lambda: self.assert_(self.__down),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self._sendMouseEvent(avg.CURSORMOTION, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self._sendMouseEvent(avg.CURSORMOTION, 0, 0),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self._sendMouseEvent(avg.CURSORUP, 0, 0),
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
                 lambda: self._sendMouseEvent(avg.CURSORDOWN, 0, 0),
                 lambda: self._sendMouseEvent(avg.CURSORUP, 0, 0),
                 lambda: self.compareImage("testUIButtonDisabled", False),
                 lambda: self.assert_(not(self.__down) and not(self.__clicked)),
                 lambda: self._sendMouseEvent(avg.CURSORDOWN, 0, 0),
                 lambda: self.compareImage("testUIButtonDisabled", False),
                 lambda: self._sendMouseEvent(avg.CURSORUP, 0, 0),
                 lambda: self.compareImage("testUIButtonDisabled", False),
                 lambda: self.assert_(not(self.__down) and not(self.__clicked)),
                 lambda: self._sendMouseEvent(avg.CURSORDOWN, 0, 0),
                 lambda: self.compareImage("testUIButtonDisabled", False),
                 lambda: b.setEnabled(True),
                 lambda: self._sendMouseEvent(avg.CURSORUP, 0, 0),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self.assert_(not(self.__down)),
                 lambda: self.assert_(not(self.__clicked)),
                 reset,

                 # Checking functionality while resetting the visible nodes
                 lambda: b.setNodes(upNode = avg.ImageNode(href="button_up.png"),
                                    downNode = avg.ImageNode(href="button_down.png"),
                                    disabledNode = avg.ImageNode(href="button_disabled.png")),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self._sendMouseEvent(avg.CURSORDOWN, 0, 0),
                 lambda: self._sendMouseEvent(avg.CURSORUP, 0, 0),
                 lambda: self.assert_(self.__down and self.__clicked),
                 reset,

                 lambda: b.setEnabled(False),
                 lambda: b.setNodes(upNode = avg.ImageNode(href="button_up.png"),
                                    downNode = avg.ImageNode(href="button_down.png"),
                                    disabledNode = avg.ImageNode(href="button_disabled.png")),
                 lambda: self.compareImage("testUIButtonDisabled", False),
                 lambda: self._sendMouseEvent(avg.CURSORDOWN, 0, 0),
                 lambda: self._sendMouseEvent(avg.CURSORUP, 0, 0),
                 lambda: self.assert_(not(self.__down) and not(self.__clicked)),
                 reset,

                 lambda: b.setEnabled(True),
                 lambda: b.setNodes(upNode = avg.ImageNode(href="button_up.png"),
                                    downNode = avg.ImageNode(href="button_down.png"),
                                    disabledNode = None),
                 lambda: self.compareImage("testUIButtonUp", False),
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
                 lambda: self.compareImage("testUIButtonUp", False),
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
        self.start((
                 # Two downs, two ups ==> click after second up.
                 lambda: self._sendTouchEvent(1, avg.CURSORDOWN, 0, 0),
                 lambda: self._sendTouchEvent(2, avg.CURSORDOWN, 0, 0),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 lambda: self._sendTouchEvent(1, avg.CURSORUP, 0, 0),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 lambda: self._sendTouchEvent(2, avg.CURSORUP, 0, 0),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self.assert_(self.__clicked),
                 reset,

                 # Two downs, one out, out up, in up ==> click
                 lambda: self._sendTouchEvent(1, avg.CURSORDOWN, 0, 0),
                 lambda: self._sendTouchEvent(2, avg.CURSORDOWN, 0, 0),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self._sendTouchEvent(2, avg.CURSORMOTION, 0, yOutDistance),
                 lambda: self._sendTouchEvent(2, avg.CURSORUP, 0, 50),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 lambda: self._sendTouchEvent(1, avg.CURSORUP, 0, 0),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self.assert_(self.__down and self.__clicked),
                 reset,

                 # Two down, both out, both in, both up ==> click
                 lambda: self._sendTouchEvent(1, avg.CURSORDOWN, 0, 0),
                 lambda: self._sendTouchEvent(2, avg.CURSORDOWN, 0, 0),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self._sendTouchEvent(1, avg.CURSORMOTION, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self._sendTouchEvent(2, avg.CURSORMOTION, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 lambda: self._sendTouchEvent(1, avg.CURSORMOTION, 0, 0),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self._sendTouchEvent(2, avg.CURSORMOTION, 0, 0),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self._sendTouchEvent(1, avg.CURSORUP, 0, 0),
                 lambda: self._sendTouchEvent(2, avg.CURSORUP, 0, 0),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self.assert_(self.__down and self.__clicked),
                 reset,

                 # Two down both out, both up ==> no click
                 lambda: self._sendTouchEvent(1, avg.CURSORDOWN, 0, 0),
                 lambda: self._sendTouchEvent(2, avg.CURSORDOWN, 0, 0),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self._sendTouchEvent(1, avg.CURSORMOTION, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self._sendTouchEvent(2, avg.CURSORMOTION, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self._sendTouchEvent(1, avg.CURSORUP, 0, yOutDistance),
                 lambda: self._sendTouchEvent(2, avg.CURSORUP, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 reset,

                 # Two downs, one out, in up, out up ==> no click
                 lambda: self._sendTouchEvent(1, avg.CURSORDOWN, 0, 0),
                 lambda: self._sendTouchEvent(2, avg.CURSORDOWN, 0, 0),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self._sendTouchEvent(2, avg.CURSORMOTION, 0, yOutDistance),
                 lambda: self._sendTouchEvent(1, avg.CURSORUP, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 lambda: self._sendTouchEvent(2, avg.CURSORUP, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 reset,

                 # Check checkable multitouch button
                 # 2 down, 2 up ==> pressed, clicked, 
                 # 2 down, 2 up ==> pressed, clicked
                 lambda: b.setCheckable(True),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self._sendTouchEvent(1, avg.CURSORDOWN, 0, 0),
                 lambda: self._sendTouchEvent(2, avg.CURSORDOWN, 0, 0),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 lambda: self._sendTouchEvent(1, avg.CURSORUP, 0, 0),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self._sendTouchEvent(2, avg.CURSORUP, 0, 0),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.assert_(self.__down and self.__clicked),
                 reset,
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self._sendTouchEvent(1, avg.CURSORDOWN, 0, 0),
                 lambda: self._sendTouchEvent(2, avg.CURSORDOWN, 0, 0),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 lambda: self._sendTouchEvent(1, avg.CURSORUP, 0, 0),
                 lambda: self._sendTouchEvent(2, avg.CURSORUP, 0, 0),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self.assert_(self.__down and self.__clicked),
                 reset,

                 # not checked, 2 down, 2 out, 2 in, first up, second up 
                 # ==> pressed, clicked
                 lambda: b.setChecked(False),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self._sendTouchEvent(1, avg.CURSORDOWN, 0, 0),
                 lambda: self._sendTouchEvent(2, avg.CURSORDOWN, 0, 0),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 lambda: self._sendTouchEvent(1, avg.CURSORMOTION, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self._sendTouchEvent(2, avg.CURSORMOTION, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self._sendTouchEvent(1, avg.CURSORMOTION, 0, 0),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self._sendTouchEvent(2, avg.CURSORMOTION, 0, 0),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self._sendTouchEvent(1, avg.CURSORUP, 0, 0),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 lambda: self._sendTouchEvent(2, avg.CURSORUP, 0, 0),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.assert_(self.__down and self.__clicked),
                 reset,

                 # checked, 2 down, 2 out, 2 in, first up, second up 
                 # ==> pressed, clicked
                 lambda: b.setCheckable(True),
                 lambda: b.setChecked(True),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self._sendTouchEvent(1, avg.CURSORDOWN, 0, 0),
                 lambda: self._sendTouchEvent(2, avg.CURSORDOWN, 0, 0),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 lambda: self._sendTouchEvent(1, avg.CURSORMOTION, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self._sendTouchEvent(2, avg.CURSORMOTION, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self._sendTouchEvent(1, avg.CURSORMOTION, 0, 0),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self._sendTouchEvent(2, avg.CURSORMOTION, 0, 0),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self._sendTouchEvent(1, avg.CURSORUP, 0, 0),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 lambda: self._sendTouchEvent(2, avg.CURSORUP, 0, 0),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self.assert_(self.__down and self.__clicked),
                 reset,

                 # not checked, 2 down, 2 out, first up, second up 
                 # ==> pressed, not clicked
                 lambda: b.setCheckable(True),
                 lambda: b.setChecked(False),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self._sendTouchEvent(1, avg.CURSORDOWN, 0, 0),
                 lambda: self._sendTouchEvent(2, avg.CURSORDOWN, 0, 0),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 lambda: self._sendTouchEvent(1, avg.CURSORMOTION, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self._sendTouchEvent(2, avg.CURSORMOTION, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self._sendTouchEvent(1, avg.CURSORUP, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 lambda: self._sendTouchEvent(2, avg.CURSORUP, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonUp", False),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 reset,

                 # checked, 2 down, 2 out, first up, second up 
                 # ==> pressed, not clicked
                 lambda: b.setCheckable(True),
                 lambda: b.setChecked(True),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self._sendTouchEvent(1, avg.CURSORDOWN, 0, 0),
                 lambda: self._sendTouchEvent(2, avg.CURSORDOWN, 0, 0),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 lambda: self._sendTouchEvent(1, avg.CURSORMOTION, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self._sendTouchEvent(2, avg.CURSORMOTION, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self._sendTouchEvent(1, avg.CURSORUP, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonDown", False),
                 lambda: self.assert_(self.__down and not(self.__clicked)),
                 lambda: self._sendTouchEvent(2, avg.CURSORUP, 0, yOutDistance),
                 lambda: self.compareImage("testUIButtonDown", False),
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
            self.start((
                     # Standard down->up
                     lambda: self._sendTouchEvent(1, avg.CURSORDOWN, 0, 0),
                     lambda: self.assert_(not(self.clicked)),
                     lambda: self.compareImage("testUIButtonDown", False),
                     lambda: self._sendTouchEvent(1, avg.CURSORUP, 0, 0),
                     lambda: self.assert_(self.clicked),
                     lambda: self.compareImage("testUIButtonUp", False),

                     # Disable, down, up -> no click
                     reset,
                     lambda: self.assert_(button.enabled),
                     lambda: enable(False),
                     lambda: self.assert_(not(button.enabled)),
                     lambda: self.compareImage("testUIButtonDisabled", False),
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
                     lambda: self.compareImage("testUIButtonUp", False),

                     # Down, move further away, up -> no click
                     reset,
                     lambda: self._sendTouchEvent(3, avg.CURSORDOWN, 0, 0),
                     lambda: self._sendTouchEvent(3, avg.CURSORMOTION, 100, 0),
                     lambda: self._sendTouchEvent(3, avg.CURSORUP, 100, 0),
                     lambda: self.assert_(not(self.clicked)),
                     lambda: self.compareImage("testUIButtonUp", False),

                     # Test if button still reacts after abort
                     lambda: self._sendTouchEvent(4, avg.CURSORDOWN, 0, 0),
                     lambda: self.assert_(not(self.clicked)),
                     lambda: self.compareImage("testUIButtonDown", False),
                     lambda: self._sendTouchEvent(4, avg.CURSORUP, 0, 0),
                     lambda: self.assert_(self.clicked),
                     lambda: self.compareImage("testUIButtonUp", False),
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
            self.start((
                     reset,
                     lambda: self.compareImage("testUIToggleUnchecked_Up", False),
                     lambda: self._sendTouchEvent(1, avg.CURSORDOWN, 0, 0),
                     lambda: self.assert_(not(self.checked)),
                     lambda: self.assert_(not(self.unchecked)),
                     lambda: self.compareImage("testUIToggleUnchecked_Down", False),
                     lambda: self._sendTouchEvent(1, avg.CURSORUP, 0, 0),
                     lambda: self.assert_(self.checked),
                     lambda: self.compareImage("testUIToggleChecked_Up", False),
                     lambda: self._sendTouchEvent(2, avg.CURSORDOWN, 0, 0),
                     lambda: self.compareImage("testUIToggleChecked_Down", False),
                     lambda: self._sendTouchEvent(2, avg.CURSORUP, 0, 0),
                     lambda: self.assert_(self.unchecked),
                     lambda: self.compareImage("testUIToggleUnchecked_Up", False),
                    ))

        def testToggleAbort():
            self.start((
                    reset,
                    lambda: self.compareImage("testUIToggleUnchecked_Up", False),
                    lambda: self._sendTouchEvent(1, avg.CURSORDOWN, 0, 0),
                    lambda: self.compareImage("testUIToggleUnchecked_Down", False),
                    lambda: self._sendTouchEvent(1, avg.CURSORUP, 100, 0),
                    lambda: self.assert_(not (self.checked)),
                    lambda: self.compareImage("testUIToggleUnchecked_Up", False),
                    lambda: button.setChecked(True),
                    lambda: self.compareImage("testUIToggleChecked_Up", False),
                    lambda: self._sendTouchEvent(2, avg.CURSORDOWN, 0, 0),
                    lambda: self.compareImage("testUIToggleChecked_Down", False),
                    lambda: self._sendTouchEvent(2, avg.CURSORUP, 100, 0),
                    lambda: self.assert_(not(self.unchecked)),
                    lambda: self.compareImage("testUIToggleChecked_Up", False),
                    ))

        def testToggleDisable():
            self.start((
                    reset,
                    lambda: self.compareImage("testUIToggleUnchecked_Up", False),
                    lambda: button.setEnabled(False),
                    lambda: self.compareImage("testUIToggleUnchecked_Disabled", False),
                    lambda: self._sendTouchEvent(1, avg.CURSORDOWN, 0, 0),
                    lambda: self._sendTouchEvent(1, avg.CURSORUP, 0, 0),
                    lambda: self.compareImage("testUIToggleUnchecked_Disabled", False),
                    lambda: button.setEnabled(True),
                    lambda: self.compareImage("testUIToggleUnchecked_Up", False),
                    lambda: self._sendTouchEvent(2, avg.CURSORDOWN, 0, 0),
                    lambda: button.setEnabled(False),
                    lambda: self.assert_(not (self.checked)),
                    lambda: self.compareImage("testUIToggleUnchecked_Disabled", False),
                    
                    lambda: button.setEnabled(True),
                    reset,
                    lambda: self.compareImage("testUIToggleUnchecked_Up", False),
                    lambda: button.setChecked(True),
                    lambda: self.compareImage("testUIToggleChecked_Up", False),
                    lambda: button.setEnabled(False),
                    lambda: self.compareImage("testUIToggleChecked_Disabled", False),
                    lambda: self._sendTouchEvent(3, avg.CURSORDOWN, 0, 0),
                    lambda: self._sendTouchEvent(3, avg.CURSORUP, 0, 0),
                    lambda: self.compareImage("testUIToggleChecked_Disabled", False),
                    lambda: button.setEnabled(True),
                    lambda: self.compareImage("testUIToggleChecked_Up", False),
                    lambda: self._sendTouchEvent(4, avg.CURSORDOWN, 0, 0),
                    lambda: button.setEnabled(False),
                    lambda: self.assert_(not (self.unchecked)),
                    lambda: self.compareImage("testUIToggleChecked_Disabled", False),
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

        self.start((
                lambda: self.compareImage("testScrollPane1", False),
                scrollLarge,
                lambda: self.compareImage("testScrollPane2", False),
                initSmallContent,
                lambda: self.compareImage("testScrollPane3", False),
                scrollSmall,
                lambda: self.compareImage("testScrollPane4", False),
                ))


def uiTestSuite(tests):
    availableTests = (
        "testKeyboard",
        "testTextArea",
        "testFocusContext",
        "testTapRecognizer",
        "testHoldRecognizer",
        "testDoubletapRecognizer",
        "testDragRecognizer",
        "testDragRecognizerRelCoords",
        "testDragRecognizerInitialEvent",
        "testDragRecognizerCoordSysNode",
        "testTransformRecognizer",
        "testKMeans",
        "testMat3x3",
        "testButton",
        "testMultitouchButton",
        "testTouchButton",
        "testToggleButton",
        "testScrollPane"
        )

    return createAVGTestSuite(availableTests, UITestCase, tests)

Player = avg.Player.get()

