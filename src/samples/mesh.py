#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import avg

player = avg.Player.get()

player.loadString("""
<avg width="160" height="120"/>
""")
mesh = avg.MeshNode(texhref="rgb24-64x64.png", 
        vertexcoords=((0,0), (64,0), (0,64), (64, 64), (32, 32)),
        texcoords=((0,0), (1,0), (0,1), (1,1), (0.5,0.5)),
        triangles=((0,1,4), (1,3,4), (3,2,4), (2,0,4)))
player.getRootNode().appendChild(mesh)
player.setResolution(0,640,0,0)
player.play()

