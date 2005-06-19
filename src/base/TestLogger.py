#!/usr/bin/python

import sys
sys.path.append('/usr/local/lib/python2.3/site-packages/avg')
import avg

Log = avg.Logger.get()
Log.setCategories(Log.APP)
Log.trace(Log.APP, "TestLog")
Log.setDestination("avgtest.log")
Log.trace(Log.APP, "Test log entry in file.")
