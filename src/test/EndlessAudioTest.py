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

import sys, time, os, platform

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

def eof():
    print "eof"

def play():
    global node
    print "play"
    node.play()

Log = avg.Logger.get()
Log.setCategories(Log.APP |
        Log.WARNING |
         Log.PROFILE |
#         Log.PROFILE_LATEFRAMES |
         Log.CONFIG
#         Log.MEMORY |
#         Log.BLTS    |
#         Log.EVENTS |
#         Log.EVENTS2
              )

Player = avg.Player.get()
self._loadEmpty()
root = Player.getRootNode()
node = Player.createNode("sound",
        {"href":"../video/testfiles/44.1kHz_16bit_mono.wav"})
node.setEOFCallback(eof)
root.appendChild(node)
Player.setInterval(120, play)
Player.play()
