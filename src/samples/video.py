#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import avg

player = avg.Player.get()
player.loadString("""<avg size="(160,120)"/>""")
videoNode = avg.VideoNode(href="mpeg1-48x48-sound.avi", pos=(10,10), 
        parent=player.getRootNode())
videoNode.play()
player.play()
