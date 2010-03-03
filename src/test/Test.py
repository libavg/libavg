#!/usr/bin/env python
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

'''
Entry point for libavg unit tests
On autotools-based systems, tests are performed on a local libavg package.
This package is created by copying all the relevant files in a local, temporary
directory, and its scope is added in order to let python to find it.
On windows instead, tests are carried on after distutils takes care of a
system-wide installation.
'''
import unittest

from optparse import OptionParser, OptionValueError
import sys
import os
import platform
import shutil

g_TempPackageDir = None

def cleanExit(rc):
    if g_TempPackageDir is not None:
        try:
            shutil.rmtree(g_TempPackageDir)
        except OSError:
            print 'ERROR: Cannot clean up test package directory'

    sys.exit(rc)

if platform.system() != 'Windows':
    print 'Creating local libavg python package'
    if os.getenv('srcdir') in ('.', None):
        # We're running make check
        shutil.copytree('../python', 'libavg')
        shutil.copy('../wrapper/__init__.py', 'libavg/')
    else:
        # make distcheck
        shutil.copytree('../../../../src/python', 'libavg')
        shutil.copy('../../../../src/wrapper/__init__.py', 'libavg/')
        sys.path.insert(0, os.getcwd())
    
    shutil.copy('../wrapper/.libs/avg.so', 'libavg/')
    g_TempPackageDir = os.path.join(os.getcwd(), 'libavg')

    # The following lines help to prevent the test to be run
    # with an unknown version of libavg, which can be laying somewhere
    # in the system
    import libavg

    cpfx = os.path.commonprefix((libavg.__file__, os.getcwd()))
    
    if cpfx != os.getcwd():
        raise RuntimeError('Tests would be performed with a non-local libavg package (%s)'
            % libavg.__file__)
    
from libavg import avg

SrcDir = os.getenv("srcdir",".")
os.chdir(SrcDir)

from PluginTest import *
from PlayerTest import *
from ImageTest import *
from VectorTest import *
from WordsTest import *
from AVTest import *
from DynamicsTest import *
from PythonTest import *
from AnimTest import *
from EventTest import *


def runConsoleTest():
    Player = avg.Player.get()
    Player.loadFile("video.avg")

def dumpConfig():
    Log = avg.Logger.get()
    Log.pushCategories()
    Log.setCategories(
            Log.APP |
            Log.WARNING |
            Log.CONFIG  |
            0)
    Player.loadString("""
            <?xml version="1.0"?>
            <avg id="avg" width="160" height="120">
            </avg>
            """)
    setUpVideo(Player)
    Player.setTimeout(0, Player.stop)
    Player.play()
    Log.popCategories()
   

def main():
    if os.getenv("AVG_CONSOLE_TEST"):
        runConsoleTest()
    else:
        def setPow2(option, opt, value, parser):
            if value not in ('yes','no'):
                raise OptionValueError('argument must be "yes" or "no"')
            setUsePOW2Textures(value == 'yes')
        def setShaders(option, opt, value, parser):
            if value not in ('yes','no'):
                raise OptionValueError('argument must be "yes" or "no"')
            setUseShaders(value == 'yes')
        def setPixBuf(option, opt, value, parser):
            if value not in ('yes','no'):
                raise OptionValueError('argument must be "yes" or "no"')
            setUsePixelBuffers(value == 'yes')

        parser = OptionParser("usage: %prog [options] [<suite> [testcase] [testcase] [...]]")
        parser.add_option("-b", "--bpp", dest = "bpp",
                type = "int",
                help = "set pixel depth")
        parser.add_option("-2", "--usepow2textures", dest = "usepow2textures",
                type = "string",
                action = 'callback', callback = setPow2,
                help = "Use power of 2 textures (yes, no)")
        parser.add_option("-s", "--useshaders", dest = "shaders",
                type = "string",
                action = 'callback', callback = setShaders,
                help = "Use shaders (yes, no)")
        parser.add_option("-p", "--usepixelbuffers", dest = "usepixelbuffers",
                type = "string",
                action = 'callback', callback = setPixBuf,
                help = "Use pixel buffers (yes, no)")
        parser.set_defaults(bpp = 24)
        options, args = parser.parse_args()

        availableSuites = {
                'plugin': (pluginTestSuite,{}),
                'player': (playerTestSuite, {'bpp':options.bpp}),
                'image': (imageTestSuite, {}),
                'vector': (vectorTestSuite, {}),
                'words': (wordsTestSuite, {}),
                'av': (AVTestSuite, {}),
                'dynamics': (dynamicsTestSuite, {}),
                'python': (pythonTestSuite, {}),
                'anim': (animTestSuite, {}),
                'event': (eventTestSuite, {}),
                }
        tests = []
        if len(args): # suite
            suiteName = args.pop(0)
            if suiteName not in availableSuites:
                parser.print_usage()
                print "ERROR: unknown test suite, known suites:"
                print ", ".join(availableSuites.keys())
                cleanExit(1)
            else:
                suitesToRun = [availableSuites[suiteName]]
            tests = args
        else:
            suitesToRun = availableSuites.values()

        suite = unittest.TestSuite()
        for s, args in suitesToRun:
            args.update({'tests':tests})
            suite.addTest(s(**args))

        Player = avg.Player.get()
        Player.setMultiSampleSamples(1)
        dumpConfig()
        Log = avg.Logger.get()

        runner = unittest.TextTestRunner()
        rmBrokenDir()
        rc = runner.run(suite)
        
        if rc.wasSuccessful():
            cleanExit(0)
        else:
            cleanExit(1)

if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        print 'CTRL-C'
        cleanExit(0)

