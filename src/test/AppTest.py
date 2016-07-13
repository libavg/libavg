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
# Original author of this file is OXullo Interecans <x at brainrapers dot org>


import os
import tempfile
import libavg
from libavg import avg, player
from libavg.app import settings
from libavg.app import keyboardmanager
from libavg.app.settings import Option
from libavg.testcase import *

res = player.getScreenResolution()

class TestApp(libavg.app.App):
    CUSTOM_SETTINGS = {'app_resolution': '160x120', 'app_window_size': '160x120'}

    def testRun(self, onFrameHandlersList=[], mainDiv=None, runtimeOptions={}):
        assert type(onFrameHandlersList) == list
        self.__onFrameHandlersList = onFrameHandlersList
        player.subscribe(player.ON_FRAME, self.__onFrame)
        player.setFramerate(10000)
        player.assumePixelsPerMM(1)
        for k, v in self.CUSTOM_SETTINGS.iteritems():
            self.settings.set(k, v)

        if mainDiv is None:
            mainDiv = libavg.app.MainDiv()

        self.run(mainDiv, **runtimeOptions)

    def __onFrame(self):
        if self.__onFrameHandlersList:
            todo = self.__onFrameHandlersList.pop(0)
            todo()
        else:
            player.stop()


class AppTestCase(AVGTestCase):
    def testSettingsOptions(self):
        self.assertRaises(ValueError, lambda: settings.Option('test', 1))
        
        self.assertRaises(avg.Exception, lambda: settings.Settings(
                [Option('foo', 'bar'), Option('foo', 'bar')]))

        s = settings.Settings([Option('foo', 'bar')])
        self.assertRaises(avg.Exception, lambda: s.addOption(Option('foo', 'baz')))

    def tearDown(self):
        libavg.app.instance = None

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

        self.assertEquals(s.getBoolean('test_boolean'), True)

        self.assertEquals(type(s.get('test_string')), str)
        self.assertRaises(ValueError, lambda: s.getBoolean('test_string'))

        self.assertEquals(s.getInt('another_value_int'), 1234)
        self.assertRaises(ValueError, lambda: s.getInt('test_string'))
        self.assertEquals(s.get('another_value_int'), '1234')

        self.assertEquals(s.getPoint2D('test_2d'), libavg.Point2D(1280, 1024))
        self.assertEquals(s.getPoint2D('test_2d_alt'), libavg.Point2D(1280, 1024))
        self.assertRaises(ValueError, lambda: s.getInt('test_2d'))

        self.assertEquals(s.getFloat('test_float'), 12.345)
        
        self.assertEquals(s.getJson('test_json'), [1, None, 3, 'string', 12.345])

    def testSettingsSet(self):
        s = settings.Settings()
        s.addOption(Option('test_value', ''))
        self.assertRaises(ValueError, lambda: s.set('test_value', 1234))

        s.set('test_value', '1234')
        self.assertEquals(s.getInt('test_value'), 1234)

    def testSettingsHasOption(self):
        s = settings.Settings()
        s.addOption(Option('test_value', ''))
        self.assertEquals(s.hasOption('test_value'), True)
        self.assertEquals(s.hasOption('test_value_foo'), False)

    def testSettingsArgvExtender(self):
        s = settings.Settings([Option('foo_bar', 'bar')])
        e = settings.ArgvExtender('', args=['foo', '--foo-bar', 'baz', '-c', 'baz2'])
        e.parser.add_option('-c')
        s.applyExtender(e)
        self.assertEquals(s.get('foo_bar'), 'baz')
        self.assertEquals(e.parsedArgs[0].c, 'baz2')
        self.assertEquals(e.parsedArgs[1], ['foo'])

        e = settings.ArgvExtender('', args=['foo', '--foo-baxxx', 'baz'])
        with SuppressOutput():
            self.assertRaises(SystemExit, lambda: s.applyExtender(e))

    def testSettingsKargsExtender(self):
        s = settings.Settings([Option('foo_bar', 'bar')])
        e = settings.KargsExtender({'foo_bar': 'baz'})
        s.applyExtender(e)
        self.assertEquals(s.get('foo_bar'), 'baz')

        e = settings.KargsExtender({'foo_baxxx': 'baz'})
        self.assertRaises(avg.Exception, lambda: s.applyExtender(e))

    def testAppAdditionalSettings(self):
        app = TestApp()
        app.settings.addOption(Option('foo_bar', 'baz'))
        app.settings.addOption(Option('bar_foo', 'baz'))
        self.assertEquals(app.settings.get('foo_bar'), 'baz')

    def testAppRuntimeSettings(self):
        app = TestApp()
        app.settings.addOption(Option('foo_bar', 'baz'))
        app.testRun([
                lambda: self.assertEquals(libavg.app.instance.settings.get('foo_bar'),
                        'bar'),
                ],
                runtimeOptions={'foo_bar':'bar'})
        
    def testAppRuntimeSettingsFail(self):
        app = TestApp()
        self.assertRaises(avg.Exception,
                lambda: app.testRun(runtimeOptions={'foo_bar':'bar'}))

    def testAppInstance(self):
        app = TestApp()
        self.assertEquals(app, libavg.app.instance)

    def testAppResolution(self):
        app = TestApp()
        app.testRun([
                lambda: self.assertEquals(player.getRootNode().size, (160, 120)),
                lambda: self.assert_(not player.isFullscreen()),
                ])

    def testAppDefaultWindowSize(self):
        app = TestApp()
        app.CUSTOM_SETTINGS = {'app_resolution': '160x120'}
        app.testRun()

    def testAppFullscreen(self):
        app = TestApp()
        resString = str(res.x) + "x" + str(res.y)
        app.CUSTOM_SETTINGS = {'app_resolution': resString}
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
        tempDir = tempfile.gettempdir()
        expectedFiles = map(lambda v: os.path.join(tempDir, v),
                ['TestApp-001.png', 'TestApp-002.png'])

        def removeFiles():
            for screenshotFile in expectedFiles:
                if os.path.exists(screenshotFile):
                    os.unlink(screenshotFile)

        def testScreenshots():
            for screenshotFile in expectedFiles:
                self.assert_(os.path.exists(screenshotFile))

        removeFiles()
        app = TestApp()
        app.testRun([
                lambda: app.takeScreenshot(tempDir),
                lambda: app.takeScreenshot(tempDir),
                testScreenshots,
                removeFiles,
                ])

    def testKeyboardManagerKeyname(self):
        tester = lambda: self.__emuKeyPress(4, "A", "a", libavg.avg.KEYMOD_NONE)
        self.__testKeyboardManager(keyname="A", tester=tester)

    def testKeyboardManagerScancode(self):
        tester = lambda: self.__emuKeyPress(4, "A", 'A', libavg.avg.KEYMOD_LSHIFT)
        self.__testKeyboardManager(scancode=4, modifiers=libavg.avg.KEYMOD_SHIFT,
                tester=tester)

    def testKeyboardManagerText(self):
        tester = lambda: self.__emuKeyPress(53, "Ö", 'ö', libavg.avg.KEYMOD_NONE)
        self.__testKeyboardManager(text=u'ö', modifiers=libavg.avg.KEYMOD_NONE,
                tester=tester)

    def __testKeyboardManager(self, scancode=None, keyname=None, text=None,
            modifiers=libavg.avg.KEYMOD_NONE, tester=None):
        self.statesRecords = [False, False]
        def keyDownPressed():
            self.statesRecords[0] = True

        def keyUpPressed():
            self.statesRecords[1] = True

        def bindKeys():
            keyboardmanager.bindKeyDown(scancode=scancode, keyname=keyname, text=text,
                    help='', modifiers=modifiers, handler=keyDownPressed)
            if text == None:
                keyboardmanager.bindKeyUp(scancode=scancode, keyname=keyname,
                        modifiers=modifiers, handler=keyUpPressed)
            else:
                self.statesRecords[1] = True

        def reset():
            keyboardmanager.unbindKeyDown(scancode=scancode, keyname=keyname, text=text,
                    modifiers=modifiers)
            if text == None:
                keyboardmanager.unbindKeyUp(scancode=scancode, keyname=keyname,
                        modifiers=modifiers)
            self.statesRecords = [False, False]

        def cleanup():
            del self.statesRecords

        app = TestApp()
        app.testRun([
                bindKeys,
                tester,
                lambda: self.assert_(all(self.statesRecords)),
                reset,
                tester,
                lambda: self.assert_(not any(self.statesRecords)),
                cleanup,
                ])

    def __emuKeyPress(self, scanCode, keyName, text, modifiers):
        helper = libavg.player.getTestHelper()
        helper.fakeKeyEvent(libavg.avg.Event.KEY_DOWN, scanCode, keyName,
                modifiers, text)
        helper.fakeKeyEvent(libavg.avg.Event.KEY_UP, scanCode, keyName,
                modifiers, "")


def appTestSuite(tests):
    availableTests = (
            'testSettingsOptions',
            'testSettingsTypes',
            'testSettingsSet',
            'testSettingsHasOption',
            'testSettingsArgvExtender',
            'testSettingsKargsExtender',
            'testAppAdditionalSettings',
            'testAppRuntimeSettings',
            'testAppRuntimeSettingsFail',
            'testAppInstance',
            'testAppResolution',
            'testAppDefaultWindowSize',
            'testAppFullscreen',
            'testAppRotation',
            'testScreenshot',
            'testKeyboardManagerKeyname',
            'testKeyboardManagerScancode',
            'testKeyboardManagerText',
    )
    return createAVGTestSuite(availableTests, AppTestCase, tests)

