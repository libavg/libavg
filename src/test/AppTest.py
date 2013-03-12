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


import sys
import libavg
from libavg import player
from libavg.app import settings
from libavg.app.settings import Option
import testcase


class TestApp(libavg.app.App):
    def testRun(self, mainScene, onFrameHandlersList):
        assert type(onFrameHandlersList) == list
        self.__onFrameHandlersList = onFrameHandlersList
        player.subscribe(player.ON_FRAME, self.__onFrame)
        player.setFramerate(10000)
        player.assumePixelsPerMM(1)
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
        app = libavg.app.App()
        self.assertEquals(app, libavg.app.instance)

    def tearDown(self):
        libavg.app.instance = None


def appTestSuite(tests):
    availableTests = (
            'testSettingsOptions',
            'testSettingsTypes',
            'testSettingsSet',
            'testSettingsArgvExtender',
            'testAppAdditionalSettings',
            'testAppInstance',
    )
    return testcase.createAVGTestSuite(availableTests, AppTestCase, tests)

