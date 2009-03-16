#!/usr/bin/python
# -*- coding: utf-8 -*-

import sys
sys.path.append('/usr/local/lib/python2.4/site-packages/libavg')
from libavg import avg
import time

def videoPlay(nodeName):
    node = AVGPlayer.getElementByID(nodeName)
    print "Starting "+nodeName
    node.play()

def rotate():
    for i in range(2):
        node = AVGPlayer.getElementByID("hd"+str(i+1))
        node.angle += 0.02

Log = avg.Logger.get();
Log.setCategories(Log.APP |
          Log.WARNING | 
          Log.PROFILE |
#          Log.PROFILE_LATEFRAMES |
          Log.CONFIG |
          Log.EVENTS)

AVGPlayer = avg.Player.get()
AVGPlayer.setResolution(0,0,0,0)

AVGPlayer.loadFile("videoperformance_hd.avg")

#AVGPlayer.setInterval(10, rotate)
AVGPlayer.setTimeout(10, lambda: videoPlay('hd1'))
AVGPlayer.setTimeout(40, lambda: videoPlay('hd2'))
AVGPlayer.setVBlankFramerate(2)
AVGPlayer.play()

