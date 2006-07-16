#!/usr/bin/python
import unittest
import sys, time, os

import avg

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
        time.sleep(0.1)
        setAllLines(0)
        self.ParPort.getStatusLine(avg.STATUS_ERROR)
        self.ParPort.getStatusLine(avg.STATUS_SELECT)
        self.ParPort.getStatusLine(avg.STATUS_PAPEROUT)
        self.ParPort.getStatusLine(avg.STATUS_ACK)
        self.ParPort.getStatusLine(avg.STATUS_BUSY)
        self.ParPort.setDataLines(avg.PARPORTDATA0 | avg.PARPORTDATA1)
        time.sleep(0.05)
        self.ParPort.setDataLines(avg.PARPORTDATA2 | avg.PARPORTDATA3)
        time.sleep(0.05)
        self.ParPort.clearDataLines(avg.PARPORTDATA2 | avg.PARPORTDATA3)
        time.sleep(0.05)
        self.ParPort.clearDataLines(avg.PARPORTDATA0 | avg.PARPORTDATA1)

class ConradRelaisTestCase(unittest.TestCase):
    def test(self):
        ConradRelais = avg.ConradRelais(Player, 0)
        print ConradRelais.getNumCards()
        for i in range(6):
            ConradRelais.set(0, i, 1)

def hardwareTestSuite():
    suite = unittest.TestSuite()
    suite.addTest(ParPortTestCase("test"))
    suite.addTest(ConradRelaisTestCase("test"))
    return suite
    
Player = avg.Player()
runner = unittest.TextTestRunner()
runner.run(hardwareTestSuite())

