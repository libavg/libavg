#!/usr/bin/python
# -*- coding: utf-8 -*-
import unittest

import sys
sys.path.append('/usr/local/lib/python2.3/site-packages/avg')
import avg

def keyup():
    def printPPLine(line, name):
        print name,
        if ParPort.getStatusLine(line):
            print ": off",
        else:
            print ":  on",
    Event = Player.getCurEvent()
    print Event.keystring
    if Event.keystring == "1":
        ParPort.setDataLines(avg.PARPORTDATA1)
    elif Event.keystring == "2":
        ParPort.setDataLines(avg.PARPORTDATA2)
    elif Event.keystring == "3":
        ParPort.setDataLines(avg.PARPORTDATA3)
    elif Event.keystring == "4":
        ParPort.setDataLines(avg.PARPORTDATA4)
    elif Event.keystring == "5":
        ParPort.setDataLines(avg.PARPORTDATA5)
    elif Event.keystring == "6":
        ParPort.setDataLines(avg.PARPORTDATA6)
    elif Event.keystring == "7":
        ParPort.setDataLines(avg.PARPORTDATA7)
    elif Event.keystring == "8":
        ParPort.setDataLines(avg.PARPORTDATA0)
    elif Event.keystring == "q":
        ParPort.clearDataLines(avg.PARPORTDATA1)
    elif Event.keystring == "w":
        ParPort.clearDataLines(avg.PARPORTDATA2)
    elif Event.keystring == "e":
        ParPort.clearDataLines(avg.PARPORTDATA3)
    elif Event.keystring == "r":
        ParPort.clearDataLines(avg.PARPORTDATA4)
    elif Event.keystring == "t":
        ParPort.clearDataLines(avg.PARPORTDATA5)
    elif Event.keystring == "z":
        ParPort.clearDataLines(avg.PARPORTDATA6)
    elif Event.keystring == "u":
        ParPort.clearDataLines(avg.PARPORTDATA7)
    elif Event.keystring == "i":
        ParPort.clearDataLines(avg.PARPORTDATA0)
    elif Event.keystring == "a":
        ParPort.setControlLine(avg.CONTROL_STROBE, 1)
    elif Event.keystring == "s":
        ParPort.setControlLine(avg.CONTROL_AUTOFD, 1)
    elif Event.keystring == "f":
        ParPort.setControlLine(avg.CONTROL_SELECT, 1)
    elif Event.keystring == "d":
        ParPort.setControlLine(avg.CONTROL_INIT, 1)
    elif Event.keystring == "y":
        ParPort.setControlLine(avg.CONTROL_STROBE, 0)
    elif Event.keystring == "x":
        ParPort.setControlLine(avg.CONTROL_AUTOFD, 0)
    elif Event.keystring == "c":
        ParPort.setControlLine(avg.CONTROL_SELECT, 0)
    elif Event.keystring == "v":
        ParPort.setControlLine(avg.CONTROL_INIT, 0)
    printPPLine (avg.STATUS_ERROR, "STATUS_ERROR")
    print ", ",
    printPPLine (avg.STATUS_SELECT, "STATUS_SELECT")
    print ", ",
    printPPLine (avg.STATUS_PAPEROUT, "STATUS_PAPEROUT")
    print ", ",
    printPPLine (avg.STATUS_ACK, "STATUS_ACK")
    print ", ",
    printPPLine (avg.STATUS_BUSY, "STATUS_BUSY")
    print

ParPort = avg.ParPort()
ParPort.init("")

Player = avg.Player.get()
Player.loadFile("parport.avg")
Player.play(30)

