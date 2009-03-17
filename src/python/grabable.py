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

import math

from libavg import avg, Point2D
from clusteredEventList import ClusteredEventList
from mathutil import solveEquationMatrix, EquationSingular, EquationNotSolvable
from mathutil import getAngle, getDistance, getScaledDim


NUM_SPEEDS = 5

g_player = avg.Player.get()

class Grabable:
    """Helper for multitouch object movement.
    Grabable will add the well-known multitouch gestures
    to a node, i.e. moving with one finger, rotating,
    resizing and moving with 2 fingers.
    It also works with many more cursors/fingers, clustering
    them.
    """
    def __init__(self,
            node,
            source = avg.TOUCH | avg.MOUSE,
            maxSize = None, minSize = None,
            onResize = lambda: None,
            onDrop = lambda: None,
            onStop = lambda: None,
            onAction = lambda: None,
            onMotion = lambda pos, size, angle, pivot: None,
            onDragStart = lambda: None,
            initialDown = None,
            inertia = 0.95,
            torque = 0.95,
            moveNode = True):

        global g_player
        self.__inertia = inertia
        self.__torque = torque
        self.__node = node
        self.__minSize = minSize
        self.__maxSize = maxSize
        self.__moveNode = moveNode
        self.__callback = {
                'onResize': onResize,
                'onDrop': onDrop,
                'onStop': onStop,
                'onAction': onAction,
                'onMotion': onMotion,
                'onDragStart': onDragStart,
                }

        self.__stopInertia()
        #self.__oldAngle = 0.0
        self.__lastFixPoint = None
        self.__lastNumFingers = 0
        self.__onFrameHandler = g_player.setOnFrameHandler(self.__onFrame)

        self.__eventList = ClusteredEventList (self.__node,
                source = source,
                onMotion = self.__onMotion,
                onUp = self.__onUp,
                onDown = self.__onDown,
                resetMotion = self.__resetMotion
                )
        if initialDown:
            self.__eventList.handleInitialDown (initialDown)
        self.__lastPos = None
        self.__stopped = True

    def isDragging(self):
        return len(self.__eventList) > 0
    
    def getSpeed(self):
        return self.__speed
    
    def getAngle(self):
        return self.__angle
    
    def delete(self):
        global g_player
        g_player.clearInterval(self.__onFrameHandler)
        self.__eventList.delete()
        self.__node = None

    def __onFrame(self):
        global g_player
        if len (self.__eventList) == 0:
            # inertia
            if self.__speed.getNorm() < 1:
                if not self.__stopped:
                    self.__stopped = True
                    self.__callback['onStop']()
                    self.__stopInertia()
                    return
            if self.__moveNode:
                self.__node.pos += self.__speed
                self.__node.angle += self.__rotSpeed
            self.__speed *= self.__inertia
            self.__rotSpeed *= self.__torque
        else:
            # save current speed for inertia
            pos = self.__getAbsCenter()
            angle = self.__node.angle
            if self.__lastPos:
                speed = pos - self.__lastPos
                rotSpeed = angle - self.__lastAngle
                # use minimal angle to avoid extreme rotation:
                if rotSpeed < -math.pi/2:
                    rotSpeed += math.pi*2
                if rotSpeed > math.pi/2:
                    rotSpeed -= math.pi*2
                self.__speeds = (self.__speeds + [speed])[-NUM_SPEEDS:]
                self.__rotSpeeds = (self.__rotSpeeds + [rotSpeed])[-NUM_SPEEDS:]
            self.__lastPos = pos
            self.__lastAngle = angle

    def __stopInertia (self):
        self.__speed = Point2D(0,0)
        self.__rotSpeed = 0
        self.__speeds = [Point2D(0,0)]
        self.__rotSpeeds = [0.0]
        self.__lastPos = None

    def __onDown(self):
        def __gotoTopLayer ():
            parent = self.__node.getParent()
            numChildren = parent.getNumChildren()
            parent.reorderChild(self.__node, numChildren - 1)

        self.__stopped = False
        self.__stopInertia()
        self.__reshape()
        if len(self.__eventList) == 1: # first finger down
            __gotoTopLayer()
            self.__callback['onDragStart']()
        self.__callback['onAction']()

    def __onMotion(self):
        self.__reshape()
        self.__callback['onAction']()

    def __drop(self):
        self.__speed = sum(self.__speeds, Point2D(0,0)) / len(self.__speeds)
        self.__rotSpeed = sum(self.__rotSpeeds, 0.0) / len(self.__rotSpeeds)
        self.__callback['onDrop']()

    def __onUp(self):
        if len(self.__eventList) == 0:
            self.__drop()
        self.__callback['onAction']()

    def __reshape (self):
        def applyAffineTransformation (parameters, p):
            """apply transformation to point:
                         M:       v:
              P' = P * (m  -n) + (v0)
                       (n   m)   (v1) """
            m, n, v0, v1 = parameters
            nx = p[0] * m - p[1] * n + v0
            ny = p[0] * n + p[1] * m + v1
            return Point2D(nx, ny)

        clusters = self.__eventList.getCursors()

        if len(clusters) == 1:
            cursor = self.__eventList.getCursors()[0]

            pos = self.__oldpos + cursor.getDelta()
            size = self.__node.size
            angle = self.__node.angle
            pivot = self.__node.pivot

        elif len(clusters) == 2:
            # calculate an affine transformation which describes cluster movement
            # TODO: explain used maths
            cursor1, cursor2 = clusters
            equationMatrix = (
                    ((cursor1.getStartPos().x, -cursor1.getStartPos().y, 1, 0), cursor1.getPos().x),
                    ((cursor1.getStartPos().y,  cursor1.getStartPos().x, 0, 1), cursor1.getPos().y),
                    ((cursor2.getStartPos().x, -cursor2.getStartPos().y, 1, 0), cursor2.getPos().x),
                    ((cursor2.getStartPos().y,  cursor2.getStartPos().x, 0, 1), cursor2.getPos().y),
                    )
            try:
                param = solveEquationMatrix (equationMatrix)
            except (EquationSingular, EquationNotSolvable):
                print "WARNING: Grabable: cannot solve equation, skipping movement"
                return False

            absTouchCenter = cursor2.getPos() + (cursor1.getPos() - cursor2.getPos()) / 2

            n_tl = applyAffineTransformation (param, self.o_tl)
            n_bl = applyAffineTransformation (param, self.o_bl)
            n_tr = applyAffineTransformation (param, self.o_tr)

            pos = n_tl
            size = Point2D(getDistance (n_tl, n_tr), getDistance (n_tl, n_bl))
            angle = getAngle(n_tl, n_tr) # pivot for this rotation is 0,0 (rel. to node)

            def getRelPos(pos, nodePos, angle, pivot):
                return (pos - nodePos).getRotated(-angle, pivot)

            def getAbsPos(pos, nodePos, angle, pivot):
                return pos.getRotated(angle, pivot) + nodePos

            # get middle of touches relative to the node
            relTouchCenter = getRelPos(absTouchCenter, n_tl, angle, Point2D(0,0))

            def getOffsetForMovedPivot(oldPivot, newPivot, angle):
                oldPos = Point2D(0,0).getRotated(angle, oldPivot)
                newPos = Point2D(0,0).getRotated(angle, newPivot)
                return oldPos - newPos

            pos += getOffsetForMovedPivot(
                    oldPivot = Point2D(0,0),
                    newPivot = relTouchCenter,
                    angle = angle)
            pivot = relTouchCenter

            def scaleMinMax(pos, size, angle, pivot):
                # the absolute position of the pivot should change when
                # resizing, so we track and revert absolute pivot movement
                oldAbsPivot = getAbsPos(pivot, pos, angle, pivot)

                newSize = getScaledDim (size,
                        max = self.__maxSize,
                        min = self.__minSize)

                ratio = newSize.x / size.x
                newPivot = pivot * ratio

                pos += getOffsetForMovedPivot(
                        oldPivot = pivot,
                        newPivot = newPivot,
                        angle = angle)
                pivot = newPivot

                size = newSize

                newAbsPivot = getAbsPos(pivot, pos, angle, pivot)

                # move node to revert absolute pivot position
                pos -= (newAbsPivot - oldAbsPivot)

                return (pos, size, angle, pivot)

            (pos, size, angle, pivot) = scaleMinMax(pos, size, angle, pivot)

        else: # more than 2 clusters
            assert False

        if self.__moveNode:
            self.__node.angle = angle
            self.__node.size = size
            self.__node.pos = pos
            self.__node.pivot = pivot
            self.__callback['onResize']()

        self.__callback['onMotion'](pos, size, angle, pivot)



    def __getAbsCenter(self):
        return self.__node.getAbsPos (self.__node.size/2)

    def __resetMotion (self):
        """ sets a new movement start point """
        # store absolute position at the beginning of the movement
        # for 2-finger-movements:
        self.o_tl = self.__node.getAbsPos ((0, 0))
        self.o_bl = self.__node.getAbsPos ((0, self.__node.height))
        self.o_tr = self.__node.getAbsPos ((self.__node.width, 0))

        # for 1-finger-movements:
        self.__oldpos = self.__node.pos

        self.__lastFixPoint = None

