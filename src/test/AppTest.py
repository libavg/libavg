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


import libavg
from libavg.app import settings
import testcase


class AppTestCase(testcase.AVGTestCase):
    def testSettingsDefaults(self):
        self.assertException(lambda: settings.Defaults(test=1), ValueError)
        
        defaults = settings.Defaults()
        def intKey():
            defaults[1] = 'foo'
        
        self.assertException(intKey, ValueError)
        
        def intValue():
            defaults['foo'] = 1

        self.assertException(intValue, ValueError)
        
    def testSettingsTypes(self):
        defaults = settings.Defaults(test_boolean='True', test_string='string',
                another_value_int='1234', test_2d='1280x1024', test_2d_alt='1280,1024',
                test_float='12.345', test_json='[1, null,3 , "string", 12.345]')

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
        s = settings.Settings(settings.Defaults())
        self.assertException(lambda: s.set('test_value', 1234), ValueError)

        s.set('test_value', '1234')
        self.assertEquals(s.getint('test_value'), 1234)

    def testSettingsIteration(self):
        defaults = settings.Defaults(a='foo', b='bar', c='baz')
        s = settings.Settings(defaults)
        
        keys = defaults.keys()
        for idx in s:
            self.assert_(idx in keys)
            keys.remove(idx)
            
        self.assertEquals(keys, [])

            
            
def appTestSuite(tests):
    availableTests = (
            'testSettingsDefaults',
            'testSettingsTypes',
            'testSettingsSet',
            'testSettingsIteration',
    )
    return testcase.createAVGTestSuite(availableTests, AppTestCase, tests)

