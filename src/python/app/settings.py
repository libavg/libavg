#!/usr/bin/env python
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


'''
Provide settings via a loosely-defined configuration, generating interoperability
between configuration and command line arguments.
'''

import sys
import os
import argparse


class Settings(object):
    def __init__(self, defaults, settingsKeywords={}):
        assert type(defaults) == dict
        # TODO: check if a thorough check is better than a simple update
        defaults.update(settingsKeywords)
        self.__config = dict([(k, str(v)) for k, v in defaults.iteritems()])

        self.__overrideDefaultsWithCliArgs()

    def get(self, key, convertFunc=lambda v: v):
        if not key in self.__config:
            raise RuntimeError('Cannot find key %s in the settings' % key)

        value = self.__config[key]
        try:
            return convertFunc(value)
        except (TypeError, ValueError), e:
            raise ValueError('%s (key=%s value=%s)' % (e,
                    key, value))

    def decodejson(self, key):
        import json

        return self.get(key, json.loads)
    
    def getint(self, key):
        return self.get(key, int)
    
    def getfloat(self, key):
        return self.get(key, float)

    def getboolean(self, key):
        value = self.get(key).lower()
        
        if value in ('yes', 'true'):
            return True
        elif value in ('no', 'false'):
            return False
        else:
            raise TypeError('Cannot convert %s to boolean' % value)

    def getall(self):
        return self.__config

    def __overrideDefaultsWithCliArgs(self):
        finalParser = argparse.ArgumentParser()

        mainGroup = finalParser.add_argument_group('Application options')
        
        for key, val in self.__config.iteritems():
            cliKey = '--%s' % key.replace('_', '-').lower()
            currentValue = val

            mainGroup.add_argument(cliKey, help='Default: %s' % currentValue)

        args = finalParser.parse_args()

        self.__config.update((k, v) for k, v in args.__dict__.iteritems() if v is not None)


if __name__ == '__main__':
    defaults = dict(test_boolean=True, test_string='string', another_value_int=1234)

    s = Settings(defaults)
    print s.getall()
    print s.getboolean('test_boolean')
    print s.get('test_string')
    print s.getint('another_value_int')

