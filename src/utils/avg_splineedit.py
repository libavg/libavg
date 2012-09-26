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

class ControlPoint(avg.DivNode):

    def __init__(self, moveCallback, parent=None, *args, **kwargs):
        super(ControlPoint, self).__init__(**kwargs)
        self.registerInstance(self, parent)
        print self.pos, self.size

        self.__circle = avg.CircleNode(r=15, parent=self)
        self.__posText = avg.WordsNode(pos=(15,-15), parent=self)
        self.__updateLabel()
        self.subscribe(self.CURSOR_DOWN, self.__onDown)
        self.__recognizer = ui.DragRecognizer(eventNode=self, 
                detectedHandler=self.__onDetected, moveHandler=self.__onMove)
        self.__moveCallback = moveCallback
    
    def __onDown(self, event):
        print "__onDown"

    def getPos(self):
        return self.__divPos
    
    def setPos(self, pos):
        self.__divPos = pos
        self.__updateLabel()
        
    __divPos = avg.DivNode.pos
    pos = property(getPos, setPos)
        
    def setActive(self, active):
        if active:
            self.__circle.color = "00FF00"
        else:
            self.__circle.color = "FFFFFF"
        
    def __onDetected(self):
        self.__dragStartPos = self.pos
        self.pos = self.__moveCallback(self)
        
    def __onMove(self, offset):
        self.pos = self.__dragStartPos + offset
        self.pos = self.__moveCallback(self)
        self.__updateLabel()
    
    def __updateLabel(self):
        splinePos = SplineEditor.get().cvtNode2SplineCoords(self.pos)
        self.__posText.text = "(%.2f,%.2f)" % (splinePos.x,splinePos.y)

class SplineEditor(AVGApp):
   
    def init(self):
        global anchors
        self.__buttonDiv = avg.DivNode(pos=(630,20), parent=self._parentNode)

        button = simple.TextButton(pos=(0,0), text="Add Point", size=(150,22), 
                parent=self.__buttonDiv)
        button.subscribe(button.CLICKED, self.__onAddPoint)
        button = simple.TextButton(pos=(0,30), text="Delete Point", size=(150,22), 
                parent=self.__buttonDiv)
        button.subscribe(button.CLICKED, self.__onDeletePoint)
        button = simple.TextButton(pos=(0,60), text="Dump to Console", size=(150,22), 
                parent=self.__buttonDiv)
        button.subscribe(button.CLICKED, self.__onDump)

        self.__anchors = anchors
        self.__minY = 1e99
        self.__maxY = -1e99
        for anchor in self.__anchors:
            if anchor.y < self.__minY:
                self.__minY = anchor.y
            if anchor.y > self.__maxY:
                self.__maxY = anchor.y
        
        self.__curveDiv = avg.DivNode(pos=(20,20), parent=self._parentNode)
        self.__curve = avg.PolyLineNode(strokewidth=2, color="FFFFFF",
                parent=self.__curveDiv)
        self.__genCurve()
        
        self.__controlPoints = []
        
        for i, anchor in enumerate(self.__anchors):
            controlPoint = self.__createControlPoint(i, anchor)
            self.__controlPoints.append(controlPoint)
        self.__curCtlPt = None

    def moveAnchor(self, controlPoint):
        i = self.__controlPoints.index(controlPoint)
        pos = controlPoint.pos
        anchor = self.cvtNode2SplineCoords(pos)
        if i == 0:
            anchor.x = self.__anchors[0].x
            self.__anchors[-1].y = anchor.y
            self.__controlPoints[-1].pos = self.cvtSpline2NodeCoords(self.__anchors[-1])
        elif i == len(self.__anchors) - 1:
            anchor.x = self.__anchors[-1].x
            self.__anchors[0].y = anchor.y
            self.__controlPoints[0].pos = self.cvtSpline2NodeCoords(self.__anchors[0])
        else:
            if anchor.x >= self.__anchors[i+1].x:
                anchor.x = self.__anchors[i+1].x - 0.01
            elif anchor.x <= self.__anchors[i-1].x:
                anchor.x = self.__anchors[i-1].x + 0.01
        
        if self.__curCtlPt is not None:
            self.__controlPoints[self.__curCtlPt].setActive(False)
        self.__curCtlPt = i
        self.__controlPoints[self.__curCtlPt].setActive(True)
        
        self.__anchors[i] = anchor
        self.__genCurve()
        return self.cvtSpline2NodeCoords(anchor)

    def __genCurve(self):
        self.__spline = avg.CubicSpline(self.__anchors, False)
        pts = []
        minPos, maxPos = self.__getMinMaxVal()
        xscale = (maxPos.x - minPos.x)/600
        for x in xrange(600):
            y = 400 * (1-self.__spline.interpolate(x*xscale-minPos.x))
            pts.append((x, y))
        self.__curve.pos = pts

    def __onAddPoint(self):
        if self.__curCtlPt is not None:
            i = self.__curCtlPt
            if i == len(self.__anchors)-1:
                i -= 1
                self.__curCtlPt += 1
            pos = (self.__anchors[i] + self.__anchors[i+1]) / 2
            self.__anchors.insert(i+1, pos)
            controlPoint = self.__createControlPoint(i+1, pos)
            self.__controlPoints.insert(i+1, controlPoint)
            self.__genCurve()

    def __onDeletePoint(self):
        i = self.__curCtlPt
        if i is not None and i != 0 and i != len(self.__anchors)-1:
            self.__anchors.pop(i)
            controlPoint = self.__controlPoints.pop(i)
            controlPoint.unlink(True)
            self.__genCurve()
            self.__curCtlPt = None
    
    def __onDump(self):
        print "[",
        for anchor in self.__anchors:
            if anchor != self.__anchors[-1]:
                print "(%.2f, %.2f)," % (anchor.x, anchor.y),
            else:
                print "(%.2f, %.2f) ]" % (anchor.x, anchor.y)

    def __createControlPoint(self, i, anchor):
        return ControlPoint(pos=self.cvtSpline2NodeCoords(anchor),
                    moveCallback=self.moveAnchor, 
                    parent=self.__curveDiv)

    def cvtSpline2NodeCoords(self, pos):
        minPos, maxPos = self.__getMinMaxVal()
        normPt = avg.Point2D()
        pos = avg.Point2D(pos)
        normPt.x = (pos.x-minPos.x) / (maxPos.x-minPos.x)
        normPt.y = (pos.y-minPos.y) / (maxPos.y-minPos.y)
        return avg.Point2D(normPt.x*600, 400-normPt.y*400)
    
    def cvtNode2SplineCoords(self, pos):
        pos = avg.Point2D(pos)
        normPt = avg.Point2D(pos.x/600, pos.y/400)
        minPos, maxPos = self.__getMinMaxVal()
        return avg.Point2D(normPt.x*(maxPos.x-minPos.x) + minPos.x, 
                maxPos.y - (normPt.y*(maxPos.y-minPos.y) + minPos.y))

    def __getMinMaxVal(self):
        return (avg.Point2D(self.__anchors[0][0], self.__minY),
                avg.Point2D(self.__anchors[-1][0], self.__maxY))
        

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
        for i, anchor in enumerate(anchors):
            anchors[i] = avg.Point2D(anchor)
    except:
        typeOk = False
        
if not(typeOk):
    print 'Can\'t parse anchor points. The points argument must be a quoted list of 2D coordinates (e.g. "[(0,1), (1,2)]").'
    print
    parser.print_help()
    exit(5)

SplineEditor.start(resolution=(800,600))

