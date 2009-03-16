#!/usr/bin/python
# -*- coding: utf-8 -*-
import unittest

import sys
sys.path.append('/usr/local/lib/python2.3/site-packages/avg')
import avg
import time

class ParPortTestCase(unittest.TestCase):
    def test(self):
        def setAllLines(val):
            self.ParPort.setControlLine(avg.CONTROL_STROBE, val)
            self.ParPort.setControlLine(avg.CONTROL_AUTOFD, val)
            self.ParPort.setControlLine(avg.CONTROL_SELECT, val)
            self.ParPort.setControlLine(avg.CONTROL_INIT, val)
        self.ParPort = avg.ParPort()
        self.ParPort.init("")
        setAllLines(1)
        time.sleep(0.5)
        setAllLines(0)
        print self.ParPort.getStatusLine(avg.STATUS_ERROR)
        print self.ParPort.getStatusLine(avg.STATUS_SELECT)
        print self.ParPort.getStatusLine(avg.STATUS_PAPEROUT)
        print self.ParPort.getStatusLine(avg.STATUS_ACK)
        print self.ParPort.getStatusLine(avg.STATUS_BUSY)
        self.ParPort.setDataLines(avg.PARPORTDATA0 | avg.PARPORTDATA1)
        time.sleep(0.2)
        self.ParPort.setDataLines(avg.PARPORTDATA2 | avg.PARPORTDATA3)
        time.sleep(0.2)
        self.ParPort.clearDataLines(avg.PARPORTDATA2 | avg.PARPORTDATA3)
        time.sleep(0.2)
        self.ParPort.clearDataLines(avg.PARPORTDATA0 | avg.PARPORTDATA1)

def playerTestSuite():
    suite = unittest.TestSuite()
    suite.addTest(ParPortTestCase("test"))
    return suite

Player = avg.Player.get()

runner = unittest.TextTestRunner()
runner.run(playerTestSuite())

