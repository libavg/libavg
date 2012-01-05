#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import avg

player = avg.Player.get()

canvas = player.createMainCanvas(size=(160,120))
# Change following line if the plugin is somewhere else.
player.pluginPath = "../test/plugin/.libs"
player.loadPlugin("colorplugin")

rootNode = canvas.getRootNode()
node = colorplugin.ColorNode(fillcolor="7f7f00", parent=rootNode)
node.fillcolor = "7f007f"
player.play()

