# -*- coding: utf-8 -*-
# libavg - Media Playback Engine.
# Copyright (C) 2003-2014 Ulrich von Zadow
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

from libavg import avg, gesture, player

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
        self.__tapRecognizer = gesture.TapRecognizer(self.image)
        self.messageTester = MessageTester(self.__tapRecognizer, 
                [gesture.Recognizer.POSSIBLE, gesture.Recognizer.DETECTED, 
                gesture.Recognizer.FAILED], self)
        player.setFakeFPS(10)
        self.start(False,
                (# Down-up: recognized as tap.
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30,
                        [gesture.Recognizer.POSSIBLE]),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, 
                        [gesture.Recognizer.DETECTED]),
                 # Down-small move-up: recognized as tap.
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30,
                        [gesture.Recognizer.POSSIBLE]),
                 self._genMouseEventFrames(avg.Event.CURSOR_MOTION, 31, 30, []),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, 
                        [gesture.Recognizer.DETECTED]),

                 # Down-small down-second up-second up-first: recognized as tap
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30,
                        [gesture.Recognizer.POSSIBLE]),
                 lambda: self._sendMouseEvent(avg.Event.CURSOR_DOWN, 31, 30, btn=2),
                 lambda: self._sendMouseEvent(avg.Event.CURSOR_UP, 31, 30, btn=2),
                 lambda: self._sendMouseEvent(avg.Event.CURSOR_DOWN, 31, 30, btn=2),
                 lambda: self._sendMouseEvent(avg.Event.CURSOR_UP, 31, 30, btn=2),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, 
                        [gesture.Recognizer.DETECTED]),

                 # Down-big move-up: fail
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30,
                        [gesture.Recognizer.POSSIBLE]),
                 self._genMouseEventFrames(avg.Event.CURSOR_MOTION, 80, 80, 
                        [gesture.Recognizer.FAILED]),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),
                 # Down-Abort-Up: not recognized as tap
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                        [gesture.Recognizer.POSSIBLE]),
                 abort,
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),
                 # Abort-Down-Up: recognized as tap
                 abort,
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                        [gesture.Recognizer.POSSIBLE]),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, 
                        [gesture.Recognizer.DETECTED]),
                 # Down-Abort-Up-Down-Up: recognized as tap
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                        [gesture.Recognizer.POSSIBLE]),
                 abort,
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                        [gesture.Recognizer.POSSIBLE]),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, 
                        [gesture.Recognizer.DETECTED]),
                 # Disable-Down-Up-Enable: not recognized as tap
                 lambda: enable(False),
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, []),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),
                 lambda: enable(True),
                 # Down-Disable-Enable-Up: not recognized as tap
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                        [gesture.Recognizer.POSSIBLE]),
                 lambda: enable(False),
                 lambda: enable(True),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),
                 # Down-Disable-Up-Enable-Down-Up: recognized as tap
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                        [gesture.Recognizer.POSSIBLE]),
                 lambda: enable(False),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),
                 lambda: enable(True),
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                        [gesture.Recognizer.POSSIBLE]),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, 
                        [gesture.Recognizer.DETECTED]),
                 # Abort-Disable-Abort-Enable-Abort-Down-Up: recognized as tap
                 abort,
                 lambda: enable(False),
                 abort,
                 lambda: enable(True),
                 abort,
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                        [gesture.Recognizer.POSSIBLE]),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, 
                        [gesture.Recognizer.DETECTED]),

                 # Remove node while tap is in progress.
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                        [gesture.Recognizer.POSSIBLE]),
                 self.__killImageNode,
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),
                ))

    def testHoldRecognizer(self):

        def abort():
            self.__holdRecognizer.abort()

        def enable(isEnabled):
            self.__holdRecognizer.enable(isEnabled)

        player.setFakeFPS(20)
        self.__initImageScene()
        self.__holdRecognizer = gesture.HoldRecognizer(self.image,
                delay=1000)
        self.messageTester = MessageTester(self.__holdRecognizer, 
                [gesture.Recognizer.POSSIBLE, gesture.Recognizer.DETECTED,
                gesture.Recognizer.FAILED, gesture.Recognizer.END], self)
        self.start(False,
                (# Standard down-hold-up sequence.
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                        [gesture.Recognizer.POSSIBLE]),
                 lambda: self.delay(1100),
                 lambda: self.messageTester.assertState([gesture.Recognizer.DETECTED]),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30,
                        [gesture.Recognizer.END]),

                 # down-up sequence, hold not long enough.
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                        [gesture.Recognizer.POSSIBLE]),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30,
                        [gesture.Recognizer.FAILED]),

                 # down-move-up sequence, should fail. 
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 1, 1, 
                        [gesture.Recognizer.POSSIBLE]),
                 self._genMouseEventFrames(avg.Event.CURSOR_MOTION, 150, 50, 
                        [gesture.Recognizer.FAILED]),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),

                 # down-hold-abort-up, should be recognized, no end event.
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                        [gesture.Recognizer.POSSIBLE]),
                 lambda: self.delay(1100),
                 lambda: self.messageTester.assertState([gesture.Recognizer.DETECTED]),
                 abort,
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),

                 # down-abort-hold-up, should not be recognized
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                        [gesture.Recognizer.POSSIBLE]),
                 abort,
                 lambda: self.delay(1100),
                 lambda: self.messageTester.assertState([]),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),

                 # down-hold-disabled-up-enabled, should be recognized, no end event.
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                        [gesture.Recognizer.POSSIBLE]),
                 lambda: self.delay(1100),
                 lambda: self.messageTester.assertState([gesture.Recognizer.DETECTED]),
                 lambda: enable(False),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),
                 lambda: enable(True),

                 # down-disabled-enabled-hold-up, should not be recognized
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                        [gesture.Recognizer.POSSIBLE]),
                 lambda: enable(False),
                 lambda: enable(True),
                 lambda: self.delay(1100),
                 lambda: self.messageTester.assertState([]),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),
                 
                 # Remove node while hold is in progress.
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                        [gesture.Recognizer.POSSIBLE]),
                 self.__killImageNode,
                 lambda: self.delay(1100),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),
                ))
        player.setFakeFPS(-1)


    def testDoubletapRecognizer(self):

        def abort():
            self.__tapRecognizer.abort()

        def enable(isEnabled):
            self.__tapRecognizer.enable(isEnabled)

        root = self.loadEmptyScene()
        image = avg.ImageNode(parent=root, href="rgb24-64x64.png", size=(128,128))
        self.__tapRecognizer = gesture.DoubletapRecognizer(image)
        self.messageTester = MessageTester(self.__tapRecognizer, 
                [gesture.Recognizer.POSSIBLE, gesture.Recognizer.DETECTED,
                gesture.Recognizer.FAILED, gesture.Recognizer.END], self)
        player.setFakeFPS(20)
        self.start(False,
                (# Down, up, down, up: click
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                        [gesture.Recognizer.POSSIBLE]),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, []),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30,
                        [gesture.Recognizer.DETECTED]),
                 # Down, move: stop
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 0, 30,
                        [gesture.Recognizer.POSSIBLE]),
                 self._genMouseEventFrames(avg.Event.CURSOR_MOTION, 80, 30, 
                        [gesture.Recognizer.FAILED]),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 0, 30, []),
                 # Down, up, move: stop
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 0, 30, 
                        [gesture.Recognizer.POSSIBLE]),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 0, 30, []),
                 self._genMouseEventFrames(avg.Event.CURSOR_MOTION, 80, 30, []),
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 80, 30, 
                        [gesture.Recognizer.FAILED]),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 0, 30, []),
                 # Down, up, down, move: stop
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 0, 30,
                        [gesture.Recognizer.POSSIBLE]),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 0, 30, []),
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 0, 30, []),
                 self._genMouseEventFrames(avg.Event.CURSOR_MOTION, 80, 30,
                        [gesture.Recognizer.FAILED]),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 0, 30, []),
                 # Down,delay: stop
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                        [gesture.Recognizer.POSSIBLE]),
                 lambda: self.delay(1000),
                 lambda: self.messageTester.assertState([gesture.Recognizer.FAILED]),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30,
                        []),
                 # Down, up, delay: stop
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                        [gesture.Recognizer.POSSIBLE]),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),
                 lambda: self.delay(1000),
                 lambda: self.messageTester.assertState([gesture.Recognizer.FAILED]),
                 # Down, up, down, delay: stop
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30,
                        [gesture.Recognizer.POSSIBLE]),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, []),
                 lambda: self.delay(1000),
                 lambda: self.messageTester.assertState([gesture.Recognizer.FAILED]),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, 
                        []),
                 # Down, abort, up, down, up, delay: just one click
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30,
                        [gesture.Recognizer.POSSIBLE]),
                 abort,
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30,
                        [gesture.Recognizer.POSSIBLE]),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),
                 lambda: self.delay(1000),
                 lambda: self.messageTester.assertState([gesture.Recognizer.FAILED]),
                 # Down, up, abort, down, up, delay: two clicks but no double-click
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                        [gesture.Recognizer.POSSIBLE]),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),
                 abort,
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                        [gesture.Recognizer.POSSIBLE]),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),
                 lambda: self.delay(1000),
                 lambda: self.messageTester.assertState([gesture.Recognizer.FAILED]),
                 # Down, up, down, abort, up: just one click
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                        [gesture.Recognizer.POSSIBLE]),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, []),
                 abort,
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),
                 # Down, abort, up, down, up, down up: first aborted then recognized
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                        [gesture.Recognizer.POSSIBLE]),
                 abort,
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30,
                        [gesture.Recognizer.POSSIBLE]),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, []),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30,
                        [gesture.Recognizer.DETECTED]),
                 # Disabled, down, up, down, up, enabled: nothing
                 lambda: enable(False),
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, []),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, []),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),
                 lambda: enable(True),                 
                 # Down, disabled up, down, up, enabled: just one down
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                        [gesture.Recognizer.POSSIBLE]),
                 lambda: enable(False),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, []),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),
                 lambda: enable(True),
                 # Down, up, disabled, down, up, enabled: just one click
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                        [gesture.Recognizer.POSSIBLE]),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),
                 lambda: enable(False),
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, []),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),
                 lambda: enable(True),
                 # Down, up, down, disabled, up, enabled: just one click
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                        [gesture.Recognizer.POSSIBLE]),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, []),
                 lambda: enable(False),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),
                 lambda: enable(True),
                 # Down, disabled, enabled, up, down, up, down, up: recognized
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                        [gesture.Recognizer.POSSIBLE]),
                 lambda: enable(False),
                 lambda: enable(True),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                        [gesture.Recognizer.POSSIBLE]),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, []),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, 
                        [gesture.Recognizer.DETECTED]),
                ))


    def testSwipeRecognizer(self):

        # One finger
        for direction, xdir in (
                (gesture.SwipeRecognizer.RIGHT, 1), (gesture.SwipeRecognizer.LEFT, -1)):
            self.__initImageScene()
            swipeRecognizer = gesture.SwipeRecognizer(self.image, minDist=20,
                    direction=direction)
            self.messageTester = MessageTester(swipeRecognizer,
                    [gesture.Recognizer.POSSIBLE, gesture.Recognizer.DETECTED,
                    gesture.Recognizer.FAILED], 
                    self)
            self.start(False,
                    (self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30,
                            [gesture.Recognizer.POSSIBLE]),
                     self._genMouseEventFrames(avg.Event.CURSOR_UP, 30+xdir*30, 30,
                            [gesture.Recognizer.DETECTED]),
                     # Check angle tolerance
                     self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30,
                            [gesture.Recognizer.POSSIBLE]),
                     self._genMouseEventFrames(avg.Event.CURSOR_UP, 30+xdir*30, 25,
                            [gesture.Recognizer.DETECTED]),
                     self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30,
                            [gesture.Recognizer.POSSIBLE]),
                     self._genMouseEventFrames(avg.Event.CURSOR_UP, 30+xdir*30, 35,
                            [gesture.Recognizer.DETECTED]),
                     # Not far enough -> fail
                     self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30,
                            [gesture.Recognizer.POSSIBLE]),
                     self._genMouseEventFrames(avg.Event.CURSOR_UP, 30+xdir*10, 30,
                            [gesture.Recognizer.FAILED]),
                     # Wrong direction -> fail
                     self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30,
                            [gesture.Recognizer.POSSIBLE]),
                     self._genMouseEventFrames(avg.Event.CURSOR_UP, 30+xdir*30, 60,
                            [gesture.Recognizer.FAILED]),
                     self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30,
                            [gesture.Recognizer.POSSIBLE]),
                     self._genMouseEventFrames(avg.Event.CURSOR_UP, 30+xdir*30, 5,
                            [gesture.Recognizer.FAILED]),
                    ))


    def testSwipeRecognizerTwoFingers(self):
        self.__initImageScene()
        swipeRecognizer = gesture.SwipeRecognizer(self.image, minDist=20, numContacts=2,
                maxContactDist=15, direction=gesture.SwipeRecognizer.RIGHT)
        self.messageTester = MessageTester(swipeRecognizer,
                [gesture.Recognizer.POSSIBLE, gesture.Recognizer.DETECTED,
                 gesture.Recognizer.FAILED], 
                self)
        self.start(False,
                (self._genTouchEventFrames(
                        [(0, avg.Event.CURSOR_DOWN, 30, 30,),],
                        []), 
                 self._genTouchEventFrames(
                        [(1, avg.Event.CURSOR_DOWN, 40, 30,),],
                        [gesture.Recognizer.POSSIBLE]), 
                 self._genTouchEventFrames(
                        [(1, avg.Event.CURSOR_UP, 70, 30,),],
                        []), 
                 self._genTouchEventFrames(
                        [(0, avg.Event.CURSOR_UP, 60, 30,),],
                        [gesture.Recognizer.DETECTED]),
                 # Not enough fingers -> not recognized
                 self._genTouchEventFrames(
                        [(0, avg.Event.CURSOR_DOWN, 30, 30,),],
                        []), 
                 self._genTouchEventFrames(
                        [(0, avg.Event.CURSOR_UP, 60, 30,),],
                        []),
                 # Fail first finger
                 self._genTouchEventFrames(
                        [(0, avg.Event.CURSOR_DOWN, 30, 30,),],
                        []), 
                 self._genTouchEventFrames(
                        [(1, avg.Event.CURSOR_DOWN, 40, 30,),],
                        [gesture.Recognizer.POSSIBLE]), 
                 self._genTouchEventFrames(
                        [(1, avg.Event.CURSOR_UP, 35, 30,),],
                        [gesture.Recognizer.FAILED]), 
                 self._genTouchEventFrames(
                        [(0, avg.Event.CURSOR_UP, 60, 30,),],
                        []),
                 # Fail second finger
                 self._genTouchEventFrames(
                        [(0, avg.Event.CURSOR_DOWN, 30, 30,),],
                        []), 
                 self._genTouchEventFrames(
                        [(1, avg.Event.CURSOR_DOWN, 40, 30,),],
                        [gesture.Recognizer.POSSIBLE]), 
                 self._genTouchEventFrames(
                        [(1, avg.Event.CURSOR_UP, 70, 30,),],
                        []), 
                 self._genTouchEventFrames(
                        [(0, avg.Event.CURSOR_UP, 35, 30,),],
                        [gesture.Recognizer.FAILED]),
                 # Fingers too far apart
                 self._genTouchEventFrames(
                        [(0, avg.Event.CURSOR_DOWN, 30, 30,),],
                        []), 
                 self._genTouchEventFrames(
                        [(1, avg.Event.CURSOR_DOWN, 50, 30,),],
                        [gesture.Recognizer.FAILED]), 
                 self._genTouchEventFrames(
                        [(1, avg.Event.CURSOR_UP, 70, 30,),
                         (0, avg.Event.CURSOR_UP, 60, 30,),],
                        []),
                ))

    def testDragRecognizer(self):

        def onMove(offset):
            if self.friction == -1:
                self.assertEqual(offset, (40,40))
            self.messageTester.setMessageReceived(gesture.Recognizer.MOTION)

        def onUp(offset):
            if self.friction == -1:
                self.assertEqual(offset, (10,-10))
            self.messageTester.setMessageReceived(gesture.Recognizer.UP)

        def enable(isEnabled):
            dragRecognizer.enable(isEnabled)

        def abort():
            dragRecognizer.abort()

        def setupRecognizer(friction, moveHandler=onMove, minDragDist=0, 
                direction=gesture.DragRecognizer.ANY_DIRECTION, **kargs):
            self.__initImageScene()
            dragRecognizer = gesture.DragRecognizer(self.image, moveHandler=moveHandler, 
                    upHandler=onUp, friction=friction, minDragDist=minDragDist, 
                    direction=direction, **kargs)
            messageTester = MessageTester(dragRecognizer, [gesture.Recognizer.POSSIBLE, 
                    gesture.Recognizer.DETECTED, gesture.Recognizer.FAILED,
                    gesture.Recognizer.END], 
                    self)
            return (dragRecognizer, messageTester)

        player.setFakeFPS(100)
        sys.stderr.write("\n")
        for self.friction in (-1, 100):
            if self.friction == -1:
                sys.stderr.write("  Simple drag, no inertia\n")
            else:
                sys.stderr.write("  Simple drag, inertia\n")
            dragRecognizer, self.messageTester = setupRecognizer(friction=self.friction)
            self.start(False,
                    (self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30,
                            [gesture.Recognizer.DETECTED]),
                     self._genMouseEventFrames(avg.Event.CURSOR_MOTION, 70, 70, 
                            [gesture.Recognizer.MOTION]),
                     self._genMouseEventFrames(avg.Event.CURSOR_UP, 40, 20, 
                            [gesture.Recognizer.UP, gesture.Recognizer.END]),
                     lambda: enable(False),
                     self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, []),
                     lambda: dragRecognizer.enable(True),
                     self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),
                     self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                            [gesture.Recognizer.DETECTED]),
                     self._genMouseEventFrames(avg.Event.CURSOR_UP, 40, 20, 
                            [gesture.Recognizer.UP, gesture.Recognizer.END]),

                     # Remove node during drag.
                     self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                            [gesture.Recognizer.DETECTED]),
                     self.__killImageNode,
                     self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),
                    ))

        # Test with constraint.
        def onVertMove(offset):
            if self.friction == -1:
                self.assertEqual(offset, (0,40))
            self.messageTester.setMessageReceived(gesture.Recognizer.MOTION)

        for self.friction in (-1, 100):
            if self.friction == -1:
                sys.stderr.write("  Drag with constraint, no inertia\n")
            else:
                sys.stderr.write("  Drag with constraint, inertia\n")
            dragRecognizer, self.messageTester = setupRecognizer(moveHandler=onVertMove, 
                    friction=self.friction, direction=gesture.DragRecognizer.VERTICAL,
                    minDragDist=5)
            self.start(False,
                    (self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                            [gesture.Recognizer.POSSIBLE]),
                     self._genMouseEventFrames(avg.Event.CURSOR_MOTION, 35, 30, []),
                     self._genMouseEventFrames(avg.Event.CURSOR_MOTION, 30, 70, 
                            [gesture.Recognizer.DETECTED, gesture.Recognizer.MOTION]),
                     self._genMouseEventFrames(avg.Event.CURSOR_UP, 40, 20, 
                            [gesture.Recognizer.UP, gesture.Recognizer.END]),
                     # Wrong direction -> stop.
                     self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                            [gesture.Recognizer.POSSIBLE]),
                     self._genMouseEventFrames(avg.Event.CURSOR_MOTION, 70, 30, 
                            [gesture.Recognizer.FAILED]),
                     self._genMouseEventFrames(avg.Event.CURSOR_UP, 70, 30, []),

                     # No movement -> stop.
                     self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                            [gesture.Recognizer.POSSIBLE]),
                     self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, 
                            [gesture.Recognizer.FAILED]),

                     # Down, Abort, Motion, Motion, Up -> not recognized
                     self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                            [gesture.Recognizer.POSSIBLE]),
                     abort,
                     self._genMouseEventFrames(avg.Event.CURSOR_MOTION, 35, 30, []),
                     self._genMouseEventFrames(avg.Event.CURSOR_MOTION, 30, 70, []),
                     self._genMouseEventFrames(avg.Event.CURSOR_UP, 40, 20, []),

                     # Down, Motion, Abort, Motion, Up -> not Recognized
                     self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                            [gesture.Recognizer.POSSIBLE]),
                     self._genMouseEventFrames(avg.Event.CURSOR_MOTION, 35, 30, []),
                     abort,
                     self._genMouseEventFrames(avg.Event.CURSOR_MOTION, 30, 70, []),
                     self._genMouseEventFrames(avg.Event.CURSOR_UP, 40, 20, []),

                     # Down, Motion, Motion, Abort, Up -> not recognized
                     self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                            [gesture.Recognizer.POSSIBLE]),
                     self._genMouseEventFrames(avg.Event.CURSOR_MOTION, 35, 30, []),
                     self._genMouseEventFrames(avg.Event.CURSOR_MOTION, 30, 70,
                            [gesture.Recognizer.DETECTED, gesture.Recognizer.MOTION]),
                     abort,
                     self._genMouseEventFrames(avg.Event.CURSOR_UP, 40, 20, []),

                     # Down, Motion, Abort, Up, Down, Motion, Motion, Up -> Recognized
                     self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                            [gesture.Recognizer.POSSIBLE]),
                     self._genMouseEventFrames(avg.Event.CURSOR_MOTION, 35, 30, []),
                     abort,
                     self._genMouseEventFrames(avg.Event.CURSOR_UP, 40, 20, []),
                     
                     self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                            [gesture.Recognizer.POSSIBLE]),
                     self._genMouseEventFrames(avg.Event.CURSOR_MOTION, 35, 30, []),
                     self._genMouseEventFrames(avg.Event.CURSOR_MOTION, 30, 70,
                            [gesture.Recognizer.DETECTED, gesture.Recognizer.MOTION]),
                     self._genMouseEventFrames(avg.Event.CURSOR_UP, 40, 20, 
                            [gesture.Recognizer.UP, gesture.Recognizer.END]),
                    ))

        # Test second down during inertia.
        sys.stderr.write("  Down during inertia\n")
        dragRecognizer, self.messageTester = setupRecognizer(friction=0.01)
        self.start(False,
                (lambda: self._sendMouseEvent(avg.Event.CURSOR_DOWN, 30, 30),
                 lambda: self._sendMouseEvent(avg.Event.CURSOR_UP, 40, 20),
                 self.messageTester.reset,
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 40, 20, 
                            [gesture.Recognizer.END, gesture.Recognizer.DETECTED,
                             gesture.Recognizer.MOTION]),
                ))

        # Test node delete during inertia
        sys.stderr.write("  Delete during inertia\n")
        dragRecognizer, self.messageTester = setupRecognizer(friction=0.01)
        self.start(False,
                (self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                        [gesture.Recognizer.DETECTED]),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 40, 20, 
                        [gesture.Recognizer.MOTION, gesture.Recognizer.UP]),
                 self.__killImageNode,
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 40, 20,
                        [gesture.Recognizer.MOTION]),
                ))

        # Test second down during inertia, constrained recognizer
        sys.stderr.write("  Down during inertia, constrained recognizer\n")
        dragRecognizer, self.messageTester = setupRecognizer(friction=0.01,
                direction=gesture.DragRecognizer.VERTICAL, minDragDist=5)
        self.start(False,
                (lambda: self._sendMouseEvent(avg.Event.CURSOR_DOWN, 30, 30),
                 self._genMouseEventFrames(avg.Event.CURSOR_MOTION, 30, 70,
                        [gesture.Recognizer.DETECTED, gesture.Recognizer.MOTION, 
                         gesture.Recognizer.POSSIBLE]),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 70,
                        [gesture.Recognizer.MOTION, gesture.Recognizer.UP]),
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30, 
                        [gesture.Recognizer.END, gesture.Recognizer.POSSIBLE,
                         gesture.Recognizer.MOTION]),
                ))

        # Test abort in possible handler
        for self.friction in (-1, 100):
            if self.friction == -1:
                sys.stderr.write("  Abort in possible handler, no inertia\n")
            else:
                sys.stderr.write("  Abort in possible handler, inertia\n")
            dragRecognizer, self.messageTester = setupRecognizer(friction=self.friction,
                    minDragDist=5, possibleHandler=abort)
            self.start(False,
                    (self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30,
                            [gesture.Recognizer.POSSIBLE]),
                     self._genMouseEventFrames(avg.Event.CURSOR_MOTION, 70, 70, []),
                     self._genMouseEventFrames(avg.Event.CURSOR_UP, 70, 70, []),
                     self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30,
                            [gesture.Recognizer.POSSIBLE]),
                    ))

        player.setFakeFPS(-1)


    def testDragRecognizerRelCoords(self):

        def onDrag(offset):
            self.assertAlmostEqual(offset, (-40,-40))
            self.__onDragCalled = True

        player.setFakeFPS(100)
        self.__onDragCalled = False
        for self.friction in (-1, 100):
            root = self.loadEmptyScene()
            div = avg.DivNode(pos=(64,64), angle=math.pi, parent=root)
            image = avg.ImageNode(parent=div, href="rgb24-64x64.png")
            dragRecognizer = gesture.DragRecognizer(image, moveHandler=onDrag,
                    friction=self.friction)
            self.start(False,
                    (lambda: self._sendMouseEvent(avg.Event.CURSOR_DOWN, 30, 30),
                     lambda: self._sendMouseEvent(avg.Event.CURSOR_MOTION, 70, 70),
                    ))
        player.setFakeFPS(-1)
        assert(self.__onDragCalled)


    def testDragRecognizerInitialEvent(self):

        def onMotion(offset):
            gesture.DragRecognizer(self.image, 
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
            self.__dragRecognizerCalled = True

        root = self.loadEmptyScene()
        div = avg.DivNode(pos=(64,64), angle=math.pi, parent=root)
        image = avg.ImageNode(parent=div, href="rgb24-64x64.png")
        dragRecognizer = gesture.DragRecognizer(image, moveHandler=onDrag,
                coordSysNode=div, friction=-1)
        self.__dragRecognizerCalled = False
        self.start(False,
                (lambda: self._sendMouseEvent(avg.Event.CURSOR_DOWN, 30, 30),
                 lambda: self._sendMouseEvent(avg.Event.CURSOR_MOTION, 70, 70),
                ))
        assert(self.__dragRecognizerCalled)


    def testDragRecognizerCoordSysNodeParentUnlink(self):

        def onDrag(offset):
            self.assertEqual(offset, (40,40))
            self.__dragRecognizerCalled = True

        def onUp(offset):
            self.__upRecognizerCalled = True

        root = self.loadEmptyScene()
        div = avg.DivNode(pos=(64,64), angle=math.pi, parent=root)
        image = avg.ImageNode(parent=div, href="rgb24-64x64.png")
        dragRecognizer = gesture.DragRecognizer(image, moveHandler=onDrag,
                coordSysNode=div, friction=-1)
        self.__dragRecognizerCalled = False
        self.__upRecognizerCalled = False
        self.start(False,
                (lambda: self._sendMouseEvent(avg.Event.CURSOR_DOWN, 30, 30),
                 lambda: self._sendMouseEvent(avg.Event.CURSOR_MOTION, 70, 70),
                 lambda: div.unlink(False),
                 lambda: self._sendMouseEvent(avg.Event.CURSOR_UP, 70, 70),
                ))
        assert(self.__dragRecognizerCalled)
        assert(not self.__upRecognizerCalled)


    def testDragRecognizerMinDist(self):

        def onMove(offset):
            self.messageTester.setMessageReceived(gesture.Recognizer.MOTION)

        self.__initImageScene()
        dragRecognizer = gesture.DragRecognizer(self.image, moveHandler=onMove,
                minDragDist=10, friction=-1)
        self.messageTester = MessageTester(dragRecognizer, [gesture.Recognizer.DETECTED], 
                self)
        self.start(False,
                (lambda: self._sendMouseEvent(avg.Event.CURSOR_DOWN, 30, 30),
                 self._genMouseEventFrames(avg.Event.CURSOR_MOTION, 30, 35,
                        []),
                 self._genMouseEventFrames(avg.Event.CURSOR_MOTION, 30, 50,
                        [gesture.Recognizer.DETECTED, gesture.Recognizer.MOTION]),
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
                    lambda: checkTransform(gesture.Transform((10,0))),
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

        player.setFakeFPS(100)
        self.__initImageScene()
        # Turn off the jitter filter.
        gesture.TransformRecognizer.FILTER_MIN_CUTOFF = None
        gesture.TransformRecognizer.FILTER_BETA = None
        
        self.__transformRecognizer = gesture.TransformRecognizer(self.image,
                friction=-1,
                detectedHandler=onDetected, moveHandler=onMove, upHandler=onUp)
        self.start(False,
                (# Check up/down handling
                 lambda: self._sendTouchEvent(1, avg.Event.CURSOR_DOWN, 10, 10),
                 lambda: checkTransform(gesture.Transform((0,0))),
                 lambda: self._sendTouchEvent(1, avg.Event.CURSOR_MOTION, 20, 10),
                 lambda: checkTransform(gesture.Transform((10,0))),
                 lambda: self._sendTouchEvent(2, avg.Event.CURSOR_DOWN, 20, 20),
                 lambda: checkTransform(gesture.Transform((0,0))),
                 lambda: self._sendTouchEvents((
                        (1, avg.Event.CURSOR_MOTION, 30, 10),
                        (2, avg.Event.CURSOR_MOTION, 30, 20))),
                 lambda: checkTransform(gesture.Transform((10,0))),
                 lambda: self._sendTouchEvent(2, avg.Event.CURSOR_UP, 30, 20),
                 lambda: checkTransform(gesture.Transform((0,0))),
                 lambda: self._sendTouchEvent(1, avg.Event.CURSOR_MOTION, 40, 10),
                 lambda: checkTransform(gesture.Transform((10,0))),
                 lambda: self._sendTouchEvent(1, avg.Event.CURSOR_UP, 50, 10),
                 lambda: checkTransform(gesture.Transform((10,0))),

                 createRotTestFrames(gesture.Transform((0,0), math.pi, 1, (0,15))),

                 createScaleTestFrames(gesture.Transform((0,5), 0, 2, (0,20))),

                 # Delete node during transform
                 lambda: self._sendTouchEvents((
                        (1, avg.Event.CURSOR_DOWN, 30, 10),
                        (2, avg.Event.CURSOR_DOWN, 30, 20))),
                 self.__killImageNode,
                 lambda: self._sendTouchEvents((
                        (1, avg.Event.CURSOR_UP, 30, 10),
                        (2, avg.Event.CURSOR_UP, 30, 20))),
                ))

        self.__initImageScene()
        self.__transformRecognizer = gesture.TransformRecognizer(self.image,
                friction=-1,
                detectedHandler=onDetected, moveHandler=onMove, upHandler=onUp)
        self.start(False,
                (# Unlink node during transform (don't delete)
                 lambda: self._sendTouchEvents((
                        (1, avg.Event.CURSOR_DOWN, 30, 10),
                        (2, avg.Event.CURSOR_DOWN, 30, 20))),
                 lambda: self.image.unlink(True),
                 lambda: self._sendTouchEvents((
                        (1, avg.Event.CURSOR_UP, 30, 10),
                        (2, avg.Event.CURSOR_UP, 30, 20))),
                ))

        # Test rel. coords.
        root = self.loadEmptyScene()
        div = avg.DivNode(parent=root, pos=(0,10))
        image = avg.ImageNode(parent=div, href="rgb24-64x64.png")
        self.__transformRecognizer = gesture.TransformRecognizer(image, friction=-1, 
                detectedHandler=onDetected, moveHandler=onMove, upHandler=onUp)
        self.start(False,
                (createTransTestFrames(),
                 createRotTestFrames(gesture.Transform((0,0), math.pi, 1, (0,5))),
                 createScaleTestFrames(gesture.Transform((0,5), 0, 2, (0,10))),
                ))

        # Test coordSysNode.
        root = self.loadEmptyScene()
        div = avg.DivNode(parent=root, pos=(0,10))
        image = avg.ImageNode(parent=div, href="rgb24-64x64.png")
        self.__transformRecognizer = gesture.TransformRecognizer(image, coordSysNode=div,
                friction=-1,
                detectedHandler=onDetected, moveHandler=onMove, upHandler=onUp)
        self.start(False,
                (createTransTestFrames(),
                 createRotTestFrames(gesture.Transform((0,0), math.pi, 1, (0,15))),
                 createScaleTestFrames(gesture.Transform((0,5), 0, 2, (0,20))),
                ))

        # Test friction
        root = self.loadEmptyScene()
        div = avg.DivNode(parent=root, pos=(0,10))
        image = avg.ImageNode(parent=div, href="rgb24-64x64.png")
        self.__transformRecognizer = gesture.TransformRecognizer(image, friction=0.01,
                detectedHandler=onDetected, moveHandler=onMove, upHandler=onUp)
        self.start(False,
                (lambda: self._sendTouchEvent(1, avg.Event.CURSOR_DOWN, 10, 10),
                 lambda: self._sendTouchEvent(1, avg.Event.CURSOR_UP, 20, 10),
                 lambda: self._sendTouchEvent(1, avg.Event.CURSOR_DOWN, 10, 10),
                 lambda: self._sendTouchEvent(1, avg.Event.CURSOR_UP, 20, 10),
                ))  

        # Test abort
        self.__initImageScene()
        self.__transformRecognizer = gesture.TransformRecognizer(self.image, 
                detectedHandler=onDetected, moveHandler=onMove, upHandler=onUp)
        self.start(False,
                (self.__transformRecognizer.abort,
                 lambda: self._sendTouchEvent(1, avg.Event.CURSOR_DOWN, 10, 10),
                 self.__transformRecognizer.abort,
                 lambda: self._sendTouchEvent(1, avg.Event.CURSOR_MOTION, 20, 10),
                 lambda: self._sendTouchEvent(1, avg.Event.CURSOR_UP, 30, 10),
                 lambda: checkTransform(gesture.Transform((0,0))),
                 self.__transformRecognizer.abort,
                ))

        # Test enable/disable
        self.__initImageScene()
        self.__transformRecognizer = gesture.TransformRecognizer(self.image, friction=-1,
                detectedHandler=onDetected, moveHandler=onMove, upHandler=onUp)

        def disableOnUp(_):
            self.__transformRecognizer.enable(False)

        self.start(False,
                (# Regular disable
                 lambda: self.__transformRecognizer.enable(False),
                 lambda: self._sendTouchEvent(1, avg.Event.CURSOR_DOWN, 10, 10),
                 lambda: self._sendTouchEvent(1, avg.Event.CURSOR_MOTION, 20, 10),
                 lambda: self._sendTouchEvent(1, avg.Event.CURSOR_UP, 30, 10),
                 lambda: checkTransform(gesture.Transform((0,0))),
                 # Re-enable
                 lambda: self.__transformRecognizer.enable(True),
                 lambda: self._sendTouchEvent(1, avg.Event.CURSOR_DOWN, 10, 10),
                 lambda: self._sendTouchEvent(1, avg.Event.CURSOR_UP, 20, 10),
                 lambda: checkTransform(gesture.Transform((10,0))),
                 # Disable during gesture
                 lambda: self._sendTouchEvent(1, avg.Event.CURSOR_DOWN, 10, 10),
                 lambda: self.__transformRecognizer.enable(False),
                 lambda: self._sendTouchEvent(1, avg.Event.CURSOR_UP, 20, 10),
                 lambda: checkTransform(gesture.Transform((0,0))),
                 lambda: self.__transformRecognizer.enable(True),
                 # Disable during up event
                 lambda: self._sendTouchEvent(1, avg.Event.CURSOR_DOWN, 10, 10),
                 lambda: self.__transformRecognizer.subscribe(
                        self.__transformRecognizer.UP, disableOnUp),
                 lambda: self._sendTouchEvent(1, avg.Event.CURSOR_UP, 20, 10),
                 lambda: checkTransform(gesture.Transform((10, 0))),
                ))

        # Test enable/disable, friction
        def disableDuringEnd():
            self.__transformRecognizer.enable(False)

        self.__initImageScene()
        self.__transformRecognizer = gesture.TransformRecognizer(self.image, friction=1,
                detectedHandler=onDetected, moveHandler=onMove, upHandler=onUp)
        self.start(False,
                (# Disable during end event
                 lambda: self._sendTouchEvent(1, avg.Event.CURSOR_DOWN, 10, 10),
                 lambda: self.__transformRecognizer.subscribe(gesture.Recognizer.END,
                        disableDuringEnd),
                 lambda: self._sendTouchEvent(1, avg.Event.CURSOR_UP, 10, 10),
                 None,
                ))

        # Test abort during up
        def abortDuringUp(event):
            self.__transformRecognizer.enable(False)

        self.__initImageScene()
        self.__transformRecognizer = gesture.TransformRecognizer(self.image, 
                friction=0.01,
                detectedHandler=onDetected, moveHandler=onMove, upHandler=onUp)
        self.start(False,
                (
                 lambda: self._sendTouchEvent(1, avg.Event.CURSOR_DOWN, 10, 10),
                 lambda: self.__transformRecognizer.subscribe(gesture.Recognizer.UP,
                        abortDuringUp),
                 lambda: self._sendTouchEvent(1, avg.Event.CURSOR_UP, 10, 10),
                ))
        

        # Test second down during inertia.
        self.__initImageScene()
        self.__transformRecognizer = gesture.TransformRecognizer(self.image, 
                friction=0.01, detectedHandler=onDetected, moveHandler=onMove, 
                upHandler=onUp)
        self.start(False,
                (
                 lambda: self._sendTouchEvent(1, avg.Event.CURSOR_DOWN, 10, 10),
                 lambda: self._sendTouchEvent(1, avg.Event.CURSOR_UP, 30, 10),
                 lambda: self._sendTouchEvent(1, avg.Event.CURSOR_DOWN, 30, 10),
                ))
        player.setFakeFPS(-1)

    def testKMeans(self):
        pts = [avg.Point2D(0,0), avg.Point2D(0,1)]
        means = gesture.calcKMeans(pts)
        self.assertEqual(means, ([0], [1]))

        pts.append (avg.Point2D(0,4))
        means = gesture.calcKMeans(pts)
        self.assertEqual(means, ([0,1], [2]))


    def testMat3x3(self):
        t = gesture.Mat3x3.translate([1,0,1])
        v = [1,0,1]
        self.assertEqual(t.applyVec(v), [2,0,1])
        r = gesture.Mat3x3.rotate(math.pi/2)
        self.assertAlmostEqual(r.applyVec(v), [0,1,1])
        self.assertAlmostEqual(t.applyMat(t).m, gesture.Mat3x3.translate([2,0,1]).m)
        self.assertAlmostEqual(t.applyMat(r).m, gesture.Mat3x3([0,-1,1],[1,0,0]).m)
        self.assertAlmostEqual(r.applyMat(t).m, gesture.Mat3x3([0,-1,0],[1,0,1]).m)
        self.assertAlmostEqual(gesture.Mat3x3().m, gesture.Mat3x3().inverse().m)
        m = gesture.Mat3x3([-1,  3, -3], 
                      [ 0, -6,  5],
                      [-5, -3,  1])
        im = gesture.Mat3x3([3./2,      1., -1./2],
                       [-25./6, -8./3,  5./6],
                       [-5.,      -3.,    1.])
        self.assertAlmostEqual(m.inverse().m, im.m)

        image = avg.ImageNode(pos=(10,20), size=(30,40), angle=1.57, 
            href="rgb24alpha-64x64.png")
        mat = gesture.Mat3x3.fromNode(image)
        mat.setNodeTransform(image)
        self.assertAlmostEqual(image.pos, (10,20))
        self.assertAlmostEqual(image.size, (30,40))
        self.assertAlmostEqual(image.angle, 1.57)

    def testTwoRecognizers(self):
        self.__initImageScene()
        self.__tapRecognizer = gesture.TapRecognizer(self.image)
        self.messageTester = MessageTester(self.__tapRecognizer,
                [gesture.Recognizer.POSSIBLE, gesture.Recognizer.DETECTED,
                gesture.Recognizer.FAILED], self)
        self.__tapRecognizer2 = gesture.TapRecognizer(self.image)
        player.setFakeFPS(10)
        self.start(False,
                (# Standard down-hold-up sequence.
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 30, 30,
                        [gesture.Recognizer.POSSIBLE]),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30,
                        [gesture.Recognizer.DETECTED]),

                 # down-move-up sequence, should fail.
                 self._genMouseEventFrames(avg.Event.CURSOR_DOWN, 1, 1,
                        [gesture.Recognizer.POSSIBLE]),
                 self._genMouseEventFrames(avg.Event.CURSOR_MOTION, 150, 50,
                        [gesture.Recognizer.FAILED]),
                 self._genMouseEventFrames(avg.Event.CURSOR_UP, 30, 30, []),
                ))

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
        "testSwipeRecognizer",
        "testSwipeRecognizerTwoFingers",
        "testDragRecognizer",
        "testDragRecognizerRelCoords",
        "testDragRecognizerInitialEvent",
        "testDragRecognizerCoordSysNode",
        "testDragRecognizerCoordSysNodeParentUnlink",
        "testDragRecognizerMinDist",
        "testTransformRecognizer",
        "testTwoRecognizers",
        "testKMeans",
        "testMat3x3",
        )

    return createAVGTestSuite(availableTests, GestureTestCase, tests)
