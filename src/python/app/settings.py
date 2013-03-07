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


import re
import optparse

import libavg


class Settings(object):
    VALUE_CONV_FUNC = str

    def __init__(self, defaults, settingsKeywords={}):
        assert type(defaults) == dict
        # TODO: check if a thorough check is better than a simple update
        defaults.update(settingsKeywords)
        self.__config = defaults

        self.__overrideDefaultsWithCliArgs()

    def __iter__(self):
        return self.__config.__iter__()

    def get(self, key, convertFunc=lambda v: v):
        if not key in self.__config:
            raise RuntimeError('Cannot find key %s in the settings' % key)

        value = self.__config[key]
        try:
            return convertFunc(value)
        except (TypeError, ValueError), e:
            raise ValueError('%s (key=%s value=%s)' % (e,
                    key, value))

    def getjson(self, key):
        import json

        return self.get(key, json.loads)
    
    def getpoint2d(self, key):
        value = self.get(key)
        maybeTuple = re.split(r'\s*[,xX]\s*', value)
        
        if len(maybeTuple) != 2:
            raise ValueError('Cannot convert key %s value %s to Point2D' % (key, value))

        return libavg.Point2D(map(float, maybeTuple))

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
            raise ValueError('Cannot convert %s to boolean' % value)

    def set(self, key, value):
        self.__config[key] = self.VALUE_CONV_FUNC(value)
        
    def __overrideDefaultsWithCliArgs(self):
        parser = optparse.OptionParser()

        mainGroup = optparse.OptionGroup(parser, 'Application options')
        
        for key, val in self.__config.iteritems():
            cliKey = '--%s' % key.replace('_', '-').lower()
            currentValue = val

            mainGroup.add_option(cliKey, help='Default: %s' % currentValue)

        parser.add_option_group(mainGroup)
        options, posargs = parser.parse_args()

        self.__config.update((k, v) for k, v in options.__dict__.iteritems() if v is not None)


