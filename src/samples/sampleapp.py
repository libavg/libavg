#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import avg, AVGApp, AVGAppUtil, anim
from libavg.AVGAppUtil import getMediaDir

# usually you need the Player in many places, so it's useful
# to keep it in a global variable (it's a singleton anyway).
g_player = avg.Player.get()

class HelloWorld(AVGApp):
    def init(self):
        # Put all your nodes in the hierarchy below self._parentNode
        self.node = avg.WordsNode(pos=(50,50), text="Hello World", 
                parent=self._parentNode)

    def _enter(self):
        # You should start and stop all animations, intervals etc.
        # in _enter and _leave, so your application uses only
        # minimal resources while it is not running.
        self.anim = anim.ContinuousAnim(self.node, 'angle', 0, 3.14)
        self.anim.start()

    def _leave(self):
        self.anim.abort()
        self.anim = None

HelloWorld.start(resolution=(640, 480))

