#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import avg

Player = avg.Player.get()
Player.loadFile("text.avg")
Player.play()
