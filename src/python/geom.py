# libavg - Media Playback Engine.
# Copyright (C) 2003-2014 Ulrich von Zadow
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

from six.moves import range
from libavg import avg

class RoundedRect(avg.PolygonNode):
    def __init__(self, size, radius, pos=(0,0), parent=None, **kwargs):
        super(RoundedRect, self).__init__(**kwargs)
        self.__pos = avg.Point2D(pos)
        self.__size = avg.Point2D(size)
        self.__radius = radius
        self.__calcPolygon()
        self.registerInstance(self, parent)
        
    def getPos(self):
        return self.__pos

    def setPos(self, pos):
        self.__pos = avg.Point2D(pos)
        self.__calcPolygon()
    polyPos = avg.PolygonNode.pos
    pos = property(getPos, setPos)
        
    def getSize(self):
        return self.__size

    def setSize(self, size):
        self.__size = avg.Point2D(size)
        self.__calcPolygon()
    size = property(getSize, setSize)
    
    def getRadius(self):
        return self.__radius

    def setRadius(self, radius):
        self.__radius = radius
        self.__calcPolygon()
    radius = property(getRadius, setRadius)

    def __calcPolygon(self):
        def calcQuarterCircle(center, r, startAngle):
            pos = []
            for i in range(int(r)+1):
                angle = i*(1.57/r)+startAngle
                p = avg.Point2D(center)+avg.Point2D.fromPolar(angle, r)
                pos.append(p)
            return pos

        r = self.__radius
        if self.__size.x < r*2:
            r = self.__size.x/2
        if self.__size.y < r*2:
            r = self.__size.y/2
        if r == 0:
            r = 0.01
        pos = []
        size = self.__size
        pos.extend(calcQuarterCircle(self.pos+(size.x-r,r), r, -1.57))
        pos.extend(calcQuarterCircle(self.pos+(size.x-r,size.y-r), r, 0))
        pos.extend(calcQuarterCircle(self.pos+(r,size.y-r), r, 1.57))
        pos.extend(calcQuarterCircle(self.pos+(r,r), r, 3.14))
        self.polyPos = pos


class PieSlice(avg.PolygonNode):
    def __init__(self, radius, startangle, endangle, pos=(0,0), parent=None,
            **kwargs):
        super(PieSlice, self).__init__(**kwargs)
        self.__pos = avg.Point2D(pos)
        self.__radius = radius
        self.__startangle = startangle
        self.__endangle = endangle
        self.__calcPolygon()
        self.registerInstance(self, parent)

    def getPos(self):
        return self.__pos

    def setPos(self, pos):
        self.__pos = avg.Point2D(pos)
        self.__calcPolygon()
    polyPos = avg.PolygonNode.pos
    pos = property(getPos, setPos)
        
    def getRadius(self):
        return self.__radius

    def setRadius(self, radius):
        self.__radius = radius
        self.__calcPolygon()
    radius = property(getRadius, setRadius)

    def getStartAngle(self):
        return self.__startangle

    def setStartAngle(self, startangle):
        self.__startangle = startangle
        self.__calcPolygon()
    startangle = property(getStartAngle, setStartAngle)

    def getEndAngle(self):
        return self.__endangle

    def setEndAngle(self, endangle):
        self.__endangle = endangle
        self.__calcPolygon()
    endangle = property(getEndAngle, setEndAngle)

    def __calcPolygon(self):

        def getCirclePoint(i):
            angle = self.__startangle + (self.__endangle-self.__startangle)*i
            return avg.Point2D(self.__pos)+avg.Point2D.fromPolar(angle, self.__radius)

        pos = []
        circlePart = (self.__endangle - self.__startangle)/6.28
        numPoints = self.__radius*2.*circlePart
        if numPoints < 4:
            numPoints = 4
        for i in range(0, int(numPoints)):
            pos.append(getCirclePoint(i/numPoints))
        pos.append(getCirclePoint(1))
        pos.append(self.__pos)
        self.polyPos = pos


class Arc(avg.PolyLineNode):
    # TODO: Code duplication with PieSlice
    def __init__(self, radius, startangle, endangle, pos=(0,0), parent=None,
            **kwargs):
        super(Arc, self).__init__(**kwargs)
        self.__pos = avg.Point2D(pos)
        self.__radius = radius
        self.__startangle = startangle
        self.__endangle = endangle
        self.__calcPolygon()
        self.registerInstance(self, parent)

    def getPos(self):
        return self.__pos

    def setPos(self, pos):
        self.__pos = avg.Point2D(pos)
        self.__calcPolygon()
    polyPos = avg.PolyLineNode.pos
    pos = property(getPos, setPos)
        
    def getRadius(self):
        return self.__radius

    def setRadius(self, radius):
        self.__radius = radius
        self.__calcPolygon()
    radius = property(getRadius, setRadius)

    def getStartAngle(self):
        return self.__startangle

    def setStartAngle(self, startangle):
        self.__startangle = startangle
        self.__calcPolygon()
    startangle = property(getStartAngle, setStartAngle)

    def getEndAngle(self):
        return self.__endangle

    def setEndAngle(self, endangle):
        self.__endangle = endangle
        self.__calcPolygon()
    endangle = property(getEndAngle, setEndAngle)

    def __calcPolygon(self):

        def getCirclePoint(i):
            angle = self.__startangle + (self.__endangle-self.__startangle)*i
            return avg.Point2D(self.__pos)+avg.Point2D.fromPolar(angle, self.__radius)

        pos = []
        circlePart = (self.__endangle - self.__startangle)/6.28
        numPoints = self.__radius*2.*circlePart
        for i in range(0, int(numPoints)):
            pos.append(getCirclePoint(i/numPoints))
        pos.append(getCirclePoint(1))
        self.polyPos = pos

