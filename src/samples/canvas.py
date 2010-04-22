#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import avg

player = avg.Player.get()
player.loadCanvasFile("canvas.avg")
player.loadString("""
  <avg width="640" height="480">
    <image href="canvas:londoncalling"/>
  </avg>
""")
player.play()
