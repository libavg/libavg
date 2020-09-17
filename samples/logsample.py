#!/usr/bin/env python
# -*- coding: utf-8 -*-

# libavg - Media Playback Engine.
# Copyright (C) 2013-2020 Ulrich von Zadow
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

import logging

from libavg import avg, app

# Setup Python Logger
hdlr = logging.StreamHandler()
# category is added as an extra formatting key by libavg
formatter = logging.Formatter('[%(asctime)s][%(levelname)s][%(category)s] : %(message)s')
hdlr.setFormatter(formatter)
pyLogger = logging.getLogger(__name__)
pyLogger.addHandler(hdlr)
pyLogger.propagate = False
pyLogger.level = logging.DEBUG


class LoggingTest(app.MainDiv):
    def onInit(self):
        # Add the python logger to libavgs logger as a message sink
        avg.logger.removeStdLogSink()
        avg.logger.addSink(pyLogger)

        avg.logger.debug("Hidden, unless AVG_LOG_CATEGORIES configured with APP:DEBUG")

        avg.logger.configureCategory(avg.logger.Category.APP, avg.logger.Severity.INFO)
        avg.logger.log("Custom Info level message", avg.logger.Category.APP,
                avg.logger.Severity.INFO)

        avg.logger.info("Info level message, with APP Category")
        avg.logger.warning("Warn level message, with APP Category")

        #Remove the logSink, no message should be logged now, if run with
        #AVG_LOG_OMIT_STDERR=1
        #avg.logger.removeSink(logging.getLogger("MY_APP"))

        avg.logger.error("std::err - Error")
        avg.logger.critical("std::err - Critical")
        avg.logger.log("std::err - Log")

        #Register custom log category
        CUSTOM_LOG_CAT = avg.logger.configureCategory("My Custom Category",
                avg.logger.Severity.INFO)

        #Log with custom log category
        avg.logger.log("Message with custom category", CUSTOM_LOG_CAT)
        avg.logger.debug("Hidden message", CUSTOM_LOG_CAT)
        avg.logger.configureCategory(CUSTOM_LOG_CAT, avg.logger.Severity.DBG)
        avg.logger.debug("This will show up", CUSTOM_LOG_CAT)


if __name__ == '__main__':
    app.App().run(LoggingTest(), app_resolution='140x140')

