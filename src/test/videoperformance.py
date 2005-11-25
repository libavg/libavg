#!/usr/bin/python
# -*- coding: utf-8 -*-

import sys
sys.path.append('/usr/local/lib/python2.4/site-packages/libavg')
import avg
import time

def videoPlay(nodeName):
    node = AVGPlayer.getElementByID(nodeName)
    print "Starting "+nodeName
    node.play()

def rotate():
    for i in range(6):
        node = AVGPlayer.getElementByID("mpeg"+str(i+1))
        node.angle += 0.02

Log = avg.Logger.get();
Log.setCategories(Log.APP |
          Log.WARNING | 
          Log.PROFILE |
          Log.PROFILE_LATEFRAMES |
          Log.CONFIG |
          Log.EVENTS)

AVGPlayer = avg.Player()
AVGPlayer.setResolution(0,0,0,0)

AVGPlayer.loadFile("videoperformance.avg")

AVGPlayer.setInterval(10, rotate)
AVGPlayer.setTimeout(10, lambda: videoPlay('mpeg1'))
AVGPlayer.setTimeout(40, lambda: videoPlay('mpeg2'))
AVGPlayer.setTimeout(80, lambda: videoPlay('mpeg3'))
AVGPlayer.setTimeout(120, lambda: videoPlay('mpeg4'))
AVGPlayer.setTimeout(160, lambda: videoPlay('mpeg5'))
AVGPlayer.setTimeout(200, lambda: videoPlay('mpeg6'))
#AVGPlayer.setTimeout(24000, lambda: videoPlay('mpeg7'))
#AVGPlayer.setTimeout(28000, lambda: videoPlay('mpeg8'))
AVGPlayer.setVBlankFramerate(2)
AVGPlayer.play()

