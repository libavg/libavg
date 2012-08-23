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
from sets import Set
from testcase import *

EVENT_DETECTED = 1
EVENT_MOVED = 2
EVENT_UP = 3
EVENT_ENDED = 4
EVENT_POSSIBLE = 5
EVENT_FAILED = 6
    
class GestureTestCase(AVGTestCase):
    
    def __init__(self, testFuncName):
        AVGTestCase.__init__(self, testFuncName)

    def testTapRecognizer(self):

        def onPossible(event):
            self.__addEventFlag(EVENT_POSSIBLE)

        def onDetected(event):
            self.__addEventFlag(EVENT_DETECTED)

        def onFail(event):
            self.__addEventFlag(EVENT_FAILED)

        def abort():
            self.__tapRecognizer.abort()
            self.__resetEventState()

        def enable(isEnabled):
            self.__tapRecognizer.enable(isEnabled)
            self.__resetEventState()

        self.__initImageScene()
        self.__tapRecognizer = ui.TapRecognizer(self.image,
                possibleHandler=onPossible,
                detectedHandler=onDetected,
                failHandler=onFail)
        self.__resetEventState()
        player.setFakeFPS(10)
        self.start(False,
                (# Down-up: recognized as tap.
                 self.__genMouseEventFrames(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                 self.__genMouseEventFrames(avg.CURSORUP, 30, 30, [EVENT_DETECTED]),
                 # Down-small move-up: recognized as tap.
                 self.__genMouseEventFrames(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                 self.__genMouseEventFrames(avg.CURSORMOTION, 31, 30, []),
                 self.__genMouseEventFrames(avg.CURSORUP, 30, 30, [EVENT_DETECTED]),
                 # Down-big move-up: fail
                 self.__genMouseEventFrames(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                 self.__genMouseEventFrames(avg.CURSORMOTION, 80, 80, [EVENT_FAILED]),
                 self.__genMouseEventFrames(avg.CURSORUP, 30, 30, []),
                 # Down-delay: fail
                 self.__genMouseEventFrames(avg.CURSORDOWN, 1, 1, [EVENT_POSSIBLE]),
                 lambda: self.delay(1000),
                 lambda: self.__assertEvents([EVENT_FAILED]),
                 self.__resetEventState,
                 self.__genMouseEventFrames(avg.CURSORUP, 1, 1, []),
                 # Down-Abort-Up: not recognized as tap
                 self.__genMouseEventFrames(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                 abort,
                 self.__genMouseEventFrames(avg.CURSORUP, 30, 30, []),
                 # Abort-Down-Up: recognized as tap
                 abort,
                 self.__genMouseEventFrames(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                 self.__genMouseEventFrames(avg.CURSORUP, 30, 30, [EVENT_DETECTED]),
                 # Down-Abort-Up-Down-Up: recognized as tap
                 self.__genMouseEventFrames(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                 abort,
                 self.__genMouseEventFrames(avg.CURSORUP, 30, 30, []),
                 self.__genMouseEventFrames(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                 self.__genMouseEventFrames(avg.CURSORUP, 30, 30, [EVENT_DETECTED]),
                 # Disable-Down-Up-Enable: not recognized as tap
                 lambda: enable(False),
                 self.__genMouseEventFrames(avg.CURSORDOWN, 30, 30, []),
                 self.__genMouseEventFrames(avg.CURSORUP, 30, 30, []),
                 lambda: enable(True),
                 # Down-Disable-Enable-Up: not recognized as tap
                 self.__genMouseEventFrames(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                 lambda: enable(False),
                 lambda: enable(True),
                 self.__genMouseEventFrames(avg.CURSORUP, 30, 30, []),
                 # Down-Disable-Up-Enable-Down-Up: recognized as tap
                 self.__genMouseEventFrames(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                 lambda: enable(False),
                 self.__genMouseEventFrames(avg.CURSORUP, 30, 30, []),
                 lambda: enable(True),
                 self.__genMouseEventFrames(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                 self.__genMouseEventFrames(avg.CURSORUP, 30, 30, [EVENT_DETECTED]),
                 # Abort-Disable-Abort-Enable-Abort-Down-Up: recognized as tap
                 abort,
                 lambda: enable(False),
                 abort,
                 lambda: enable(True),
                 abort,
                 self.__genMouseEventFrames(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                 self.__genMouseEventFrames(avg.CURSORUP, 30, 30, [EVENT_DETECTED]),

                 # Remove node while tap is in progress.
                 self.__genMouseEventFrames(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                 self.__killImageNode,
                 self.__genMouseEventFrames(avg.CURSORUP, 30, 30, []),
                ))


    def testHoldRecognizer(self):

        def onPossible(event):
            self.__addEventFlag(EVENT_POSSIBLE)

        def onDetected(event):
            self.__addEventFlag(EVENT_DETECTED)

        def onFail(event):
            self.__addEventFlag(EVENT_FAILED)

        def onStop(event):
            self.__addEventFlag(EVENT_ENDED)

        def abort():
            self.__holdRecognizer.abort()
            self.__resetEventState()

        def enable(isEnabled):
            self.__holdRecognizer.enable(isEnabled)
            self.__resetEventState()

        player.setFakeFPS(20)
        self.__initImageScene()
        self.__holdRecognizer = ui.HoldRecognizer(self.image,
                delay=1000,
                possibleHandler=onPossible, 
                detectedHandler=onDetected, 
                failHandler=onFail, 
                stopHandler=onStop)
        self.__resetEventState()
        self.start(False,
                (# Standard down-hold-up sequence.
                 self.__genMouseEventFrames(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                 lambda: self.delay(1100),
                 lambda: self.__assertEvents([EVENT_DETECTED]),
                 self.__resetEventState,
                 self.__genMouseEventFrames(avg.CURSORUP, 30, 30, [EVENT_ENDED]),

                 # down-up sequence, hold not long enough.
                 self.__genMouseEventFrames(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                 self.__genMouseEventFrames(avg.CURSORUP, 30, 30, [EVENT_FAILED]),

                 # down-move-up sequence, should fail. 
                 self.__genMouseEventFrames(avg.CURSORDOWN, 1, 1, [EVENT_POSSIBLE]),
                 self.__genMouseEventFrames(avg.CURSORMOTION, 150, 50, [EVENT_FAILED]),
                 self.__genMouseEventFrames(avg.CURSORUP, 30, 30, []),

                 # down-hold-abort-up, should be recognized, no end event.
                 self.__genMouseEventFrames(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                 lambda: self.delay(1100),
                 lambda: self.__assertEvents([EVENT_DETECTED]),
                 abort,
                 self.__genMouseEventFrames(avg.CURSORUP, 30, 30, []),

                 # down-abort-hold-up, should not be recognized
                 self.__genMouseEventFrames(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                 abort,
                 lambda: self.delay(1100),
                 lambda: self.__assertEvents([]),
                 self.__genMouseEventFrames(avg.CURSORUP, 30, 30, []),

                 # down-hold-disabled-up-enabled, should be recognized, no end event.
                 self.__genMouseEventFrames(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                 lambda: self.delay(1100),
                 lambda: self.__assertEvents([EVENT_DETECTED]),
                 lambda: enable(False),
                 self.__genMouseEventFrames(avg.CURSORUP, 30, 30, []),
                 lambda: enable(True),

                 # down-disabled-enabled-hold-up, should not be recognized
                 self.__genMouseEventFrames(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                 lambda: enable(False),
                 lambda: enable(True),
                 lambda: self.delay(1100),
                 lambda: self.__assertEvents([]),
                 self.__genMouseEventFrames(avg.CURSORUP, 30, 30, []),
                 
                 # Remove node while hold is in progress.
                 self.__genMouseEventFrames(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                 self.__killImageNode,
                 self.delay(1100),
                 self.__genMouseEventFrames(avg.CURSORUP, 30, 30, []),
                ))
        player.setFakeFPS(-1)


    def testDoubletapRecognizer(self):

        def onPossible(event):
            self.__addEventFlag(EVENT_POSSIBLE)

        def onDetected(event):
            self.__addEventFlag(EVENT_DETECTED)

        def onFail(event):
            self.__addEventFlag(EVENT_FAILED)

        def abort():
            self.__tapRecognizer.abort()
            self.__resetEventState()

        def enable(isEnabled):
            self.__tapRecognizer.enable(isEnabled)
            self.__resetEventState()

        root = self.loadEmptyScene()
        image = avg.ImageNode(parent=root, href="rgb24-64x64.png", size=(128,128))
        self.__tapRecognizer = ui.DoubletapRecognizer(image,
                possibleHandler=onPossible,
                detectedHandler=onDetected,
                failHandler=onFail)
        self.__resetEventState()
        player.setFakeFPS(20)
        self.start(False,
                (# Down, up, down, up: click
                 self.__genMouseEventFrames(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                 self.__genMouseEventFrames(avg.CURSORUP, 30, 30, []),
                 self.__genMouseEventFrames(avg.CURSORDOWN, 30, 30, []),
                 self.__genMouseEventFrames(avg.CURSORUP, 30, 30, [EVENT_DETECTED]),
                 # Down, move: stop
                 self.__genMouseEventFrames(avg.CURSORDOWN, 0, 30, [EVENT_POSSIBLE]),
                 self.__genMouseEventFrames(avg.CURSORMOTION, 80, 30, [EVENT_FAILED]),
                 self.__genMouseEventFrames(avg.CURSORUP, 0, 30, []),
                 # Down, up, move: stop
                 self.__genMouseEventFrames(avg.CURSORDOWN, 0, 30, [EVENT_POSSIBLE]),
                 self.__genMouseEventFrames(avg.CURSORUP, 0, 30, []),
                 self.__genMouseEventFrames(avg.CURSORMOTION, 80, 30, []),
                 self.__genMouseEventFrames(avg.CURSORDOWN, 80, 30, [EVENT_FAILED]),
                 self.__genMouseEventFrames(avg.CURSORUP, 0, 30, []),
                 # Down, up, down, move: stop
                 self.__genMouseEventFrames(avg.CURSORDOWN, 0, 30, [EVENT_POSSIBLE]),
                 self.__genMouseEventFrames(avg.CURSORUP, 0, 30, []),
                 self.__genMouseEventFrames(avg.CURSORDOWN, 0, 30, []),
                 self.__genMouseEventFrames(avg.CURSORMOTION, 80, 30, [EVENT_FAILED]),
                 self.__genMouseEventFrames(avg.CURSORUP, 0, 30, []),
                 # Down,delay: stop
                 self.__genMouseEventFrames(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                 lambda: self.delay(1000),
                 lambda: self.__assertEvents([EVENT_FAILED]),
                 self.__genMouseEventFrames(avg.CURSORUP, 30, 30, [EVENT_FAILED]),
                 # Down, up, delay: stop
                 self.__genMouseEventFrames(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                 self.__genMouseEventFrames(avg.CURSORUP, 30, 30, []),
                 lambda: self.delay(1000),
                 lambda: self.__assertEvents([EVENT_FAILED]),
                 # Down, up, down, delay: stop
                 self.__resetEventState,
                 self.__genMouseEventFrames(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                 self.__genMouseEventFrames(avg.CURSORUP, 30, 30, []),
                 self.__genMouseEventFrames(avg.CURSORDOWN, 30, 30, []),
                 lambda: self.delay(1000),
                 lambda: self.__assertEvents([EVENT_FAILED]),
                 self.__genMouseEventFrames(avg.CURSORUP, 30, 30, [EVENT_FAILED]),
                 # Down, abort, up, down, up, delay: just one click
                 self.__genMouseEventFrames(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                 abort,
                 self.__genMouseEventFrames(avg.CURSORUP, 30, 30, []),
                 self.__genMouseEventFrames(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                 self.__genMouseEventFrames(avg.CURSORUP, 30, 30, []),
                 lambda: self.delay(1000),
                 lambda: self.__assertEvents([EVENT_FAILED]),
                 # Down, up, abort, down, up, delay: two clicks but no double-click
                 self.__resetEventState,
                 self.__genMouseEventFrames(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                 self.__genMouseEventFrames(avg.CURSORUP, 30, 30, []),
                 abort,
                 self.__genMouseEventFrames(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                 self.__genMouseEventFrames(avg.CURSORUP, 30, 30, []),
                 lambda: self.delay(1000),
                 lambda: self.__assertEvents([EVENT_FAILED]),
                 # Down, up, down, abort, up: just one click
                 self.__resetEventState,
                 self.__genMouseEventFrames(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                 self.__genMouseEventFrames(avg.CURSORUP, 30, 30, []),
                 self.__genMouseEventFrames(avg.CURSORDOWN, 30, 30, []),
                 abort,
                 self.__genMouseEventFrames(avg.CURSORUP, 30, 30, []),
                 # Down, abort, up, down, up, down up: first aborted then recognized
                 self.__genMouseEventFrames(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                 abort,
                 self.__genMouseEventFrames(avg.CURSORUP, 30, 30, []),
                 self.__genMouseEventFrames(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                 self.__genMouseEventFrames(avg.CURSORUP, 30, 30, []),
                 self.__genMouseEventFrames(avg.CURSORDOWN, 30, 30, []),
                 self.__genMouseEventFrames(avg.CURSORUP, 30, 30, [EVENT_DETECTED]),
                 # Disabled, down, up, down, up, enabled: nothing
                 lambda: enable(False),
                 self.__genMouseEventFrames(avg.CURSORDOWN, 30, 30, []),
                 self.__genMouseEventFrames(avg.CURSORUP, 30, 30, []),
                 self.__genMouseEventFrames(avg.CURSORDOWN, 30, 30, []),
                 self.__genMouseEventFrames(avg.CURSORUP, 30, 30, []),
                 lambda: enable(True),                 
                 # Down, disabled up, down, up, enabled: just one down
                 self.__genMouseEventFrames(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                 lambda: enable(False),
                 self.__genMouseEventFrames(avg.CURSORUP, 30, 30, []),
                 self.__genMouseEventFrames(avg.CURSORDOWN, 30, 30, []),
                 self.__genMouseEventFrames(avg.CURSORUP, 30, 30, []),
                 lambda: enable(True),
                 # Down, up, disabled, down, up, enabled: just one click
                 self.__genMouseEventFrames(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                 self.__genMouseEventFrames(avg.CURSORUP, 30, 30, []),
                 lambda: enable(False),
                 self.__genMouseEventFrames(avg.CURSORDOWN, 30, 30, []),
                 self.__genMouseEventFrames(avg.CURSORUP, 30, 30, []),
                 lambda: enable(True),
                 # Down, up, down, disabled, up, enabled: just one click
                 self.__genMouseEventFrames(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                 self.__genMouseEventFrames(avg.CURSORUP, 30, 30, []),
                 self.__genMouseEventFrames(avg.CURSORDOWN, 30, 30, []),
                 lambda: enable(False),
                 self.__genMouseEventFrames(avg.CURSORUP, 30, 30, []),
                 lambda: enable(True),
                 # Down, disabled, enabled, up, down, up, down, up: recognized
                 self.__genMouseEventFrames(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                 lambda: enable(False),
                 lambda: enable(True),
                 self.__genMouseEventFrames(avg.CURSORUP, 30, 30, []),
                 self.__genMouseEventFrames(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                 self.__genMouseEventFrames(avg.CURSORUP, 30, 30, []),
                 self.__genMouseEventFrames(avg.CURSORDOWN, 30, 30, []),
                 self.__genMouseEventFrames(avg.CURSORUP, 30, 30, [EVENT_DETECTED]),
                ))


    def testDragRecognizer(self):

        def onDetected(event):
            self.__addEventFlag(EVENT_DETECTED)

        def onMove(event, offset):
            if self.friction == -1:
                self.assertEqual(offset, (40,40))
            self.__addEventFlag(EVENT_MOVED)

        def onUp(event, offset):
            if self.friction == -1:
                self.assertEqual(offset, (10,-10))
            self.__addEventFlag(EVENT_UP)

        def onEnd(event):
            self.__addEventFlag(EVENT_ENDED)

        def enable(isEnabled):
            dragRecognizer.enable(isEnabled)
            self.__resetEventState()

        def abort():
            dragRecognizer.abort()
            self.__resetEventState()

        player.setFakeFPS(100)
        sys.stderr.write("\n")
        for self.friction in (-1, 100):
            if self.friction == -1:
                sys.stderr.write("  Simple drag, no inertia\n")
            else:
                sys.stderr.write("  Simple drag, inertia\n")
            self.__initImageScene()
            dragRecognizer = ui.DragRecognizer(self.image, 
                    detectedHandler=onDetected, moveHandler=onMove, upHandler=onUp, 
                    endHandler=onEnd, friction=self.friction)
            self.__resetEventState()
            self.start(False,
                    (self.__genMouseEventFrames(avg.CURSORDOWN, 30, 30, [EVENT_DETECTED]),
                     self.__genMouseEventFrames(avg.CURSORMOTION, 70, 70, [EVENT_MOVED]),
                     self.__genMouseEventFrames(avg.CURSORUP, 40, 20, 
                            [EVENT_UP, EVENT_ENDED]),
                     lambda: enable(False),
                     self.__genMouseEventFrames(avg.CURSORDOWN, 30, 30, []),
                     lambda: dragRecognizer.enable(True),
                     self.__genMouseEventFrames(avg.CURSORUP, 30, 30, []),
                     self.__genMouseEventFrames(avg.CURSORDOWN, 30, 30, [EVENT_DETECTED]),
                     self.__genMouseEventFrames(avg.CURSORUP, 40, 20, 
                            [EVENT_UP, EVENT_ENDED]),

                     # Remove node during drag.
                     self.__genMouseEventFrames(avg.CURSORDOWN, 30, 30, [EVENT_DETECTED]),
                     self.__killImageNode,
                     self.__genMouseEventFrames(avg.CURSORUP, 30, 30, []),
                    ))

        # Test with constraint.
        def onPossible(event):
            self.__addEventFlag(EVENT_POSSIBLE)

        def onFail(event):
            self.__addEventFlag(EVENT_FAILED)

        def onVertMove(event, offset):
            if self.friction == -1:
                self.assertEqual(offset, (0,40))
            self.__addEventFlag(EVENT_MOVED)

        for self.friction in (-1, 100):
            if self.friction == -1:
                sys.stderr.write("  Drag with constraint, no inertia\n")
            else:
                sys.stderr.write("  Drag with constraint, inertia\n")
            self.__initImageScene()
            dragRecognizer = ui.DragRecognizer(self.image, 
                    possibleHandler=onPossible, failHandler=onFail, 
                    detectedHandler=onDetected, 
                    moveHandler=onVertMove, upHandler=onUp, endHandler=onEnd, 
                    friction=self.friction, direction=ui.DragRecognizer.VERTICAL)
            self.__resetEventState()
            self.start(False,
                    (self.__genMouseEventFrames(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                     self.__genMouseEventFrames(avg.CURSORMOTION, 35, 30, []),
                     self.__genMouseEventFrames(avg.CURSORMOTION, 30, 70, 
                            [EVENT_DETECTED, EVENT_MOVED]),
                     self.__genMouseEventFrames(avg.CURSORUP, 40, 20, 
                            [EVENT_UP, EVENT_ENDED]),
                     # Wrong direction -> stop.
                     self.__genMouseEventFrames(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                     self.__genMouseEventFrames(avg.CURSORMOTION, 70, 30, [EVENT_FAILED]),
                     self.__genMouseEventFrames(avg.CURSORUP, 70, 30, []),

                     # No movement -> stop.
                     self.__genMouseEventFrames(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                     self.__genMouseEventFrames(avg.CURSORUP, 30, 30, [EVENT_FAILED]),

                     # Down, Abort, Motion, Motion, Up -> not recognized
                     self.__genMouseEventFrames(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                     abort,
                     self.__genMouseEventFrames(avg.CURSORMOTION, 35, 30, []),
                     self.__genMouseEventFrames(avg.CURSORMOTION, 30, 70, []),
                     self.__genMouseEventFrames(avg.CURSORUP, 40, 20, []),

                     # Down, Motion, Abort, Motion, Up -> not Recognized
                     self.__genMouseEventFrames(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                     self.__genMouseEventFrames(avg.CURSORMOTION, 35, 30, []),
                     abort,
                     self.__genMouseEventFrames(avg.CURSORMOTION, 30, 70, []),
                     self.__genMouseEventFrames(avg.CURSORUP, 40, 20, []),

                     # Down, Motion, Motion, Abort, Up -> not recognized
                     self.__genMouseEventFrames(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                     self.__genMouseEventFrames(avg.CURSORMOTION, 35, 30, []),
                     self.__genMouseEventFrames(avg.CURSORMOTION, 30, 70,
                            [EVENT_DETECTED, EVENT_MOVED]),
                     abort,
                     self.__genMouseEventFrames(avg.CURSORUP, 40, 20, []),

                     # Down, Motion, Abort, Up, Down, Motion, Motion, Up -> Recognized
                     self.__genMouseEventFrames(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                     self.__genMouseEventFrames(avg.CURSORMOTION, 35, 30, []),
                     abort,
                     self.__genMouseEventFrames(avg.CURSORUP, 40, 20, []),
                     
                     self.__genMouseEventFrames(avg.CURSORDOWN, 30, 30, [EVENT_POSSIBLE]),
                     self.__genMouseEventFrames(avg.CURSORMOTION, 35, 30, []),
                     self.__genMouseEventFrames(avg.CURSORMOTION, 30, 70,
                            [EVENT_DETECTED, EVENT_MOVED]),
                     self.__genMouseEventFrames(avg.CURSORUP, 40, 20, 
                            [EVENT_UP, EVENT_ENDED]),
                    ))

        # Test second down during inertia.
        sys.stderr.write("  Down during inertia\n")
        self.__initImageScene()
        dragRecognizer = ui.DragRecognizer(self.image, 
                possibleHandler=onPossible, failHandler=onFail, 
                detectedHandler=onDetected, 
                moveHandler=onMove, upHandler=onUp, endHandler=onEnd, 
                friction=0.01)
        self.__resetEventState()
        self.start(False,
                (lambda: self._sendMouseEvent(avg.CURSORDOWN, 30, 30),
                 lambda: self._sendMouseEvent(avg.CURSORUP, 40, 20),
                 self.__resetEventState,
                 self.__genMouseEventFrames(avg.CURSORDOWN, 40, 20, 
                            [EVENT_ENDED, EVENT_DETECTED, 
                             EVENT_MOVED]),
                ))

        # Test node delete during inertia
        sys.stderr.write("  Delete during inertia\n")
        self.__initImageScene()
        dragRecognizer = ui.DragRecognizer(self.image, 
                possibleHandler=onPossible, failHandler=onFail, 
                detectedHandler=onDetected, 
                moveHandler=onMove, upHandler=onUp, endHandler=onEnd, 
                friction=0.01)
        self.__resetEventState()
        self.start(False,
                (self.__genMouseEventFrames(avg.CURSORDOWN, 30, 30, [EVENT_DETECTED]),
                 self.__genMouseEventFrames(avg.CURSORUP, 40, 20, 
                        [EVENT_MOVED, EVENT_UP]),
                 self.__killImageNode,
                 self.__genMouseEventFrames(avg.CURSORDOWN, 40, 20, [EVENT_MOVED]),
                ))

        # Test second down during inertia, constrained recognizer
        sys.stderr.write("  Down during inertia, constrained recognizer\n")
        self.__initImageScene()
        dragRecognizer = ui.DragRecognizer(self.image, 
                possibleHandler=onPossible, failHandler=onFail, 
                detectedHandler=onDetected, 
                moveHandler=onMove, upHandler=onUp, endHandler=onEnd, 
                friction=0.01, direction=ui.DragRecognizer.VERTICAL)
        self.__resetEventState()
        self.start(False,
                (lambda: self._sendMouseEvent(avg.CURSORDOWN, 30, 30),
                 self.__genMouseEventFrames(avg.CURSORMOTION, 30, 70,
                        [EVENT_DETECTED, EVENT_MOVED, 
                         EVENT_POSSIBLE]),
                 self.__genMouseEventFrames(avg.CURSORUP, 30, 70,
                        [EVENT_MOVED, EVENT_UP]),
                 self.__genMouseEventFrames(avg.CURSORDOWN, 30, 30, 
                        [EVENT_MOVED, EVENT_ENDED, 
                         EVENT_POSSIBLE]),
                 self.__genMouseEventFrames(avg.CURSORMOTION, 30, 70, 
                        [EVENT_DETECTED, EVENT_MOVED]),
                ))

        player.setFakeFPS(-1)


    def testDragRecognizerRelCoords(self):

        def onDrag(event, offset):
            self.assertAlmostEqual(offset, (-40,-40))

        player.setFakeFPS(100)
        for self.friction in (-1, 100):
            root = self.loadEmptyScene()
            div = avg.DivNode(pos=(64,64), angle=math.pi, parent=root)
            image = avg.ImageNode(parent=div, href="rgb24-64x64.png")
            ui.DragRecognizer(image, moveHandler=onDrag, friction=self.friction)
            self.start(False,
                    (lambda: self._sendMouseEvent(avg.CURSORDOWN, 30, 30),
                     lambda: self._sendMouseEvent(avg.CURSORMOTION, 70, 70),
                    ))
        player.setFakeFPS(-1)


    def testDragRecognizerInitialEvent(self):

        def onMotion(event):
            ui.DragRecognizer(self.image, 
                    detectedHandler=onDragStart, moveHandler=onDrag, initialEvent=event)
            self.image.disconnectEventHandler(self)

        def onDragStart(event):
            self.__dragStartCalled = True

        def onDrag(event, offset):
            self.assertEqual(offset, (10,0))

        self.__initImageScene()
        self.image.connectEventHandler(avg.CURSORMOTION, avg.MOUSE, self, onMotion)
        self.__dragStartCalled = False
        self.start(False,
                (lambda: self._sendMouseEvent(avg.CURSORDOWN, 30, 30),
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
        self.start(False,
                (lambda: self._sendMouseEvent(avg.CURSORDOWN, 30, 30),
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

        self.__initImageScene()
        self.__transformRecognizer = ui.TransformRecognizer(self.image, 
                detectedHandler=onDetected, moveHandler=onMove, upHandler=onUp)
        self.start(False,
                (# Check up/down handling
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

                 createScaleTestFrames(ui.Transform((0,5), 0, 2, (0,20))),

                 # Delete node during transform
                 lambda: self._sendTouchEvents((
                        (1, avg.CURSORDOWN, 30, 10),
                        (2, avg.CURSORDOWN, 30, 20))),
                 self.__killImageNode,
                 lambda: self._sendTouchEvents((
                        (1, avg.CURSORUP, 30, 10),
                        (2, avg.CURSORUP, 30, 20))),
                ))

        # Test rel. coords.
        root = self.loadEmptyScene()
        div = avg.DivNode(parent=root, pos=(0,10))
        image = avg.ImageNode(parent=div, href="rgb24-64x64.png")
        self.__transformRecognizer = ui.TransformRecognizer(image, 
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
                detectedHandler=onDetected, moveHandler=onMove, upHandler=onUp)
        self.start(False,
                (createTransTestFrames(),
                 createRotTestFrames(ui.Transform((0,0), math.pi, 1, (0,15))),
                 createScaleTestFrames(ui.Transform((0,5), 0, 2, (0,20))),
                ))

        root = self.loadEmptyScene()
        div = avg.DivNode(parent=root, pos=(0,10))
        image = avg.ImageNode(parent=div, href="rgb24-64x64.png")
        self.__transformRecognizer = ui.TransformRecognizer(image, friction=0.01,
                detectedHandler=onDetected, moveHandler=onMove, upHandler=onUp)
        self.start(False,
                (lambda: self._sendTouchEvent(1, avg.CURSORDOWN, 10, 10),
                 lambda: self._sendTouchEvent(1, avg.CURSORUP, 20, 10),
                 lambda: self._sendTouchEvent(1, avg.CURSORDOWN, 10, 10),
                 lambda: self._sendTouchEvent(1, avg.CURSORUP, 20, 10),
                ))  

        # Test abort
        self.__initImageScene()
        self.__transformRecognizer = ui.TransformRecognizer(self.image, 
                detectedHandler=onDetected, moveHandler=onMove, upHandler=onUp)
        self.start(False,
                (self.__transformRecognizer.abort,
                 lambda: self._sendTouchEvent(1, avg.CURSORDOWN, 10, 10),
                 self.__transformRecognizer.abort,
                 lambda: self._sendTouchEvent(1, avg.CURSORMOTION, 20, 10),
                 lambda: self._sendTouchEvent(1, avg.CURSORUP, 30, 10),
                 lambda: checkTransform(ui.Transform((0,0))),
                 self.__transformRecognizer.abort,
                ))

        # Test enable/disable
        self.__initImageScene()
        self.__transformRecognizer = ui.TransformRecognizer(self.image, 
                detectedHandler=onDetected, moveHandler=onMove, upHandler=onUp)
        self.start(False,
                (# Regular disable
                 lambda: self.__transformRecognizer.enable(False),
                 lambda: self._sendTouchEvent(1, avg.CURSORDOWN, 10, 10),
                 lambda: self._sendTouchEvent(1, avg.CURSORMOTION, 20, 10),
                 lambda: self._sendTouchEvent(1, avg.CURSORUP, 30, 10),
                 lambda: checkTransform(ui.Transform((0,0))),
                 # Re-enable
                 lambda: self.__transformRecognizer.enable(True),
                 lambda: self._sendTouchEvent(1, avg.CURSORDOWN, 10, 10),
                 lambda: self._sendTouchEvent(1, avg.CURSORUP, 20, 10),
                 lambda: checkTransform(ui.Transform((10,0))),
                 # Disable during gesture
                 lambda: self._sendTouchEvent(1, avg.CURSORDOWN, 10, 10),
                 lambda: self.__transformRecognizer.enable(False),
                 lambda: self._sendTouchEvent(1, avg.CURSORUP, 20, 10),
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

    def __genMouseEventFrames(self, type, x, y, expectedEvents):
        return [
                 lambda: self._sendMouseEvent(type, x, y),
                 lambda: self.__assertEvents(expectedEvents),
                 self.__resetEventState
                ]
    
    def __assertEvents(self, expectedFlags):
        expectedFlags = Set(expectedFlags)
        if expectedFlags != self.__flags:
            sys.stderr.write("\nState expected: "+str(expectedFlags)+"\n")
            sys.stderr.write("Actual state: "+str(self.__flags)+"\n")
            self.assert_(False)

    def __resetEventState(self):
        self.__flags = Set()

    def __addEventFlag(self, flag):
        self.__flags.add(flag)

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
        "testTransformRecognizer",
        "testKMeans",
        "testMat3x3",
        )

    return createAVGTestSuite(availableTests, GestureTestCase, tests)
