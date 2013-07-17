#!/usr/bin/env python
# -*- coding: utf-8 -*-

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


class LoggingTest(app.MainDiv):
    def onInit(self):
        # Add the python logger to libavgs logger as a message sink
        avg.logger.addSink(pyLogger)

        avg.logger.log("Custom Info level message",avg.logger.APP, avg.logger.INFO)

        avg.logger.debug("Debug level message, with APP Category")
        avg.logger.info("Info level message, with APP Category")
        avg.logger.warning("Warn level message, with APP Category")

        #Remove the logSink, no message should be logged now, if run with
        #AVG_LOG_OMIT_STDERR=1
        avg.logger.removeSink(logging.getLogger("MY_APP"))

        avg.logger.error("std::err - Error")
        avg.logger.critical("std::err - Critical")
        avg.logger.log("std::err - Log")

        #Register custom log category
        CUSTOM_LOG_CAT = avg.logger.registerCategory("My Custom Category")

        #Enable custom log category
        currentCats = avg.logger.getCategories()
        avg.logger.setCategories(currentCats | CUSTOM_LOG_CAT)

        #Log with custom log category
        avg.logger.log("Message with custom category", CUSTOM_LOG_CAT)

app.App().run(LoggingTest(), app_resolution='40x40')
