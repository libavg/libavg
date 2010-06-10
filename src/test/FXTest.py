#!/usr/bin/env python
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

import math

from libavg import avg
from testcase import *

class FXTestCase(AVGTestCase):
    def __init__(self, testFuncName):
        AVGTestCase.__init__(self, testFuncName)

    def testFXBasics(self):
        def activateFX():
            node2.setEffect(avg.NullFXNode())

        def newNode():
            node = avg.ImageNode(parent=root, href="rgb24-32x32.png", pos=(64,0))
            node.setEffect(avg.NullFXNode())

        self.loadEmptyScene()
        root = Player.getRootNode()
        node = avg.ImageNode(parent=root, href="rgb24-32x32.png")
        node.setEffect(avg.NullFXNode())
        node = avg.ImageNode(parent=root, href="rgb24alpha-32x32.png", pos=(0,32))
        node.setEffect(avg.NullFXNode())
        node2 = avg.ImageNode(parent=root, href="rgb24-32x32.png", pos=(32,0))
        self.start(None,
                (lambda: self.compareImage("testFXBasics1", False),
                 activateFX,
                 lambda: self.compareImage("testFXBasics1", False),
                 newNode,
                 lambda: self.compareImage("testFXBasics2", False),
                ))

def fxTestSuite(tests):
    availableTests = (
            "testFXBasics",
            )
    return createAVGTestSuite(availableTests, FXTestCase, tests)

Player = avg.Player.get()
