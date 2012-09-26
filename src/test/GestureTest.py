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

from libavg import avg, ui, player

import math
from testcase import *

class GestureTestCase(AVGTestCase):
    
    def __init__(self, testFuncName):
        AVGTestCase.__init__(self, testFuncName)

    def testTapRecognizer(self):

        def abort():
            self.__tapRecognizer.abort()

        def enable(isEnabled):
            self.__tapRecognizer.enable(isEnabled)

        self.__initImageScene()
        self.__tapRecognizer = ui.TapRecognizer(self.image)
        self.messageTester = MessageTester(self.__tapRecognizer, [ui.Recognizer.POSSIBLE, 
                ui.Recognizer.DETECTED, ui.Recognizer.FAILED], self)
        player.setFakeFPS(10)
        self.start(False,
                (# Down-up: recognized as tap.
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30,
                        [ui.Recognizer.POSSIBLE]),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, 
                        [ui.Recognizer.DETECTED]),
                 # Down-small move-up: recognized as tap.
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30,
                        [ui.Recognizer.POSSIBLE]),
                 self._genMouseEventFrames(avg.Event.CURSOR_MOTION, 31, 30, []),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, 
                        [ui.Recognizer.DETECTED]),
                 # Down-big move-up: fail
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30,
                        [ui.Recognizer.POSSIBLE]),
                 self._genMouseEventFrames(avg.Event.CURSOR_MOTION, 80, 80, 
                        [ui.Recognizer.FAILED]),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),
                 # Down-Abort-Up: not recognized as tap
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                        [ui.Recognizer.POSSIBLE]),
                 abort,
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),
                 # Abort-Down-Up: recognized as tap
                 abort,
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                        [ui.Recognizer.POSSIBLE]),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, 
                        [ui.Recognizer.DETECTED]),
                 # Down-Abort-Up-Down-Up: recognized as tap
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                        [ui.Recognizer.POSSIBLE]),
                 abort,
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                        [ui.Recognizer.POSSIBLE]),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, 
                        [ui.Recognizer.DETECTED]),
                 # Disable-Down-Up-Enable: not recognized as tap
                 lambda: enable(False),
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, []),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),
                 lambda: enable(True),
                 # Down-Disable-Enable-Up: not recognized as tap
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                        [ui.Recognizer.POSSIBLE]),
                 lambda: enable(False),
                 lambda: enable(True),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),
                 # Down-Disable-Up-Enable-Down-Up: recognized as tap
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                        [ui.Recognizer.POSSIBLE]),
                 lambda: enable(False),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),
                 lambda: enable(True),
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                        [ui.Recognizer.POSSIBLE]),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, 
                        [ui.Recognizer.DETECTED]),
                 # Abort-Disable-Abort-Enable-Abort-Down-Up: recognized as tap
                 abort,
                 lambda: enable(False),
                 abort,
                 lambda: enable(True),
                 abort,
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                        [ui.Recognizer.POSSIBLE]),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, 
                        [ui.Recognizer.DETECTED]),

                 # Remove node while tap is in progress.
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                        [ui.Recognizer.POSSIBLE]),
                 self.__killImageNode,
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),
                ))


    def testHoldRecognizer(self):

        def abort():
            self.__holdRecognizer.abort()
            self.messageTester.reset()

        def enable(isEnabled):
            self.__holdRecognizer.enable(isEnabled)
            self.messageTester.reset()

        player.setFakeFPS(20)
        self.__initImageScene()
        self.__holdRecognizer = ui.HoldRecognizer(self.image,
                delay=1000)
        self.messageTester = MessageTester(self.__holdRecognizer, 
                [ui.Recognizer.POSSIBLE, ui.Recognizer.DETECTED, ui.Recognizer.FAILED, 
                ui.Recognizer.END], self)
        self.start(False,
                (# Standard down-hold-up sequence.
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                        [ui.Recognizer.POSSIBLE]),
                 lambda: self.delay(1100),
                 lambda: self.messageTester.assertState([ui.Recognizer.DETECTED]),
                 self.messageTester.reset,
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30,
                        [ui.Recognizer.END]),

                 # down-up sequence, hold not long enough.
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                        [ui.Recognizer.POSSIBLE]),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30,
                        [ui.Recognizer.FAILED]),

                 # down-move-up sequence, should fail. 
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 1, 1, 
                        [ui.Recognizer.POSSIBLE]),
                 self._genMouseEventFrames(avg.Event.CURSOR_MOTION, 150, 50, 
                        [ui.Recognizer.FAILED]),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),

                 # down-hold-abort-up, should be recognized, no end event.
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                        [ui.Recognizer.POSSIBLE]),
                 lambda: self.delay(1100),
                 lambda: self.messageTester.assertState([ui.Recognizer.DETECTED]),
                 abort,
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),

                 # down-abort-hold-up, should not be recognized
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                        [ui.Recognizer.POSSIBLE]),
                 abort,
                 lambda: self.delay(1100),
                 lambda: self.messageTester.assertState([]),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),

                 # down-hold-disabled-up-enabled, should be recognized, no end event.
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                        [ui.Recognizer.POSSIBLE]),
                 lambda: self.delay(1100),
                 lambda: self.messageTester.assertState([ui.Recognizer.DETECTED]),
                 lambda: enable(False),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),
                 lambda: enable(True),

                 # down-disabled-enabled-hold-up, should not be recognized
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                        [ui.Recognizer.POSSIBLE]),
                 lambda: enable(False),
                 lambda: enable(True),
                 lambda: self.delay(1100),
                 lambda: self.messageTester.assertState([]),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),
                 
                 # Remove node while hold is in progress.
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                        [ui.Recognizer.POSSIBLE]),
                 self.__killImageNode,
                 self.delay(1100),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),
                ))
        player.setFakeFPS(-1)


    def testDoubletapRecognizer(self):

        def abort():
            self.__tapRecognizer.abort()
            self.messageTester.reset()

        def enable(isEnabled):
            self.__tapRecognizer.enable(isEnabled)
            self.messageTester.reset()

        root = self.loadEmptyScene()
        image = avg.ImageNode(parent=root, href="rgb24-64x64.png", size=(128,128))
        self.__tapRecognizer = ui.DoubletapRecognizer(image)
        self.messageTester = MessageTester(self.__tapRecognizer, 
                [ui.Recognizer.POSSIBLE, ui.Recognizer.DETECTED, ui.Recognizer.FAILED, 
                ui.Recognizer.END], self)
        player.setFakeFPS(20)
        self.start(False,
                (# Down, up, down, up: click
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                        [ui.Recognizer.POSSIBLE]),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, []),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30,
                        [ui.Recognizer.DETECTED]),
                 # Down, move: stop
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 0, 30,
                        [ui.Recognizer.POSSIBLE]),
                 self._genMouseEventFrames(avg.Event.CURSOR_MOTION, 80, 30, 
                        [ui.Recognizer.FAILED]),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 0, 30, []),
                 # Down, up, move: stop
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 0, 30, 
                        [ui.Recognizer.POSSIBLE]),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 0, 30, []),
                 self._genMouseEventFrames(avg.Event.CURSOR_MOTION, 80, 30, []),
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 80, 30, 
                        [ui.Recognizer.FAILED]),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 0, 30, []),
                 # Down, up, down, move: stop
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 0, 30,
                        [ui.Recognizer.POSSIBLE]),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 0, 30, []),
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 0, 30, []),
                 self._genMouseEventFrames(avg.Event.CURSOR_MOTION, 80, 30,
                        [ui.Recognizer.FAILED]),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 0, 30, []),
                 # Down,delay: stop
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                        [ui.Recognizer.POSSIBLE]),
                 lambda: self.delay(1000),
                 lambda: self.messageTester.assertState([ui.Recognizer.FAILED]),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30,
                        [ui.Recognizer.FAILED]),
                 # Down, up, delay: stop
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                        [ui.Recognizer.POSSIBLE]),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),
                 lambda: self.delay(1000),
                 lambda: self.messageTester.assertState([ui.Recognizer.FAILED]),
                 # Down, up, down, delay: stop
                 self.messageTester.reset,
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30,
                        [ui.Recognizer.POSSIBLE]),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, []),
                 lambda: self.delay(1000),
                 lambda: self.messageTester.assertState([ui.Recognizer.FAILED]),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, 
                        [ui.Recognizer.FAILED]),
                 # Down, abort, up, down, up, delay: just one click
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30,
                        [ui.Recognizer.POSSIBLE]),
                 abort,
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30,
                        [ui.Recognizer.POSSIBLE]),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),
                 lambda: self.delay(1000),
                 lambda: self.messageTester.assertState([ui.Recognizer.FAILED]),
                 # Down, up, abort, down, up, delay: two clicks but no double-click
                 self.messageTester.reset,
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                        [ui.Recognizer.POSSIBLE]),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),
                 abort,
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                        [ui.Recognizer.POSSIBLE]),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),
                 lambda: self.delay(1000),
                 lambda: self.messageTester.assertState([ui.Recognizer.FAILED]),
                 # Down, up, down, abort, up: just one click
                 self.messageTester.reset,
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                        [ui.Recognizer.POSSIBLE]),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, []),
                 abort,
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),
                 # Down, abort, up, down, up, down up: first aborted then recognized
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                        [ui.Recognizer.POSSIBLE]),
                 abort,
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30,
                        [ui.Recognizer.POSSIBLE]),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, []),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30,
                        [ui.Recognizer.DETECTED]),
                 # Disabled, down, up, down, up, enabled: nothing
                 lambda: enable(False),
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, []),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, []),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),
                 lambda: enable(True),                 
                 # Down, disabled up, down, up, enabled: just one down
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                        [ui.Recognizer.POSSIBLE]),
                 lambda: enable(False),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, []),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),
                 lambda: enable(True),
                 # Down, up, disabled, down, up, enabled: just one click
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                        [ui.Recognizer.POSSIBLE]),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),
                 lambda: enable(False),
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, []),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),
                 lambda: enable(True),
                 # Down, up, down, disabled, up, enabled: just one click
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                        [ui.Recognizer.POSSIBLE]),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, []),
                 lambda: enable(False),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),
                 lambda: enable(True),
                 # Down, disabled, enabled, up, down, up, down, up: recognized
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                        [ui.Recognizer.POSSIBLE]),
                 lambda: enable(False),
                 lambda: enable(True),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                        [ui.Recognizer.POSSIBLE]),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, []),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, 
                        [ui.Recognizer.DETECTED]),
                ))


    def testDragRecognizer(self):

        def onMove(offset):
            if self.friction == -1:
                self.assertEqual(offset, (40,40))
            self.messageTester.setMessageReceived(ui.Recognizer.MOTION)

        def onUp(offset):
            if self.friction == -1:
                self.assertEqual(offset, (10,-10))
            self.messageTester.setMessageReceived(ui.Recognizer.UP)

        def enable(isEnabled):
            dragRecognizer.enable(isEnabled)
            self.messageTester.reset()

        def abort():
            dragRecognizer.abort()
            self.messageTester.reset()

        player.setFakeFPS(100)
        sys.stderr.write("\n")
        for self.friction in (-1, 100):
            if self.friction == -1:
                sys.stderr.write("  Simple drag, no inertia\n")
            else:
                sys.stderr.write("  Simple drag, inertia\n")
            self.__initImageScene()
            dragRecognizer = ui.DragRecognizer(self.image, moveHandler=onMove,
                    upHandler=onUp, minDragDist=0, friction=self.friction)
            self.messageTester = MessageTester(dragRecognizer, 
                    [ui.Recognizer.DETECTED, ui.Recognizer.END], self)
            self.start(False,
                    (self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30,
                            [ui.Recognizer.DETECTED]),
                     self._genMouseEventFrames(avg.Event.CURSOR_MOTION, 70, 70, 
                            [ui.Recognizer.MOTION]),
                     self._genMouseEventFrames(avg.Event.CURSOR_UP, 40, 20, 
                            [ui.Recognizer.UP, ui.Recognizer.END]),
                     lambda: enable(False),
                     self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, []),
                     lambda: dragRecognizer.enable(True),
                     self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),
                     self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                            [ui.Recognizer.DETECTED]),
                     self._genMouseEventFrames(avg.Event.CURSOR_UP, 40, 20, 
                            [ui.Recognizer.UP, ui.Recognizer.END]),

                     # Remove node during drag.
                     self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                            [ui.Recognizer.DETECTED]),
                     self.__killImageNode,
                     self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),
                    ))

        # Test with constraint.
        def onVertMove(offset):
            if self.friction == -1:
                self.assertEqual(offset, (0,40))
            self.messageTester.setMessageReceived(ui.Recognizer.MOTION)

        for self.friction in (-1, 100):
            if self.friction == -1:
                sys.stderr.write("  Drag with constraint, no inertia\n")
            else:
                sys.stderr.write("  Drag with constraint, inertia\n")
            self.__initImageScene()
            dragRecognizer = ui.DragRecognizer(self.image, moveHandler=onVertMove, 
                    upHandler=onUp, friction=self.friction, 
                    direction=ui.DragRecognizer.VERTICAL)
            self.messageTester = MessageTester(dragRecognizer, [ui.Recognizer.POSSIBLE, 
                    ui.Recognizer.DETECTED, ui.Recognizer.FAILED, ui.Recognizer.END], 
                    self)
            self.messageTester.reset()
            self.start(False,
                    (self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                            [ui.Recognizer.POSSIBLE]),
                     self._genMouseEventFrames(avg.Event.CURSOR_MOTION, 35, 30, []),
                     self._genMouseEventFrames(avg.Event.CURSOR_MOTION, 30, 70, 
                            [ui.Recognizer.DETECTED, ui.Recognizer.MOTION]),
                     self._genMouseEventFrames(avg.Event.CURSOR_UP, 40, 20, 
                            [ui.Recognizer.UP, ui.Recognizer.END]),
                     # Wrong direction -> stop.
                     self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                            [ui.Recognizer.POSSIBLE]),
                     self._genMouseEventFrames(avg.Event.CURSOR_MOTION, 70, 30, 
                            [ui.Recognizer.FAILED]),
                     self._genMouseEventFrames(avg.Event.CURSOR_UP, 70, 30, []),

                     # No movement -> stop.
                     self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                            [ui.Recognizer.POSSIBLE]),
                     self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, 
                            [ui.Recognizer.FAILED]),

                     # Down, Abort, Motion, Motion, Up -> not recognized
                     self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                            [ui.Recognizer.POSSIBLE]),
                     abort,
                     self._genMouseEventFrames(avg.Event.CURSOR_MOTION, 35, 30, []),
                     self._genMouseEventFrames(avg.Event.CURSOR_MOTION, 30, 70, []),
                     self._genMouseEventFrames(avg.Event.CURSOR_UP, 40, 20, []),

                     # Down, Motion, Abort, Motion, Up -> not Recognized
                     self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                            [ui.Recognizer.POSSIBLE]),
                     self._genMouseEventFrames(avg.Event.CURSOR_MOTION, 35, 30, []),
                     abort,
                     self._genMouseEventFrames(avg.Event.CURSOR_MOTION, 30, 70, []),
                     self._genMouseEventFrames(avg.Event.CURSOR_UP, 40, 20, []),

                     # Down, Motion, Motion, Abort, Up -> not recognized
                     self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                            [ui.Recognizer.POSSIBLE]),
                     self._genMouseEventFrames(avg.Event.CURSOR_MOTION, 35, 30, []),
                     self._genMouseEventFrames(avg.Event.CURSOR_MOTION, 30, 70,
                            [ui.Recognizer.DETECTED, ui.Recognizer.MOTION]),
                     abort,
                     self._genMouseEventFrames(avg.Event.CURSOR_UP, 40, 20, []),

                     # Down, Motion, Abort, Up, Down, Motion, Motion, Up -> Recognized
                     self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                            [ui.Recognizer.POSSIBLE]),
                     self._genMouseEventFrames(avg.Event.CURSOR_MOTION, 35, 30, []),
                     abort,
                     self._genMouseEventFrames(avg.Event.CURSOR_UP, 40, 20, []),
                     
                     self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                            [ui.Recognizer.POSSIBLE]),
                     self._genMouseEventFrames(avg.Event.CURSOR_MOTION, 35, 30, []),
                     self._genMouseEventFrames(avg.Event.CURSOR_MOTION, 30, 70,
                            [ui.Recognizer.DETECTED, ui.Recognizer.MOTION]),
                     self._genMouseEventFrames(avg.Event.CURSOR_UP, 40, 20, 
                            [ui.Recognizer.UP, ui.Recognizer.END]),
                    ))

        # Test second down during inertia.
        sys.stderr.write("  Down during inertia\n")
        self.__initImageScene()
        dragRecognizer = ui.DragRecognizer(self.image, moveHandler=onMove, upHandler=onUp,
                minDragDist=0, friction=0.01)
        self.messageTester = MessageTester(dragRecognizer, [ui.Recognizer.POSSIBLE, 
                ui.Recognizer.DETECTED, ui.Recognizer.FAILED, ui.Recognizer.END], 
                self)
        self.start(False,
                (lambda: self._sendMouseEvent(avg.Event.CURSOR_DOWN, 30, 30),
                 lambda: self._sendMouseEvent(avg.Event.CURSOR_UP, 40, 20),
                 self.messageTester.reset,
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 40, 20, 
                            [ui.Recognizer.END, ui.Recognizer.DETECTED, 
                             ui.Recognizer.MOTION]),
                ))

        # Test node delete during inertia
        sys.stderr.write("  Delete during inertia\n")
        self.__initImageScene()
        dragRecognizer = ui.DragRecognizer(self.image, moveHandler=onMove, upHandler=onUp,
                minDragDist=0, friction=0.01)
        self.messageTester = MessageTester(dragRecognizer, [ui.Recognizer.POSSIBLE, 
                ui.Recognizer.DETECTED, ui.Recognizer.FAILED, ui.Recognizer.END], 
                self)
        self.start(False,
                (self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                        [ui.Recognizer.DETECTED]),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 40, 20, 
                        [ui.Recognizer.MOTION, ui.Recognizer.UP]),
                 self.__killImageNode,
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 40, 20,
                        [ui.Recognizer.MOTION]),
                ))

        # Test second down during inertia, constrained recognizer
        sys.stderr.write("  Down during inertia, constrained recognizer\n")
        self.__initImageScene()
        dragRecognizer = ui.DragRecognizer(self.image, moveHandler=onMove, upHandler=onUp,
                friction=0.01, direction=ui.DragRecognizer.VERTICAL)
        self.messageTester = MessageTester(dragRecognizer, [ui.Recognizer.POSSIBLE, 
                ui.Recognizer.DETECTED, ui.Recognizer.FAILED, ui.Recognizer.END], 
                self)
        self.start(False,
                (lambda: self._sendMouseEvent(avg.Event.CURSOR_DOWN, 30, 30),
                 self._genMouseEventFrames(avg.Event.CURSOR_MOTION, 30, 70,
                        [ui.Recognizer.DETECTED, ui.Recognizer.MOTION, 
                         ui.Recognizer.POSSIBLE]),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 70,
                        [ui.Recognizer.MOTION, ui.Recognizer.UP]),
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                        [ui.Recognizer.MOTION, ui.Recognizer.END, 
                         ui.Recognizer.POSSIBLE]),
                 self._genMouseEventFrames(avg.Event.CURSOR_MOTION, 30, 70, 
                        [ui.Recognizer.DETECTED, ui.Recognizer.MOTION]),
                ))

        player.setFakeFPS(-1)


    def testDragRecognizerRelCoords(self):

        def onDrag(offset):
            self.assertAlmostEqual(offset, (-40,-40))

        player.setFakeFPS(100)
        for self.friction in (-1, 100):
            root = self.loadEmptyScene()
            div = avg.DivNode(pos=(64,64), angle=math.pi, parent=root)
            image = avg.ImageNode(parent=div, href="rgb24-64x64.png")
            ui.DragRecognizer(image, moveHandler=onDrag, friction=self.friction)
            self.start(False,
                    (lambda: self._sendMouseEvent(avg.Event.CURSOR_DOWN, 30, 30),
                     lambda: self._sendMouseEvent(avg.Event.CURSOR_MOTION, 70, 70),
                    ))
        player.setFakeFPS(-1)


    def testDragRecognizerInitialEvent(self):

        def onMotion(offset):
            ui.DragRecognizer(self.image, 
                    detectedHandler=onDragStart, moveHandler=onDrag, 
                    initialEvent=player.getCurrentEvent())
            self.image.unsubscribe(avg.Node.CURSOR_MOTION, onMotion)

        def onDragStart():
            self.__dragStartCalled = True

        def onDrag(offset):
            self.assertEqual(offset, (10,0))

        self.__initImageScene()
        self.image.subscribe(avg.Node.CURSOR_MOTION, onMotion)
        self.__dragStartCalled = False
        self.start(False,
                (lambda: self._sendMouseEvent(avg.Event.CURSOR_DOWN, 30, 30),
                 lambda: self._sendMouseEvent(avg.Event.CURSOR_MOTION, 40, 30),
                 lambda: self._sendMouseEvent(avg.Event.CURSOR_MOTION, 50, 30),
                ))
        assert(self.__dragStartCalled)


    def testDragRecognizerCoordSysNode(self):

        def onDrag(offset):
            self.assertEqual(offset, (40,40))

        root = self.loadEmptyScene()
        div = avg.DivNode(pos=(64,64), angle=math.pi, parent=root)
        image = avg.ImageNode(parent=div, href="rgb24-64x64.png")
        ui.DragRecognizer(image, moveHandler=onDrag, coordSysNode=div, friction=-1)
        self.start(False,
                (lambda: self._sendMouseEvent(avg.Event.CURSOR_DOWN, 30, 30),
                 lambda: self._sendMouseEvent(avg.Event.CURSOR_MOTION, 70, 70),
                ))

    def testDragRecognizerMinDist(self):

        def onMove(offset):
            self.messageTester.setMessageReceived(ui.Recognizer.MOTION)

        self.__initImageScene()
        dragRecognizer = ui.DragRecognizer(self.image, moveHandler=onMove, minDragDist=10,
                friction=-1)
        self.messageTester = MessageTester(dragRecognizer, [ui.Recognizer.DETECTED], 
                self)
        self.start(False,
                (lambda: self._sendMouseEvent(avg.Event.CURSOR_DOWN, 30, 30),
                 self._genMouseEventFrames(avg.Event.CURSOR_MOTION, 30, 35,
                        []),
                 self._genMouseEventFrames(avg.Event.CURSOR_MOTION, 30, 50,
                        [ui.Recognizer.DETECTED, ui.Recognizer.MOTION]),
                ))


    def testTransformRecognizer(self):

        def onDetected():
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
                    lambda: self._sendTouchEvent(1, avg.Event.CURSOR_DOWN, 10, 10),
                    lambda: self._sendTouchEvent(1, avg.Event.CURSOR_UP, 20, 10),
                    lambda: checkTransform(ui.Transform((10,0))),
                   )

        def createRotTestFrames(expectedTransform):
            return (
                    lambda: self._sendTouchEvents((
                            (1, avg.Event.CURSOR_DOWN, 0, 10),
                            (2, avg.Event.CURSOR_DOWN, 0, 20))),
                    lambda: self._sendTouchEvents((
                            (1, avg.Event.CURSOR_MOTION, 0, 20),
                            (2, avg.Event.CURSOR_MOTION, 0, 10))),
                    lambda: checkTransform(expectedTransform),
                    lambda: self._sendTouchEvents((
                            (1, avg.Event.CURSOR_UP, 0, 20),
                            (2, avg.Event.CURSOR_UP, 0, 10))),
                   )

        def createScaleTestFrames(expectedTransform):
            return (
                 lambda: self._sendTouchEvent(1, avg.Event.CURSOR_DOWN, 0, 10),
                 lambda: self._sendTouchEvent(2, avg.Event.CURSOR_DOWN, 0, 20),
                 lambda: self._sendTouchEvent(1, avg.Event.CURSOR_MOTION, 0, 10),
                 lambda: self._sendTouchEvent(2, avg.Event.CURSOR_MOTION, 0, 30),
                 lambda: checkTransform(expectedTransform),
                 lambda: self._sendTouchEvent(1, avg.Event.CURSOR_UP, 0, 10),
                 lambda: self._sendTouchEvent(2, avg.Event.CURSOR_UP, 0, 30),
                )

        self.__initImageScene()
        # Turn off the jitter filter.
        ui.TransformRecognizer.FILTER_MIN_CUTOFF = None
        ui.TransformRecognizer.FILTER_BETA = None
        
        self.__transformRecognizer = ui.TransformRecognizer(self.image,
                friction=-1,
                detectedHandler=onDetected, moveHandler=onMove, upHandler=onUp)
        self.start(False,
                (# Check up/down handling
                 lambda: self._sendTouchEvent(1, avg.Event.CURSOR_DOWN, 10, 10),
                 lambda: checkTransform(ui.Transform((0,0))),
                 lambda: self._sendTouchEvent(1, avg.Event.CURSOR_MOTION, 20, 10),
                 lambda: checkTransform(ui.Transform((10,0))),
                 lambda: self._sendTouchEvent(2, avg.Event.CURSOR_DOWN, 20, 20),
                 lambda: checkTransform(ui.Transform((0,0))),
                 lambda: self._sendTouchEvents((
                        (1, avg.Event.CURSOR_MOTION, 30, 10),
                        (2, avg.Event.CURSOR_MOTION, 30, 20))),
                 lambda: checkTransform(ui.Transform((10,0))),
                 lambda: self._sendTouchEvent(2, avg.Event.CURSOR_UP, 30, 20),
                 lambda: checkTransform(ui.Transform((0,0))),
                 lambda: self._sendTouchEvent(1, avg.Event.CURSOR_MOTION, 40, 10),
                 lambda: checkTransform(ui.Transform((10,0))),
                 lambda: self._sendTouchEvent(1, avg.Event.CURSOR_UP, 50, 10),
                 lambda: checkTransform(ui.Transform((10,0))),

                 createRotTestFrames(ui.Transform((0,0), math.pi, 1, (0,15))),

                 createScaleTestFrames(ui.Transform((0,5), 0, 2, (0,20))),

                 # Delete node during transform
                 lambda: self._sendTouchEvents((
                        (1, avg.Event.CURSOR_DOWN, 30, 10),
                        (2, avg.Event.CURSOR_DOWN, 30, 20))),
                 self.__killImageNode,
                 lambda: self._sendTouchEvents((
                        (1, avg.Event.CURSOR_UP, 30, 10),
                        (2, avg.Event.CURSOR_UP, 30, 20))),
                ))

        # Test rel. coords.
        root = self.loadEmptyScene()
        div = avg.DivNode(parent=root, pos=(0,10))
        image = avg.ImageNode(parent=div, href="rgb24-64x64.png")
        self.__transformRecognizer = ui.TransformRecognizer(image, friction=-1, 
                detectedHandler=onDetected, moveHandler=onMove, upHandler=onUp)
        self.start(False,
                (createTransTestFrames(),
                 createRotTestFrames(ui.Transform((0,0), math.pi, 1, (0,5))),
                 createScaleTestFrames(ui.Transform((0,5), 0, 2, (0,10))),
                ))

        # Test coordSysNode.
        root = self.loadEmptyScene()
        div = avg.DivNode(parent=root, pos=(0,10))
        image = avg.ImageNode(parent=div, href="rgb24-64x64.png")
        self.__transformRecognizer = ui.TransformRecognizer(image, coordSysNode=div,
                friction=-1,
                detectedHandler=onDetected, moveHandler=onMove, upHandler=onUp)
        self.start(False,
                (createTransTestFrames(),
                 createRotTestFrames(ui.Transform((0,0), math.pi, 1, (0,15))),
                 createScaleTestFrames(ui.Transform((0,5), 0, 2, (0,20))),
                ))

        # Test friction
        root = self.loadEmptyScene()
        div = avg.DivNode(parent=root, pos=(0,10))
        image = avg.ImageNode(parent=div, href="rgb24-64x64.png")
        self.__transformRecognizer = ui.TransformRecognizer(image, friction=0.01,
                detectedHandler=onDetected, moveHandler=onMove, upHandler=onUp)
        self.start(False,
                (lambda: self._sendTouchEvent(1, avg.Event.CURSOR_DOWN, 10, 10),
                 lambda: self._sendTouchEvent(1, avg.Event.CURSOR_UP, 20, 10),
                 lambda: self._sendTouchEvent(1, avg.Event.CURSOR_DOWN, 10, 10),
                 lambda: self._sendTouchEvent(1, avg.Event.CURSOR_UP, 20, 10),
                ))  

        # Test abort
        self.__initImageScene()
        self.__transformRecognizer = ui.TransformRecognizer(self.image, 
                detectedHandler=onDetected, moveHandler=onMove, upHandler=onUp)
        self.start(False,
                (self.__transformRecognizer.abort,
                 lambda: self._sendTouchEvent(1, avg.Event.CURSOR_DOWN, 10, 10),
                 self.__transformRecognizer.abort,
                 lambda: self._sendTouchEvent(1, avg.Event.CURSOR_MOTION, 20, 10),
                 lambda: self._sendTouchEvent(1, avg.Event.CURSOR_UP, 30, 10),
                 lambda: checkTransform(ui.Transform((0,0))),
                 self.__transformRecognizer.abort,
                ))

        # Test enable/disable
        self.__initImageScene()
        self.__transformRecognizer = ui.TransformRecognizer(self.image, friction=-1,
                detectedHandler=onDetected, moveHandler=onMove, upHandler=onUp)
        self.start(False,
                (# Regular disable
                 lambda: self.__transformRecognizer.enable(False),
                 lambda: self._sendTouchEvent(1, avg.Event.CURSOR_DOWN, 10, 10),
                 lambda: self._sendTouchEvent(1, avg.Event.CURSOR_MOTION, 20, 10),
                 lambda: self._sendTouchEvent(1, avg.Event.CURSOR_UP, 30, 10),
                 lambda: checkTransform(ui.Transform((0,0))),
                 # Re-enable
                 lambda: self.__transformRecognizer.enable(True),
                 lambda: self._sendTouchEvent(1, avg.Event.CURSOR_DOWN, 10, 10),
                 lambda: self._sendTouchEvent(1, avg.Event.CURSOR_UP, 20, 10),
                 lambda: checkTransform(ui.Transform((10,0))),
                 # Disable during gesture
                 lambda: self._sendTouchEvent(1, avg.Event.CURSOR_DOWN, 10, 10),
                 lambda: self.__transformRecognizer.enable(False),
                 lambda: self._sendTouchEvent(1, avg.Event.CURSOR_UP, 20, 10),
                 lambda: checkTransform(ui.Transform((0,0))),
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

    def __initImageScene(self):
        root = self.loadEmptyScene()
        self.image = avg.ImageNode(parent=root, href="rgb24-64x64.png")

    def __killImageNode(self):
        self.image.unlink(True)
        self.image = None


def gestureTestSuite(tests):
    availableTests = (
        "testTapRecognizer",
        "testHoldRecognizer",
        "testDoubletapRecognizer",
        "testDragRecognizer",
        "testDragRecognizerRelCoords",
        "testDragRecognizerInitialEvent",
        "testDragRecognizerCoordSysNode",
        "testDragRecognizerMinDist",
        "testTransformRecognizer",
        "testKMeans",
        "testMat3x3",
        )

    return createAVGTestSuite(availableTests, GestureTestCase, tests)
