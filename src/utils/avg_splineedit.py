#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# libavg - Media Playback Engine.
# Copyright (C) 2003-2011 Ulrich von Zadow
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
#

import optparse
from libavg import avg, AVGApp, ui
from libavg.ui import simple

g_Player = avg.Player.get()

class ControlPoint(avg.CircleNode):
    def __init__(self, moveCallback, parent, *args, **kwargs):
        super(ControlPoint, self).__init__(**kwargs)
        if parent:
            parent.appendChild(self)
        self.r = 15
        ui.DragRecognizer(eventNode=self, detectedHandler=self.__onDetected,
                moveHandler=self.__onMove)
        self.__moveCallback = moveCallback
        
    def __onDetected(self, event):
        self.__dragStartPos = self.pos

    def __onMove(self, event, offset):
        self.pos = self.__dragStartPos + offset
        self.__moveCallback(self.pos)
#        moveNodeOnScreen(self)

class SplineEditor(AVGApp):
   
    def init(self):
        global anchors
        self.__buttonDiv = avg.DivNode(pos=(630,20), parent=self._parentNode)

        simple.TextButton(pos=(0,0), text="Add Point", size=(150,22), 
                clickHandler=self.__onAddPoint, parent=self.__buttonDiv)
        simple.TextButton(pos=(0,30), text="Delete Point", size=(150,22), 
                clickHandler=self.__onDeletePoint, parent=self.__buttonDiv)
        simple.TextButton(pos=(0,60), text="Dump to Console", size=(150,22), 
                clickHandler=self.__onDump, parent=self.__buttonDiv)

        self.__anchors = anchors
        self.__curveDiv = avg.DivNode(pos=(20,20), parent=self._parentNode)
        self.__curve = avg.PolyLineNode(strokewidth=2, color="FFFFFF",
                parent=self.__curveDiv)
        self.__genCurve()
        
        self.__controlPoints = []
        for i, anchor in enumerate(self.__anchors):
            controlPoint = ControlPoint(pos=self.__cvtSpline2NodeCoords(anchor),
                    moveCallback=lambda pos, i=i: self.moveAnchor(i, pos), 
                    parent=self.__curveDiv)
            self.__controlPoints.append(controlPoint)

    def moveAnchor(self, i, pos):
        self.__anchors[i] = self.__cvtNode2SplineCoords(pos)
        self.__genCurve()

    def __genCurve(self):
        anchors = sorted(self.__anchors, key=lambda pos: pos[0])
        self.__spline = avg.CubicSpline(anchors, False)
        pts = []
        minPos, maxPos = self.__getMinMaxVal()
        xscale = (maxPos.x - minPos.x)/600
        for x in xrange(600):
            y = 400 * (1-self.__spline.interpolate(x*xscale-minPos.x))
            pts.append((x, y))
        self.__curve.pos = pts

    def __onAddPoint(self, event):
        pass

    def __onDeletePoint(self, event):
        pass
    
    def __onDump(self, event):
        pass

    def __cvtSpline2NodeCoords(self, pos):
        minPos, maxPos = self.__getMinMaxVal()
        normPt = avg.Point2D()
        pos = avg.Point2D(pos)
        normPt.x = (pos.x-minPos.x) / (maxPos.x-minPos.x)
        normPt.y = (pos.y-minPos.y) / (maxPos.y-minPos.y)
        return avg.Point2D(normPt.x*600, 400-normPt.y*400)
    
    def __cvtNode2SplineCoords(self, pos):
        pos = avg.Point2D(pos)
        normPt = avg.Point2D(pos.x/600, pos.y/400)
        minPos, maxPos = self.__getMinMaxVal()
        return avg.Point2D(normPt.x*(maxPos.x-minPos.x) + minPos.x, 
                maxPos.y - (normPt.y*(maxPos.y-minPos.y) + minPos.y))

    def __getMinMaxVal(self):
        return (avg.Point2D(self.__anchors[0][0], 0),
                avg.Point2D(self.__anchors[-1][0], 1))
        

parser = optparse.OptionParser(usage="Usage: %prog points [options]",
        description="An editor for libavg cubic splines.")
#parser.add_argument("spline", type="list",
#        help="The spline anchor points as they would be specified when constructing a CubicSpline.")
parser.add_option("-a", "--aspect", dest="aspect", default="1", type="float",
        help="Aspect ratio for the graph display. Does not affect the actual spline data.")
parser.add_option("-l", "--disable-loop", action="store_false", default=True,
        help="Corresponds to the loop parameter when constructing the spline.")

(options, args) = parser.parse_args()

# Convert command-line argument into list of points
anchorString = args[0]
try:
    anchors = eval(anchorString)
    typeOk = isinstance(anchors, (list, tuple))
except:
    typeOk = False
  
if typeOk:
    try:  
        for anchor in anchors:
            pt = avg.Point2D(anchor)
    except:
        typeOk = False
        
if not(typeOk):
    print "Can't parse anchor points. The points argument must be a quoted list of 2D coordinates."
    print
    parser.print_help()
    exit(5)

SplineEditor.start(resolution=(800,600))

