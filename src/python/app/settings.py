#!/usr/bin/env python
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


import sys
import re
import optparse

import libavg


class Option(object):
    def __init__(self, key, value, help=None):
        if not isinstance(key, str):
            raise ValueError('The type of %s key is not string (value=%s)' % (key, value))

        self.__key = key
        self.value = value
        self.__help = help

    def __repr__(self):
        return '<%s key=%s value=%s help=%s>' % (self.__class__.__name__,
                self.key, self.value, self.help)

    @property
    def key(self):
        return self.__key

    @property
    def value(self):
        return self.__value

    @value.setter
    def value(self, value):
        if not isinstance(value, str):
            raise ValueError('The type of %s value (%s) '
                    'must be string instead of %s' % (self.__key, value, type(value)))
        
        self.__value = value

    @property
    def group(self):
        components = self.__getComponents()
        if len(components) == 1:
            return 'DEFAULT'
        else:
            return components[0]

    @property
    def tail(self):
        components = self.__getComponents()
        if len(components) == 1:
            return self.key
        else:
            return components[1]
        
    @property
    def help(self):
        return self.__help

    def __getComponents(self):
        return self.key.split('_', 1)


class KargsExtender(object):
    def __init__(self, optionsKargs):
        self.__optionsKargs = optionsKargs

    def __call__(self, optionsList):
        optionsKeyset = set([option.key for option in optionsList])
        kaKeyset = set(self.__optionsKargs.keys())

        if not optionsKeyset.issuperset(kaKeyset):
            raise RuntimeError('No such option/s: %s' % list(kaKeyset - optionsKeyset))
            
        for option in optionsList:
            if option.key in self.__optionsKargs:
                option.value = self.__optionsKargs[option.key]

        return optionsList


class HelpPrintingOptionParser(optparse.OptionParser):
    def error(self, *args, **kargs):
        self.print_help()
        optparse.OptionParser.error(self, *args, **kargs)


class ArgvExtender(object):
    def __init__(self, appVersionInfo, args=None):
        self.__appVersionInfo = appVersionInfo
        self.__parser = HelpPrintingOptionParser()
        self.__args = args
        self.__parsedArgs = None

    def __call__(self, optionsList):
        self.__parser.add_option('-v', '--version', dest='version', action='store_true',
                help='print libavg and application version information')

        groups = self.__groupOptionsKeys(optionsList)

        for group in sorted(groups):
            parserGroup = optparse.OptionGroup(self.__parser,
                    '%s section' % group.title())

            keys = sorted(groups[group])

            for option in [option for option in optionsList if option.key in keys]:
                cliKey = '--%s' % option.key.replace('_', '-').lower()
                currentValue = option.value if option.value else '<undefined>'

                help = '[Default: %s]' % currentValue

                if option.help:
                    help = '%s %s' % (option.help, help)

                parserGroup.add_option(cliKey, help=help)

            self.__parser.add_option_group(parserGroup)

        if self.__args is None:
            self.__args = sys.argv[1:]

        self.__parsedArgs = self.__parser.parse_args(args=self.__args)

        parsedOptions = self.__parsedArgs[0]

        if parsedOptions.version:
            print 'libavg'
            vi = libavg.VersionInfo()
            print ' version  : %s' % vi.full
            print ' builder  : %s (%s)' % (vi.builder, vi.buildtime)
            print
            print 'application'
            print ' version: %s' % self.__appVersionInfo
            sys.exit(0)

        for key, value in parsedOptions.__dict__.iteritems():
            if value is not None:
                for option in optionsList:
                    if option.key == key:
                        option.value = value
        
        return optionsList

    @property
    def parsedArgs(self):
        if self.__parsedArgs is None:
            raise RuntimeError('Cannot provide parsedArgs before applying the extender')

        return self.__parsedArgs

    @property
    def parser(self):
        return self.__parser

    def __groupOptionsKeys(self, optionsList):
        groups = {}
        for option in optionsList:
            if not option.group in groups:
                groups[option.group] = []
            
            groups[option.group].append(option.key)

        return groups


class Settings(object):
    def __init__(self, defaults=[]):
        if (type(defaults) not in (tuple, list) or
                not all([isinstance(opt, Option) for opt in defaults])):
            raise ValueError('Settings must be initialized with a list '
                    'of Option instances')

        self.__options = []

        for option in defaults:
            self.addOption(option)

    def __iter__(self):
        return self.__options.__iter__()

    def applyExtender(self, extender):
        self.__options = extender(self.__options)

    def hasOption(self, key):
        return self.__getOptionOrNone(key) is not None

    def getOption(self, key):
        option = self.__getOptionOrNone(key)

        if option is None:
            raise RuntimeError('Cannot find key %s in the settings' % key)

        return option
        
    def get(self, key, convertFunc=lambda v: v):
        option = self.getOption(key)

        try:
            return convertFunc(option.value)
        except (TypeError, ValueError), e:
            raise ValueError('%s (option=%s)' % (e, option))

    def getJson(self, key):
        import json

        return self.get(key, json.loads)
    
    def getPoint2D(self, key):
        value = self.get(key)
        maybeTuple = re.split(r'\s*[,xX]\s*', value)
        
        if len(maybeTuple) != 2:
            raise ValueError('Cannot convert key %s value %s to Point2D' % (key, value))

        return libavg.Point2D(map(float, maybeTuple))

    def getInt(self, key):
        return self.get(key, int)
    
    def getFloat(self, key):
        return self.get(key, float)

    def getBoolean(self, key):
        value = self.get(key).lower()
        
        if value in ('yes', 'true'):
            return True
        elif value in ('no', 'false'):
            return False
        else:
            raise ValueError('Cannot convert %s to boolean' % value)

    def set(self, key, value):
        option = self.getOption(key)
        option.value = value

    def addOption(self, option):
        if not isinstance(option, Option):
            raise TypeError('Must be an instance of Option')

        if self.__getOptionOrNone(option.key):
            raise RuntimeError('Option %s has been already defined' % option.key)
            
        self.__options.append(option)

    def __getOptionOrNone(self, key):
        for option in self.__options:
            if option.key == key:
                return option

        return None


