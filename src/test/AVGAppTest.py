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

import sys
import os

import libavg
from libavg import avg, Point2D
import testcase

g_player = avg.Player.get()

TEST_RESOLUTION = (160, 120)

class TestAppBase(libavg.AVGApp):
    def init(self):
        g_player.setTimeout(0, g_player.stop)


class AVGAppTestCase(testcase.AVGTestCase):
    def testMinimal(self):
        class MinimalApp(TestAppBase):
            testInstance = self
            def init(self):
                self.testInstance.assert_(not g_player.isFullscreen())
                super(MinimalApp, self).init()

        if 'AVG_DEPLOY' in os.environ:
            del os.environ['AVG_DEPLOY']
        MinimalApp.start(resolution=TEST_RESOLUTION)
    
    def testAvgDeploy(self):
        class FullscreenApp(TestAppBase):
            testInstance = self
            def init(self):
                self.testInstance.assert_(g_player.isFullscreen())
                rootNodeSize = g_player.getRootNode().size
                self.testInstance.assert_(rootNodeSize == TEST_RESOLUTION)
                super(FullscreenApp, self).init()
                
        os.environ['AVG_DEPLOY'] = '1'
        FullscreenApp.start(resolution=TEST_RESOLUTION)
        del os.environ['AVG_DEPLOY']

    def testDebugWindowSize(self):
        class DebugwindowApp(TestAppBase):
            testInstance = self
            def init(self):
                self.testInstance.assert_(not g_player.isFullscreen())
                rootNodeSize = g_player.getRootNode().size
                self.testInstance.assert_(rootNodeSize == TEST_RESOLUTION)
                
                windowSize = g_player.getWindowResolution()
                self.testInstance.assert_(windowSize == Point2D(TEST_RESOLUTION) / 2)

                super(DebugwindowApp, self).init()
        
        DebugwindowApp.start(resolution=TEST_RESOLUTION,
                debugWindowSize=Point2D(TEST_RESOLUTION) / 2)
        
        
def avgAppTestSuite(tests):
    availableTests = (
            'testMinimal',
            'testAvgDeploy',
            'testDebugWindowSize',
    )
    return testcase.createAVGTestSuite(availableTests, AVGAppTestCase, tests)
