#!/usr/bin/python
# -*- coding: utf-8 -*-
# libavg - Media Playback Engine.
# Copyright (C) 2003-2013 Ulrich von Zadow
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
# Original author of this file is OXullo Interecans <x at brainrapers dot org>
# Sponsored by Archimedes Exhibitions GmbH ( http://www.archimedes-exhibitions.de )


import os
import sys
import libavg
from libavg import player
from libavg.app import settings
from libavg.app import keyboardmanager
from libavg.app.settings import Option
import testcase


class TestApp(libavg.app.App):
    def testRun(self, onFrameHandlersList=[], mainScene=None):
        assert type(onFrameHandlersList) == list
        self.__onFrameHandlersList = onFrameHandlersList
        player.subscribe(player.ON_FRAME, self.__onFrame)
        player.setFramerate(10000)
        player.assumePixelsPerMM(1)
        self.settings.set('app_resolution', '160x120')
        self.settings.set('app_window_size', '160x120')

        if mainScene is None:
            mainScene = libavg.app.MainScene()

        self.run(mainScene)

    def __onFrame(self):
        if self.__onFrameHandlersList:
            todo = self.__onFrameHandlersList.pop(0)
            todo()
        else:
            player.stop()


class AppTestCase(testcase.AVGTestCase):
    def testSettingsOptions(self):
        self.assertException(lambda: settings.Option('test', 1), ValueError)
        
        self.assertException(lambda: settings.Settings(
                [Option('foo', 'bar'), Option('foo', 'bar')]), RuntimeError)

        s = settings.Settings([Option('foo', 'bar')])
        self.assertException(lambda: s.addOption(Option('foo', 'baz')))
        
    def testSettingsTypes(self):
        defaults = [
                Option('test_boolean', 'True', 'help'),
                Option('test_string', 'string'),
                Option('another_value_int', '1234'),
                Option('test_2d', '1280x1024'),
                Option('test_2d_alt','1280,1024'),
                Option('test_float','12.345'),
                Option('test_json','[1, null,3 , "string", 12.345]')
        ]

        s = settings.Settings(defaults)

        self.assertEquals(s.getboolean('test_boolean'), True)

        self.assertEquals(type(s.get('test_string')), str)
        self.assertException(lambda: s.getboolean('test_string'), ValueError)

        self.assertEquals(s.getint('another_value_int'), 1234)
        self.assertException(lambda: s.getint('test_string'), ValueError)
        self.assertEquals(s.get('another_value_int'), '1234')

        self.assertEquals(s.getpoint2d('test_2d'), libavg.Point2D(1280, 1024))
        self.assertEquals(s.getpoint2d('test_2d_alt'), libavg.Point2D(1280, 1024))
        self.assertException(lambda: s.getint('test_2d'), ValueError)

        self.assertEquals(s.getfloat('test_float'), 12.345)
        
        self.assertEquals(s.getjson('test_json'), [1, None, 3, 'string', 12.345])

    def testSettingsSet(self):
        s = settings.Settings()
        s.addOption(Option('test_value', ''))
        self.assertException(lambda: s.set('test_value', 1234), ValueError)

        s.set('test_value', '1234')
        self.assertEquals(s.getint('test_value'), 1234)

    def testSettingsArgvExtender(self):
        s = settings.Settings([Option('foo_bar', 'bar')])
        e = settings.ArgvExtender(args=['foo', '--foo-bar', 'baz'])
        s.applyExtender(e)
        self.assertEquals(s.get('foo_bar'), 'baz')

    def testAppAdditionalSettings(self):
        app = TestApp()
        app.settings.addOption(Option('foo_bar', 'baz'))
        app.settings.addOption(Option('bar_foo', 'baz'))
        self.assertEquals(app.settings.get('foo_bar'), 'baz')

    def testAppInstance(self):
        app = TestApp()
        self.assertEquals(app, libavg.app.instance)

    def testAppResolution(self):
        app = TestApp()
        app.testRun([
                lambda: self.assertEquals(player.getRootNode().size, (160, 120)),
                lambda: self.assert_(not player.isFullscreen()),
                ])

    def testAppFullscreen(self):
        app = TestApp()
        app.settings.set('app_fullscreen', 'true')
        app.testRun([
                lambda: self.assert_(player.isFullscreen()),
                ])

    def testAppRotation(self):
        app = TestApp()
        app.settings.set('app_rotation', 'left')
        app.testRun([
                lambda: self.assertEquals(player.getRootNode().size, (120, 160)),
                ])

    def testScreenshot(self):
        expectedFiles = ['TestApp-001.png', 'TestApp-002.png']
        
        def removeFiles():
            for file in expectedFiles:
                if os.path.exists(file):
                    os.unlink(file)

        def testScreenshots():
            for file in expectedFiles:
                self.assert_(os.path.exists(file))

        removeFiles()
        app = TestApp()
        app.testRun([
                app.takeScreenshot,
                app.takeScreenshot,
                testScreenshots,
                removeFiles,
                ])

    def testKeyboardManager(self):
        statesRecords = [False, False]
        def keyDownPressed():
            statesRecords[0] = True
        
        def keyUpPressed():
            statesRecords[1] = True
            
        app = TestApp()
        app.testRun([
                lambda: keyboardmanager.bindKeyDown('a', keyDownPressed, ''),
                lambda: keyboardmanager.bindKeyUp('a', keyUpPressed, ''),
                lambda: self.__emuKeyPress('a'),
                lambda: self.assert_(all(statesRecords)),
                ])
                
    def tearDown(self):
        libavg.app.instance = None

    def __emuKeyPress(self, char):
        helper = libavg.player.getTestHelper()
        helper.fakeKeyEvent(libavg.avg.Event.KEY_DOWN, ord(char), ord(char), char,
                ord(char), libavg.avg.KEYMOD_NONE)
        helper.fakeKeyEvent(libavg.avg.Event.KEY_UP, ord(char), ord(char), char,
                ord(char), libavg.avg.KEYMOD_NONE)


def appTestSuite(tests):
    availableTests = (
            'testSettingsOptions',
            'testSettingsTypes',
            'testSettingsSet',
            'testSettingsArgvExtender',
            'testAppAdditionalSettings',
            'testAppInstance',
            'testAppResolution',
            'testAppFullscreen',
            'testAppRotation',
            'testScreenshot',
            'testKeyboardManager',
    )
    return testcase.createAVGTestSuite(availableTests, AppTestCase, tests)

