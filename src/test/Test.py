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

import sys
import os

"""
Note: The tests can be run in several locations. On make-based systems, they can run
in the staging area (either by calling Test.py directly or by using make check) or
from within site-packages after install. On windows, tests are always run on the
installed package (within site-packages).

The following line allows us to find a local libavg in the staging area.
"""
if sys.platform != 'win32':
    sys.path.insert(0, '../..')

import libavg
libavg.logger.configureCategory(libavg.Logger.Category.APP, libavg.Logger.Severity.INFO)
libavg.logger.info("Using libavg from: "+ os.path.dirname(libavg.__file__),
        libavg.Logger.Category.APP)
# Ensure mouse is activated
libavg.player.enableMouse(True)

from libavg import testapp

libavg.player.keepWindowOpen()

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
import WidgetTest
import GestureTest
import LoggerTest
import AppTest
import MultiWindowTest

app = testapp.TestApp()

app.registerSuiteFactory('player', PlayerTest.playerTestSuite)
app.registerSuiteFactory('image', ImageTest.imageTestSuite)
app.registerSuiteFactory('vector', VectorTest.vectorTestSuite)
app.registerSuiteFactory('words', WordsTest.wordsTestSuite)
app.registerSuiteFactory('dynamics', DynamicsTest.dynamicsTestSuite)
app.registerSuiteFactory('event', EventTest.eventTestSuite)
app.registerSuiteFactory('av', AVTest.AVTestSuite)
app.registerSuiteFactory('offscreen', OffscreenTest.offscreenTestSuite)
app.registerSuiteFactory('fx', FXTest.fxTestSuite)
app.registerSuiteFactory('python', PythonTest.pythonTestSuite)
app.registerSuiteFactory('anim', AnimTest.animTestSuite)
app.registerSuiteFactory('widget', WidgetTest.widgetTestSuite)
app.registerSuiteFactory('gesture', GestureTest.gestureTestSuite)
app.registerSuiteFactory('logger', LoggerTest.loggerTestSuite)
app.registerSuiteFactory('app', AppTest.appTestSuite)
app.registerSuiteFactory('plugin', PluginTest.pluginTestSuite)
app.registerSuiteFactory('multiwindow', MultiWindowTest.multiWindowTestSuite)

app.run()

sys.exit(app.exitCode())

