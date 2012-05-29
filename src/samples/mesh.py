#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import avg, player

canvas = player.createMainCanvas(size=(160,120))
avg.MeshNode(texhref="rgb24-64x64.png", 
        vertexcoords=((0,0), (64,0), (0,64), (64, 64), (32, 32)),
        texcoords=((0,0), (1,0), (0,1), (1,1), (0.5,0.5)),
        triangles=((0,1,4), (1,3,4), (3,2,4), (2,0,4)),
        parent=player.getRootNode())
player.play()

