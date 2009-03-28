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

