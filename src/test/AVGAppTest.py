#!/usr/bin/python
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

import os
import time

import libavg
from libavg import avg, Point2D, player
import testcase

g_helper = player.getTestHelper()

TEST_RESOLUTION = (160, 120)

class TestAppBase(libavg.AVGApp):
    @classmethod
    def start(cls, **kwargs):
        with testcase.SuppressOutput():
            super(TestAppBase, cls).start(**kwargs)

    def requestStop(self, timeout=0):
        player.setTimeout(timeout, player.stop)

    def singleKeyPress(self, char):
        g_helper.fakeKeyEvent(avg.Event.KEY_DOWN, ord(char), ord(char), char, ord(char), 
                avg.KEYMOD_NONE)
        g_helper.fakeKeyEvent(avg.Event.KEY_UP, ord(char), ord(char), char, ord(char), 
                avg.KEYMOD_NONE)


class AVGAppTestCase(testcase.AVGTestCase):
    def testMinimal(self):
        class MinimalApp(TestAppBase):
            testInstance = self
            def init(self):
                self.testInstance.assert_(not player.isFullscreen())
                self.requestStop()

        if 'AVG_DEPLOY' in os.environ:
            del os.environ['AVG_DEPLOY']
        MinimalApp.start(resolution=TEST_RESOLUTION)
    
    def testAvgDeploy(self):
        class FullscreenApp(TestAppBase):
            testInstance = self
            def init(self):
                self.testInstance.assert_(player.isFullscreen())
                rootNodeSize = player.getRootNode().size
                self.testInstance.assertEqual(rootNodeSize, resolution)
                self.requestStop()
                
        resolution = player.getScreenResolution()
        os.environ['AVG_DEPLOY'] = '1'
        FullscreenApp.start(resolution=resolution)
        del os.environ['AVG_DEPLOY']

    def testDebugWindowSize(self):
        class DebugwindowApp(TestAppBase):
            testInstance = self
            def init(self):
                self.testInstance.assert_(not player.isFullscreen())
                rootNodeSize = player.getRootNode().size
                self.testInstance.assertEqual(rootNodeSize, TEST_RESOLUTION)
                
                # windowSize = player.getWindowResolution()
                # self.testInstance.assertEqual(windowSize, Point2D(TEST_RESOLUTION)/2)
                self.requestStop()
        
        DebugwindowApp.start(resolution=TEST_RESOLUTION,
                debugWindowSize=Point2D(TEST_RESOLUTION) / 2)
    
    def testScreenshot(self):
        if not(self._isCurrentDirWriteable()):
            self.skip("Current dir not writeable")
            return
            
        expectedFiles = ['screenshot-000.png', 'screenshot-001.png']

        def cleanup():
            for screenshotFile in expectedFiles[::-1]:
                if os.path.exists(screenshotFile):
                    os.unlink(screenshotFile)
            
        def checkCallback():
            for screenshotFile in expectedFiles[::-1]:
                if os.path.exists(screenshotFile):
                    avg.Bitmap(screenshotFile)
                else:
                    raise RuntimeError('Cannot find the expected '
                            'screenshot file %s' % screenshotFile)
            
            player.stop()
            
        class ScreenshotApp(TestAppBase):
            def init(self):
                self.singleKeyPress('s')
                self.singleKeyPress('s')
                self.timeStarted = time.time()
                self.timerId = player.subscribe(player.ON_FRAME, self.onFrame)
            
            def onFrame(self):
                if (os.path.exists(expectedFiles[-1]) or
                        time.time() - self.timeStarted > 1):
                    player.clearInterval(self.timerId)
                    checkCallback()
        
        cleanup()
        ScreenshotApp.start(resolution=TEST_RESOLUTION)
        cleanup()
    
    def testGraphs(self):
        class GraphsApp(TestAppBase):
            def init(self):
                self.enableGraphs()
            
            def enableGraphs(self):
                self.singleKeyPress('f')
                self.singleKeyPress('m')
                player.setTimeout(500, self.disableGraphs)
                
            def disableGraphs(self):
                self.singleKeyPress('m')
                self.singleKeyPress('f')
                self.requestStop()
        
        GraphsApp.start(resolution=TEST_RESOLUTION)
    
    def testToggleKeys(self):
        TOGGLE_KEYS = ['?', 't', 'e']
        class ToggleKeysApp(TestAppBase):
            def init(self):
                self.keys = TOGGLE_KEYS[:]
                player.setTimeout(0, self.nextKey)
            
            def nextKey(self):
                if not self.keys:
                    player.stop()
                else:
                    key = self.keys.pop()
                    self.singleKeyPress(key)
                    player.setTimeout(0, self.nextKey)
    
        ToggleKeysApp.start(resolution=TEST_RESOLUTION)
    
    def testFakeFullscreen(self):
        class FakeFullscreenApp(TestAppBase):
            fakeFullscreen = True
            def init(self):
                player.setTimeout(0, player.stop)
              
        resolution = player.getScreenResolution()
        if os.name == 'nt':
            FakeFullscreenApp.start(resolution=resolution)
        else:
            self.assertRaises(RuntimeError,
                    lambda: FakeFullscreenApp.start(resolution=resolution))
        
def avgAppTestSuite(tests):
    availableTests = (
            'testMinimal',
            'testAvgDeploy',
            'testDebugWindowSize',
            'testScreenshot',
            'testGraphs',
            'testToggleKeys',
            'testFakeFullscreen',
    )
    return testcase.createAVGTestSuite(availableTests, AVGAppTestCase, tests)
