#!/usr/bin/python
# -*- coding: utf-8 -*-

import sys, syslog
sys.path.append('/usr/local/lib/python2.3/site-packages/libavg')
sys.path.append('/usr/local/lib/python2.4/site-packages/libavg')
import avg
import time
import anim

def dumpMouseEvent():
    Event = Player.getCurEvent()
    print Event
    print "  type: "+str(Event.type)
    print "  leftbuttonstate: "+str(Event.leftbuttonstate)
    print "  middlebuttonstate: "+str(Event.middlebuttonstate)
    print "  rightbuttonstate: "+str(Event.rightbuttonstate)
    print "  position: "+str(Event.x)+","+str(Event.y)
    print "  node: "+Event.node.id

def onMouseOver():
    print ("onMouseOver()")
    dumpMouseEvent()

def onMouseOut():
    print ("onMouseOut()")
    dumpMouseEvent()

Player = avg.Player()
Log = avg.Logger.get()
Log.setCategories(Log.APP |
            Log.WARNING | 
            Log.PROFILE |
            Log.CONFIG |
            Log.MEMORY | 
            Log.EVENTS
            )

Player.loadFile("TestEvents.avg")
Player.setVBlankFramerate(2)
Player.play()

