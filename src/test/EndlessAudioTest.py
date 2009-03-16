#!/usr/bin/python
# -*- coding: utf-8 -*-

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
