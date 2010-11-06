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

import sys
import os
import shutil


g_TempPackageDir = None


def symtree(src, dest):
    os.mkdir(dest)
    for f in os.listdir(src):
        fpath = os.path.join(src, f)
        if (f and f[0] != '.' and
            (os.path.isdir(fpath) or
            (os.path.isfile(fpath) and os.path.splitext(f)[1] == '.py'))):
                os.symlink(os.path.join(os.pardir, src, f), os.path.join(dest, f))

        
if sys.platform != 'win32':
    g_TempPackageDir = os.path.join(os.getcwd(), 'libavg')
    if os.getenv('srcdir') in ('.', None):
        if os.path.basename(os.getcwd()) != 'test':
            raise RuntimeError('Manual tests must be performed inside directory "test"')
        
        if os.path.isdir(g_TempPackageDir):
            print 'Cleaning up old test package'
            shutil.rmtree(g_TempPackageDir)
        
        try:
            # We're running make check / manual tests
            symtree('../python', 'libavg')
            # os.system('cp -r ../python libavg')
            os.symlink('../../wrapper/__init__.py', 'libavg/__init__.py')
        except OSError:
            pass
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
    libavg.avg.Logger.get().trace(libavg.avg.Logger.APP, "Using libavg from: "+
            os.path.dirname(libavg.__file__))

    cpfx = os.path.commonprefix((libavg.__file__, os.getcwd()))
    
#    if cpfx != os.getcwd():
#        raise RuntimeError(
#            'Tests would be performed with a non-local libavg package (%s)'
#            % libavg.__file__)

srcDir = os.getenv("srcdir",".")
os.chdir(srcDir)

import testapp   
   
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
from EventTest import mainMouseDown
from EventTest import mainMouseUp


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


try:
    app.run()
finally:
    if g_TempPackageDir is not None:
        try:
            shutil.rmtree(g_TempPackageDir)
        except OSError:
            print 'ERROR: Cannot clean up test package directory'

sys.exit(app.exitCode())

