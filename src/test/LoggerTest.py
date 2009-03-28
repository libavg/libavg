#!/usr/bin/python
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

import unittest

import sys, os, platform
import tempfile

try:
    import syslog
    SYSLOG_AVAILABLE = True
except ImportError:
    SYSLOG_AVAILABLE = False

# Import the correct version of libavg. Since it should be possible to
# run the tests without installing libavg, we add the location of the 
# uninstalled libavg to the path.
# TODO: This is a mess. 
sys.path += ['../wrapper/.libs', '../python']
if platform.system() == 'Darwin':
    sys.path += ['../..']     # Location of libavg in a mac installation. 

if platform.system() == 'Windows':
    from libavg import avg    # Under windows, there is no uninstalled version.
else:    
    import avg

class LoggerTestCase(unittest.TestCase):
    def runTest(self):
        self.Log = avg.Logger.get()
        self.Log.setCategories(self.Log.APP |
                  self.Log.WARNING
#                  self.Log.PROFILE |
#                  self.Log.PROFILE_LATEFRAMES |
#                  self.Log.CONFIG |
#                  self.Log.MEMORY |
#                  self.Log.BLTS    |
#                  self.Log.EVENTS |
#                  self.Log.EVENTS2
                  )
        myTempFile = os.path.join(tempfile.gettempdir(), "testavg.log")
        try:
            os.remove(myTempFile)
        except OSError:
            pass
        self.Log.setFileDest(myTempFile)
        self.Log.trace(self.Log.APP, "Test file log entry.")
        readLog = file(myTempFile, "r").readlines()
        self.assert_(len(readLog) == 1)
        myBaseLine = "APP: Test file log entry."
        self.assert_(readLog[0].find(myBaseLine) >= 0)
        stats = os.stat(myTempFile)
        # Windows text files have two chars for linefeed
        self.assert_(stats.st_size in [50, 51])
        
        if SYSLOG_AVAILABLE:
            self.Log.setSyslogDest(syslog.LOG_USER, syslog.LOG_CONS)
            self.Log.trace(self.Log.APP, "Test syslog entry.")
        self.Log.setConsoleDest()


def loggerTestSuite (tests):
    suite = unittest.TestSuite()
    suite.addTest (LoggerTestCase())
    return suite

Player = avg.Player.get()

