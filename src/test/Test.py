#!/usr/bin/env python
# -*- coding: utf-8 -*-
# libavg - Media Playback Engine.
# Copyright (C) 2003-2011 Ulrich von Zadow
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
Runner for libavg unit tests

On autotools-based systems, tests are performed on a local libavg package.
This package is created by symlinking all the relevant files in a local, temporary
directory, letting python find it as first instance.
On windows, instead, tests are always carried on after distutils installs the package.
'''

import sys
import os
import shutil
import atexit

def cleanup(folder):
    if os.path.isdir(folder):
        sys.stderr.write('Wiping out directory: %s\n' % folder)
        shutil.rmtree(folder)

def symtree(src, dest):
    os.mkdir(dest)
    for f in os.listdir(src):
        fpath = os.path.join(src, f)
        if (f and f[0] != '.' and
            (os.path.isdir(fpath) or
            (os.path.isfile(fpath) and os.path.splitext(f)[1] in ('.py', '.glsl')))):
                os.symlink(os.path.join(os.pardir, src, f), os.path.join(dest, f))


if sys.platform != 'win32':
    tempPackageDir = os.path.join(os.getcwd(), 'libavg')
    # Possible values for srcdir:
    # '.': make check
    # None: ./Test.py
    # dir name: make distcheck
    srcDir = os.getenv("srcdir",".")
    if srcDir == '.':
        # Running make check or ./Test.py
        if os.path.basename(os.getcwd()) != 'test':
            raise RuntimeError('Manual tests must be performed inside directory "test"')
        
        cleanup(tempPackageDir)
        
        try:
            symtree('../python', 'libavg')
            os.symlink('../../graphics/shaders', 'libavg/shaders')
        except OSError:
            pass
    else:
        # Running make distcheck
        symtree('../../../../src/python', 'libavg')
        os.symlink('../../../../../src/graphics/shaders', 'libavg/shaders')

        # distcheck doesn't want leftovers (.pyc files)
        atexit.register(lambda tempPackageDir=tempPackageDir: cleanup(tempPackageDir))
    
    if os.path.exists('../wrapper/.libs/avg.so'):
        # Normal case: use the local version (not the installed one)
        shutil.copy2('../wrapper/.libs/avg.so', 'libavg/avg.so')
    elif os.path.exists('../../avg.so'):
        # Mac version after installer dmg
        pass
    else:
        raise RuntimeError('Compile libavg before running tests or use "make check"')

    # The following line prevents the test to be run
    # with an unknown version of libavg, which can be hiding somewhere
    # in the system
    sys.path.insert(0, os.getcwd())

    # Meaningful only for distcheck
    os.chdir(srcDir)

import libavg
libavg.avg.logger.trace(libavg.avg.Logger.APP, "Using libavg from: "+
        os.path.dirname(libavg.__file__))
# Ensure mouse is activated
libavg.player.enableMouse(True)

import testapp

libavg.Player.get().keepWindowOpen()

import PluginTest
import PlayerTest
import OffscreenTest
import ImageTest
import FXTest
import VectorTest
import WordsTest
import AVTest
import DynamicsTest
import PythonTest
import AnimTest
import EventTest
import InputDeviceTest
import AVGAppTest
import WidgetTest
import GestureTest

app = testapp.TestApp()

app.registerSuiteFactory('plugin', PluginTest.pluginTestSuite)
app.registerSuiteFactory('player', PlayerTest.playerTestSuite)
app.registerSuiteFactory('offscreen', OffscreenTest.offscreenTestSuite)
app.registerSuiteFactory('image', ImageTest.imageTestSuite)
app.registerSuiteFactory('fx', FXTest.fxTestSuite)
app.registerSuiteFactory('vector', VectorTest.vectorTestSuite)
app.registerSuiteFactory('words', WordsTest.wordsTestSuite)
app.registerSuiteFactory('av', AVTest.AVTestSuite)
app.registerSuiteFactory('dynamics', DynamicsTest.dynamicsTestSuite)
app.registerSuiteFactory('python', PythonTest.pythonTestSuite)
app.registerSuiteFactory('anim', AnimTest.animTestSuite)
app.registerSuiteFactory('event', EventTest.eventTestSuite)
app.registerSuiteFactory('inputdevice', InputDeviceTest.inputDeviceTestSuite)
app.registerSuiteFactory('widget', WidgetTest.widgetTestSuite)
app.registerSuiteFactory('gesture', GestureTest.gestureTestSuite)
app.registerSuiteFactory('avgapp', AVGAppTest.avgAppTestSuite)

app.run()

sys.exit(app.exitCode())

