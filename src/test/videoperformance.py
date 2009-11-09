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
from libavg import avg
import time

def videoPlay(nodeName):
    node = AVGPlayer.getElementByID(nodeName)
    print "Starting "+nodeName
    node.play()

def rotate():
    for i in range(6):
        node = AVGPlayer.getElementByID("mpeg"+str(i+1))
        node.angle += 0.02

log = avg.Logger.get()
log.setCategories(log.CONFIG | log.PROFILE)

AVGPlayer = avg.Player.get()
AVGPlayer.loadFile("videoperformance.avg")

AVGPlayer.setTimeout(10, lambda: videoPlay('mpeg1'))
AVGPlayer.setTimeout(40, lambda: videoPlay('mpeg2'))
AVGPlayer.setTimeout(80, lambda: videoPlay('mpeg3'))
#AVGPlayer.setTimeout(120, lambda: videoPlay('mpeg4'))
#AVGPlayer.setTimeout(160, lambda: videoPlay('mpeg5'))
#AVGPlayer.setTimeout(200, lambda: videoPlay('mpeg6'))
#AVGPlayer.setTimeout(24000, lambda: videoPlay('mpeg7'))
#AVGPlayer.setTimeout(28000, lambda: videoPlay('mpeg8'))
AVGPlayer.play()
