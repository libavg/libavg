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

    if isinstance(rc, Exception):
        raise rc
    else:
        sys.exit(rc)

def symtree(src, dest):
    os.mkdir(dest)
    for f in os.listdir(src):
        fpath = os.path.join(src, f)
        if (f and f[0] != '.' and
            (os.path.isdir(fpath) or
            (os.path.isfile(fpath) and os.path.splitext(f)[1] == '.py'))):
                os.symlink(os.path.join(os.pardir, src, f), os.path.join(dest, f))
        
if platform.system() != 'Windows':
    g_TempPackageDir = os.path.join(os.getcwd(), 'libavg')
    if os.getenv('srcdir') in ('.', None):
        if os.path.basename(os.getcwd()) != 'test':
            raise RuntimeError('Manual tests must be performed inside directory "test"')
        
        if os.path.isdir(g_TempPackageDir):
            print 'Cleaning up old test package'
            shutil.rmtree(g_TempPackageDir)
        
        # We're running make check / manual tests
        symtree('../python', 'libavg')
        # os.system('cp -r ../python libavg')
        os.symlink('../../wrapper/__init__.py', 'libavg/__init__.py')
    else:
        # make distcheck
        symtree('../../../../src/python', 'libavg')
        os.symlink('../../../../../src/wrapper/__init__.py', 'libavg/__init__.py')
        sys.path.insert(0, os.getcwd())
    
    os.symlink('../../wrapper/.libs/avg.so', 'libavg/avg.so')

    # The following lines help to prevent the test to be run
    # with an unknown version of libavg, which can be hiding somewhere
    # in the system
    import libavg

    cpfx = os.path.commonprefix((libavg.__file__, os.getcwd()))
    
    if cpfx != os.getcwd():
        raise RuntimeError(
            'Tests would be performed with a non-local libavg package (%s)'
            % libavg.__file__)
    
from libavg import avg

SrcDir = os.getenv("srcdir",".")
os.chdir(SrcDir)

from PluginTest import *
from PlayerTest import *
from OffscreenTest import *
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


def main():
    if os.getenv("AVG_CONSOLE_TEST"):
        runConsoleTest()
        return
    
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

    availableSuites = [ {'plugin': pluginTestSuite},
                        {'player': playerTestSuite},
                        {'offscreen': offscreenTestSuite},
                        {'image': imageTestSuite},
                        {'vector': vectorTestSuite},
                        {'words': wordsTestSuite},
                        {'av': AVTestSuite},
                        {'dynamics': dynamicsTestSuite},
                        {'python': pythonTestSuite},
                        {'anim': animTestSuite},
                        {'event': eventTestSuite} ]
    
    testSubset = []
    if len(args): # suite
        suiteName = args.pop(0)
        foundSuites = [ suit for suit in availableSuites if suiteName in suit.keys() ]
        if not(foundSuites):
            parser.print_usage()
            print "ERROR: unknown test suite, known suites:"
            print ", ".join(availableSuites.keys())
            cleanExit(1)
            
        suitesToRun = foundSuites
        testSubset = args
    else:
        suitesToRun = availableSuites

    mainSuite = unittest.TestSuite()
    for element in suitesToRun:
        suit = element.items()[0][1]
        mainSuite.addTest(suit(testSubset))

    Player = avg.Player.get()
    Player.setMultiSampleSamples(1)
    dumpConfig()

    runner = unittest.TextTestRunner()
    rmBrokenDir()
    rc = runner.run(mainSuite)
    
    if rc.wasSuccessful():
        cleanExit(0)
    else:
        cleanExit(1)


if __name__ == '__main__':
    try:
        main()
    except Exception, e:
        cleanExit(e)

