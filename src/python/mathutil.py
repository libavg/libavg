# libavg - Media Playback Engine.
# Copyright (C) 2003-2008 Ulrich von Zadow
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
# Original author of this file is Martin Heistermann <mh at sponc dot de>
#

# TODO: Some of this stuff is duplicated - either in Point2D or in MathHelper.h/.cpp.
# Clean that up.

import math
from libavg import Point2D

def getAngle(p1, p2):
    vec = p2 - p1
    res = math.atan2(vec.y, vec.x)
    if res < 0:
        res += math.pi * 2
    return res

def getDistance (p, q):
    return math.sqrt((p.x-q.x)**2 + (p.y-q.y)**2)

def getDistSquared (p, q):
    return (p.x-q.x)**2 + (p.y-q.y)**2

def getScaleToSize ((width, height), (max_width, max_height)):
    if width < max_width:
        height = height * (float(max_width) / width)
        width = max_width
    elif height > max_height:
        width = width * (float(max_height) / height)
        height = max_height
    return getScaledDim((width, height), (max_width, max_height))

def getScaledDim (size, max = None, min = None):
    width, height = size
    if width == 0 or height == 0:
        return size

    if max:
        max = Point2D(max)
        assert (max.x > 0 and max.y > 0)
        if width > max.x:
            height = height * (max.x / width)
            width = max.x
        if height > max.y:
            width = width * (max.y / height)
            height = max.y

    if min:
        min = Point2D(min)
        assert (min.x > 0 and min.y > 0)
        if width < min.x:
            height = height * (min.x / width)
            width = min.x
        if height < min.y:
            width = width * (min.y / height)
            height = min.y

    return Point2D(width, height)


class EquationNotSolvable (Exception):
    pass
class EquationSingular (Exception):
    pass

def gauss_jordan(m, eps = 1.0/(10**10)):
    """Puts given matrix (2D array) into the Reduced Row Echelon Form.
         Returns True if successful, False if 'm' is singular.
         NOTE: make sure all the matrix items support fractions! Int matrix will NOT work!
         Written by Jarno Elonen in April 2005, released into Public Domain
         http://elonen.iki.fi/code/misc-notes/affine-fit/index.html"""
    (h, w) = (len(m), len(m[0]))
    for y in range(0,h):
        maxrow = y
        for y2 in range(y+1, h):        # Find max pivot
            if abs(m[y2][y]) > abs(m[maxrow][y]):
                maxrow = y2
        (m[y], m[maxrow]) = (m[maxrow], m[y])
        if abs(m[y][y]) <= eps:         # Singular?
            raise EquationSingular
        for y2 in range(y+1, h):        # Eliminate column y
            c = m[y2][y] / m[y][y]
            for x in range(y, w):
                m[y2][x] -= m[y][x] * c
    for y in range(h-1, 0-1, -1): # Backsubstitute
        c    = m[y][y]
        for y2 in range(0,y):
            for x in range(w-1, y-1, -1):
                m[y2][x] -=    m[y][x] * m[y2][y] / c
        m[y][y] /= c
        for x in range(h, w):             # Normalize row y
            m[y][x] /= c
    return m


def solveEquationMatrix(_matrix, eps = 1.0/(10**10)):
    matrix=[]
    for coefficients, res in _matrix:
        newrow = map(float, coefficients + (res,))
        matrix.append(newrow)
    matrix = gauss_jordan (matrix)
    res=[]
    for col in xrange(len(matrix[0])-1):
        rows = filter(lambda row: row[col] >= eps, matrix)
        if len(rows)!=1:
            raise EquationNotSolvable
        res.append (rows[0][-1])

    return res


def getOffsetForMovedPivot(oldPivot, newPivot, angle):
    oldPos = Point2D(0,0).getRotated(angle, oldPivot)
    newPos = Point2D(0,0).getRotated(angle, newPivot)
    return oldPos - newPos

def isNaN(x):
    return (not(x<=0) and not(x>=0))

def sgn (x):
    if x<0:
        return -1
    elif x==0:
        return 0
    else:
        return 1

class MovingAverage:
    """
    Moving average implementation.
    Example:
    ma = MovingAverage(20)
    print ma(2)
    print ma(3)
    print ma(10)
    """
    def __init__(self, points):
        self.__points = points
        self.__values = []

    def __appendValue(self, value):
        self.__values = (self.__values + [value])[-self.__points:]

    def __getAverage(self):
        sum = reduce(lambda a,b:a+b, self.__values)
        return float(sum) / len(self.__values)

    def __call__(self, value):
        self.__appendValue(value)
        return self.__getAverage()
