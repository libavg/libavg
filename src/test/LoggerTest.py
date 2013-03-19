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

import StringIO
import logging

from libavg import logger

from testcase import *


class LoggerTestCase(AVGTestCase):
    def __init__(self, testFuncName):
        AVGTestCase.__init__(self, testFuncName)

    def testAddSink(self):
        logger.clearSinks()
        stream = StringIO.StringIO()
        hdlr = logging.StreamHandler(stream)
        formatter = logging.Formatter('[%(asctime)s][%(levelname)s][%(category)s] : %(message)s')
        pyLogger = logging.getLogger(__name__)
        pyLogger.addHandler(hdlr)
        pyLogger.propagate = False
        pyLogger.level = logging.DEBUG
        testText = u'福 means good fortune'
        logger.addSink(pyLogger)

        logger.info(testText)
        stream.flush()
        self.assert_(stream.getvalue().decode('utf8').find(testText) != -1)
        stream.truncate(0)
        stream.flush()
        self.assert_(stream.getvalue().decode('utf8').find(testText) == -1)

def loggerTestSuite(tests):
    availableTests = (
            "testAddSink",)
    return createAVGTestSuite(availableTests, LoggerTestCase, tests)
