#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# libavg - Media Playback Engine.
# Copyright (C) 2003-2011 Ulrich von Zadow
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
from libavg import avg, AVGApp

g_Player = avg.Player.get()

def onFrame():
    curTime = node.getCurTime()
    g_Player.getElementByID("curtime").text = "Time: "+str(curTime/1000.0)

class AudioPlayer(AVGApp):
    def init(self):
        global node
        self._parentNode.appendChild(node)
        avg.WordsNode(parent=self._parentNode, id="curtime", pos=(10, 22), 
                font="arial", fontsize=10)

        g_Player.setOnFrameHandler(onFrame)
    

if len(sys.argv) ==1:
    print
    print "avg_audioplayer.py plays back an audio file using libavg."
    print
    print "Usage: avg_audioplayer.py <filename>"
    sys.exit(1)

node = avg.SoundNode(href=sys.argv[1])
node.play()
AudioPlayer.start(resolution=(320,200))

