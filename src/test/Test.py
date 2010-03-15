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

import testapp   
   
import PluginTest
import PlayerTest
import OffscreenTest
import ImageTest
import VectorTest
import WordsTest
import AVTest
import DynamicsTest
import PythonTest
import AnimTest
import EventTest
from EventTest import mainMouseDown
from EventTest import mainMouseUp

try:
    app = testapp.TestApp()
    
    app.registerSuiteFactory('plugin', PluginTest.pluginTestSuite)
    app.registerSuiteFactory('player', PlayerTest.playerTestSuite)
    app.registerSuiteFactory('offscreen', OffscreenTest.offscreenTestSuite)
    app.registerSuiteFactory('image', ImageTest.imageTestSuite)
    app.registerSuiteFactory('vector', VectorTest.vectorTestSuite)
    app.registerSuiteFactory('words', WordsTest.wordsTestSuite)
    app.registerSuiteFactory('av', AVTest.AVTestSuite)
    app.registerSuiteFactory('dynamics', DynamicsTest.dynamicsTestSuite)
    app.registerSuiteFactory('python', PythonTest.pythonTestSuite)
    app.registerSuiteFactory('anim', AnimTest.animTestSuite)
    app.registerSuiteFactory('event', EventTest.eventTestSuite)
    
    app.run()

except Exception, e:
    testapp.cleanExit(e)
