#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import avg

player = avg.Player.get()
player.loadFile("video.avg")
videoNode = player.getElementByID("videoid")
videoNode.play()
player.play()
