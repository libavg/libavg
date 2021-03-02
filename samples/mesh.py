#!/usr/bin/env python
# -*- coding: utf-8 -*-

# libavg - Media Playback Engine.
# Copyright (C) 2010-2021 Ulrich von Zadow
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
# Current versions can be found at www.libavg.de

from libavg import avg, player

canvas = player.createMainCanvas(size=(160,120))
avg.MeshNode(texhref="rgb24-64x64.png", 
        vertexcoords=((0,0), (64,0), (0,64), (64, 64), (32, 32)),
        texcoords=((0,0), (1,0), (0,1), (1,1), (0.5,0.5)),
        triangles=((0,1,4), (1,3,4), (3,2,4), (2,0,4)),
        parent=player.getRootNode())
player.play()

