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
        self.testMsg = u'福 means good fortune'

    def setUp(self):
        self.stream = StringIO.StringIO()
        self.hdlr = logging.StreamHandler(self.stream)
        self.pyLogger = logging.getLogger(__name__)
        self.pyLogger.addHandler(self.hdlr)
        self.pyLogger.propagate = False
        self.pyLogger.level = logging.DEBUG
        logger.addSink(self.pyLogger)

    def tearDown(self):
        self.pyLogger.removeHandler(self.hdlr)
        logger.clearSinks()

    def _assertMsg(self):
        self.stream.flush()
        self.assert_(self.stream.getvalue().decode('utf8').find(self.testMsg) != -1)
        self.stream.close()

    def _assertNoMsg(self):
        self.stream.flush()
        self.assert_(self.stream.getvalue().decode('utf8').find(self.testMsg) == -1)
        self.stream.close()

    def testAddSink(self):
        logger.clearSinks()
        logger.addSink(self.pyLogger)
        logger.info(self.testMsg)
        self._assertMsg()

    def testClearSinks(self):
        logger.clearSinks()
        logger.info(self.testMsg)
        self._assertNoMsg()

    def testRemoveSink(self):
        logger.removeSink(self.pyLogger)
        logger.info(self.testMsg)
        self._assertNoMsg()

    def testRegisterAndSetCategory(self):
        snowmanCategory = logger.registerCategory(u'☃ Category')
        logger.setCategories(logger.getCategories() | snowmanCategory)
        logger.info(self.testMsg, snowmanCategory)
        self._assertMsg()

    def testSetNoCategories(self):
        logger.setCategories(logger.NONE)
        logger.info(self.testMsg)
        self._assertNoMsg()
    
    def testSetCategories(self):
        logger.setCategories(logger.APP)
        logger.info(self.testMsg)
        self._assertMsg()

    def testSetSeverityAbove(self):
        logger.setSeverity(logger.APP, logger.WARNING)
        logger.info(self.testMsg)
        self._assertNoMsg()

    def testSetSeverity(self):
        logger.setSeverity(logger.APP, logger.INFO)
        logger.info(self.testMsg)
        self._assertMsg()

    def testSetDefaultSeverity1(self):
        logger.setDefaultSeverity(logger.DEBUG)
        snowmanCategory = logger.registerCategory(u'☃ Category')
        logger.setCategories(logger.getCategories() | snowmanCategory)
        logger.debug(self.testMsg, snowmanCategory)
        self._assertMsg()

    def testSetDefaultSeverity2(self):
        logger.setDefaultSeverity(logger.INFO)
        snowmanCategory = logger.registerCategory(u'☃ Category')
        logger.setCategories(logger.getCategories() | snowmanCategory)
        logger.debug(self.testMsg, snowmanCategory)
        self._assertNoMsg()


def loggerTestSuite(tests):
    availableTests = (
            "testAddSink",
            "testClearSinks",
            "testRemoveSink",
            "testRegisterAndSetCategory",
            "testSetNoCategories",
            "testSetCategories",
            "testSetSeverityAbove",
            "testSetSeverity",
            "testSetDefaultSeverity1",
            "testSetDefaultSeverity2",
            )
    return createAVGTestSuite(availableTests, LoggerTestCase, tests)
