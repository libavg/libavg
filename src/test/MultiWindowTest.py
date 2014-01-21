#!/usr/bin/env python
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


from libavg import avg, player
from testcase import *


class MultiWindowTestCase(AVGTestCase):
    def __init__(self, testFuncName):
        AVGTestCase.__init__(self, testFuncName)

    def testMultiWindowBase(self):
        root = self.loadEmptyScene()
        avg.ImageNode(pos=(0,0), href="rgb24-64x64.png", parent=root)
        player.setWindowConfig("avgwindowconfig.xml")
        self.start(False,
                (None,
                 lambda: self.delay(10000),
                ))


def multiWindowTestSuite(tests):
    availableTests = (
            "testMultiWindowBase",
            )
    return createAVGTestSuite(availableTests, MultiWindowTestCase, tests)
