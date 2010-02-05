#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import avg

Player = avg.Player.get()

Player.loadString("""<avg width="160" height="120" id="container" />""")
Player.pluginPath = "/Users/uzadow/devel/libavg/libavg/src/test/plugin/.libs"
Player.loadPlugin("libColorNode")

node = Player.createNode('<colornode fillcolor="0f3f7f" id="mynode" />')
Player.getElementByID("container").appendChild(node)
mynode = Player.getElementByID("mynode")
mynode.fillcolor = "7f007f"
Player.play()
