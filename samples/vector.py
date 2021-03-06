#!/usr/bin/env python
# -*- coding: utf-8 -*-

# libavg - Media Playback Engine.
# Copyright (C) 2014-2021 Ulrich von Zadow
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

import libavg
from libavg import app, geom


class VectorDiv(app.MainDiv):
    def onInit(self):
        self.toggleTouchVisualization()
    
        libavg.CircleNode(pos=(20,20), color="30E030", r=15, parent=self)
        libavg.CircleNode(pos=(60,20), fillopacity=1, fillcolor="E03030", r=15,
                parent=self)
        libavg.CircleNode(pos=(100,20), fillopacity=1, fillcolor="E03030", opacity=0,
                r=15, parent=self)
        libavg.CircleNode(pos=(140,20), fillopacity=1, filltexhref="rgb24-64x64.png",
                r=15, parent=self)
        libavg.CircleNode(pos=(180,20), fillopacity=1, filltexhref="rgb24-64x64.png",
                filltexcoord1=(0,0), filltexcoord2=(2,2), r=15, parent=self)
        libavg.CircleNode(pos=(220,20), strokewidth=5, texhref="dottedline.png",
                texcoord1=0, texcoord2=8, r=15, parent=self)

        polygonPos1 = [(20,55), (30,50), (40,55), (50,50), (60,55), (60,65),
                (50,70), (40,65), (30,70), (20,65)]
        libavg.PolygonNode(pos=polygonPos1, parent=self)
        
        polygonPos2 = [libavg.Point2D(60,0) + pt for pt in polygonPos1]
        libavg.PolygonNode(pos=polygonPos2, fillopacity=1, fillcolor="E03030", 
                parent=self)

        polygonPos3 = [libavg.Point2D(120,0) + pt for pt in polygonPos1]
        libavg.PolygonNode(pos=polygonPos3, strokewidth=5, linejoin="bevel",
                parent=self)

        polygonPos4 = [libavg.Point2D(180,0) + pt for pt in polygonPos1]
        libavg.PolygonNode(pos=polygonPos4, strokewidth=5, linejoin="miter",
                parent=self)

        geom.Arc(pos=(20,100), radius=15, startangle=0, endangle=3.1415, parent=self)
        geom.PieSlice(pos=(60,100), radius=15, startangle=0, endangle=1, parent=self)
        geom.RoundedRect(pos=(85,85), size=(30,30), radius=5, parent=self)


app.App().run(VectorDiv())

