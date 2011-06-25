# -*- coding: utf-8 -*-
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

from libavg import avg

from helper import *

from math import *

g_Player = avg.Player.get()

class ContactData:

    def __init__(self, listenerid):
        self.listenerid = listenerid


class Recognizer(object):

    def __init__(self, node, eventSource, maxContacts, initialEvent):
        self._node = node
        self.__eventSource = eventSource
       
        self.__setEventHandler() 
        self.__isEnabled = True
        self.__maxContacts = maxContacts
        self._contacts = {}
        self.__dirty = False
        if initialEvent:
            self._onDown(initialEvent)

    def enable(self, isEnabled):
        if isEnabled != self.__isEnabled:
            self.__isEnabled = isEnabled
            if isEnabled:
                self.__setEventHandler()
            else:
                self._abort()
                self._node.disconnectEventHandler(self)

    def _onDown(self, event):
        if self.__maxContacts == None or len(self._contacts) < self.__maxContacts:
            listenerid = event.contact.connectListener(self._onMotion, self._onUp)
            self._contacts[event.contact] = ContactData(listenerid)
            if len(self._contacts) == 1:
                self.__frameHandlerID = g_Player.setOnFrameHandler(self.__onFrame)
            self.__dirty = True
            return self._handleDown(event)

    def _onMotion(self, event):
        self.__dirty = True
        self._handleMove(event)

    def _onUp(self, event):
        self.__dirty = True
        listenerid = self._contacts[event.contact].listenerid
        self._handleUp(event)
        del self._contacts[event.contact]
        event.contact.disconnectListener(listenerid)
        if self._contacts == {}:
            g_Player.clearInterval(self.__frameHandlerID)

    def _abort(self):
        for contact, contactData in self._contacts.iteritems():
            contact.disconnectListener(contactData.listenerid)
        self._contacts = {}
        g_Player.clearInterval(self.__frameHandlerID)

    def _handleDown(self, event):
        pass

    def _handleMove(self, event):
        pass

    def _handleUp(self, event):
        pass

    def _handleChange(self):
        pass

    def __onFrame(self):
        if self.__dirty:
            self._handleChange()
            self.__dirty = False

    def __setEventHandler(self):
        self._node.connectEventHandler(avg.CURSORDOWN, self.__eventSource, self, 
                self._onDown)


class DragRecognizer(Recognizer):

    def __init__(self, node, eventSource=avg.TOUCH | avg.MOUSE, startHandler=None,
            moveHandler=None, upHandler=None, stopHandler=None, initialEvent=None,
            friction=-1):
        self.__startHandler = optionalCallback(startHandler, lambda event:None)
        self.__moveHandler = optionalCallback(moveHandler, lambda event,offset:None)
        self.__stopHandler = optionalCallback(stopHandler, lambda:None)
        self.__upHandler = optionalCallback(upHandler, lambda event,offset:None)
        self.__friction = friction
        self.__inertiaHandlerID = None
        Recognizer.__init__(self, node, eventSource, 1, initialEvent)

    def abortInertia(self):
        if self.__inertiaHandlerID:
            self.__stop()

    def _handleDown(self, event):
        if self.__inertiaHandlerID:
            self.__stopHandler()
            g_Player.clearInterval(self.__inertiaHandlerID)
        self.__dragStartPos = event.pos
        self.__dragStartMotionVec = event.contact.motionvec
        self.__startHandler(event)
        self.__speed = avg.Point2D(0,0)
        self.__frameHandlerID = g_Player.setOnFrameHandler(self.__onFrame)

    def _handleMove(self, event):
        # TODO: Offset is in the global coordinate system. We should really be using
        # the coordinate system we're in at the moment the drag starts. 
        self.__moveHandler(event, event.contact.motionvec-self.__dragStartMotionVec)
        self.__speed += 0.1*event.speed

    def __onFrame(self):
        self.__speed *= 0.9

    def _handleUp(self, event):
        self.__upHandler(event, event.contact.motionvec)
        g_Player.clearInterval(self.__frameHandlerID)
        if self.__friction != -1:
            self.__inertiaHandlerID = g_Player.setOnFrameHandler(self.__handleInertia)
            self.__speed += 0.1*event.speed
            self.__offset = event.contact.motionvec
        else:
            self.__stopHandler()

    def __handleInertia(self):
        norm = self.__speed.getNorm()
        if norm-self.__friction > 0:
            direction = self.__speed.getNormalized()
            self.__speed = direction*(norm-self.__friction)
            self.__offset += self.__speed * g_Player.getFrameDuration()
            if self.__moveHandler:
                self.__moveHandler(None, self.__offset)
        else:
            self.__stop()

    def __stop(self):
        self.__speed = avg.Point2D(0,0)
        self.__stopHandler()
        g_Player.clearInterval(self.__inertiaHandlerID)
        self.__inertiaHandlerID = None


class HoldRecognizer(Recognizer):

    # States
    UP = 0          # No action pending
    DOWN = 1        # Down, but <  holdDelay
    HOLDING = 2     # Down, > holdDelay, < activateDelay
    ACTIVE = 3      # > activateDelay

    def __init__(self, node, holdDelay, activateDelay, eventSource=avg.TOUCH | avg.MOUSE, 
            startHandler=None, holdHandler=None, activateHandler=None, stopHandler=None,
            initialEvent=None):
        self.__startHandler = optionalCallback(startHandler, lambda pos:None)
        self.__holdHandler = optionalCallback(holdHandler, lambda t:None)
        self.__activateHandler = optionalCallback(activateHandler, lambda:None)
        self.__stopHandler = optionalCallback(stopHandler, lambda:None)

        self.__holdDelay = holdDelay
        self.__activateDelay = activateDelay

        self.__frameHandlerID = None
        self.__state = HoldRecognizer.UP

        self.__relTime = 0
        self.__lastEvent = None
        Recognizer.__init__(self, node, eventSource, 1, initialEvent)

    def abort(self):
        self._onUp(self.__lastEvent)

    def getRelTime(self):
        return self.__relTime

    def getLastEvent(self):
        return self.__lastEvent

    def _handleDown(self, event):
        self.__startPos = event.pos
        self.__startTime = g_Player.getFrameTime()
        self.__lastEvent = event
        self.__changeState(HoldRecognizer.DOWN)
        self.__frameHandlerID = g_Player.setOnFrameHandler(self.__onFrame)

    def _handleMove(self, event):
        self.__lastEvent = event
        if event.contact.distancefromstart > 8:
            self.__startPos = event.pos
            self.__startTime = g_Player.getFrameTime()
            if self.__state != HoldRecognizer.DOWN:
                self.__stopHandler()
                self.__changeState(HoldRecognizer.DOWN)

    def __onFrame(self):
        self.__relTime = g_Player.getFrameTime() - self.__startTime
        if self.__state == HoldRecognizer.DOWN:
            if self.__relTime > self.__holdDelay:
                holdOk = self.__startHandler(self.__startPos)
                if holdOk:
                    self.__changeState(HoldRecognizer.HOLDING)
        if self.__state == HoldRecognizer.HOLDING:
            if self.__relTime > self.__activateDelay:
                self.__changeState(HoldRecognizer.ACTIVE)
                self.__activateHandler()
            else:
                self.__holdHandler(float(self.__relTime-self.__holdDelay)/
                        (self.__activateDelay-self.__holdDelay))

    def _handleUp(self, event):
        g_Player.clearInterval(self.__frameHandlerID)
        self.__frameHandlerID = None
        self.__lastEvent = None
        if self.__state != HoldRecognizer.DOWN:
            self.__stopHandler()
        self.__changeState(HoldRecognizer.UP)
        self.__relTime = 0

    def __changeState(self, newState):
#        print self, ": ", self.__state, " --> ", newState
        self.__state = newState


class TapRecognizer(Recognizer):

    UP = 0
    POSSIBLE = 1

    MAX_DISTANCE_IN_MM = 5

    def __init__(self, node, eventSource=avg.TOUCH | avg.MOUSE, startHandler=None,
            tapHandler=None, failHandler=None, initialEvent=None):
        self.__startHandler = optionalCallback(startHandler, lambda:None)
        self.__tapHandler = optionalCallback(tapHandler, lambda:None)
        self.__failHandler = optionalCallback(failHandler, lambda:None)
        self.__state = TapRecognizer.UP
        self.__maxDistance = TapRecognizer.MAX_DISTANCE_IN_MM*g_Player.getPixelsPerMM()
        Recognizer.__init__(self, node, eventSource, 1, initialEvent)

    def _handleDown(self, event):
        self.__state = TapRecognizer.POSSIBLE
        self.__startHandler()
    
    def _handleMove(self, event):
        if event.contact.distancefromstart > self.__maxDistance:
            self._abort()
            self.__fail(event)

    def _handleUp(self, event):
        if event.contact.distancefromstart > self.__maxDistance:
            self.__fail(event)
        else:
            self.__recognize()

    def __recognize(self):
        self.__tapHandler()
   
    def __fail(self, event):
        self.__failHandler()
        self.__state = TapRecognizer.UP


class Mat3x3:
    # Internal class. Will be removed again.

    def __init__(self, row0=(1,0,0), row1=(0,1,0), row2=(0,0,1)):
        self.m = [row0, row1, row2]

    @classmethod
    def translate(cls, t):
        return Mat3x3([1, 0, t[0]],
                      [0, 1, t[1]])

    @classmethod
    def rotate(cls, a):
        return Mat3x3([cos(a), -sin(a), 0],
                      [sin(a), cos(a), 0])

    @classmethod
    def scale(cls, s):
        return Mat3x3([s, 0, 0],
                      [0, s, 0])

    def __str__(self):
        return self.m.__str__()

    def applyVec(self, v):
        m = self.m
        v1 = []
        for i in range(3):
            v1.append(m[i][0]*v[0] + m[i][1]*v[1] + m[i][2]*v[2])
        return v1

    def applyMat(self, m1):
        m0 = self.m
        result = Mat3x3() 
        for i in range(3):
            v = []
            for j in range(3):
                v.append(m0[i][0]*m1.m[0][j] + m0[i][1]*m1.m[1][j] + m0[i][2]*m1.m[2][j])
            result.m[i] = v
        return result

    def det(self):
        m = self.m
        return float( m[0][0] * (m[2][2]*m[1][1]-m[2][1]*m[1][2])
                     -m[1][0] * (m[2][2]*m[0][1]-m[2][1]*m[0][2])
                     +m[2][0] * (m[1][2]*m[0][1]-m[1][1]*m[0][2]))

    def scalarMult(self, s):
        m = self.m
        result = Mat3x3()
        for i in range(3):
            v = []
            for j in range(3):
                v.append(m[i][j]*s)
            result.m[i] = v
        return result

    def inverse(self):
        m = self.m
        temp = Mat3x3([  m[2][2]*m[1][1]-m[2][1]*m[1][2],  -(m[2][2]*m[0][1]-m[2][1]*m[0][2]),   m[1][2]*m[0][1]-m[1][1]*m[0][2] ],
                      [-(m[2][2]*m[1][0]-m[2][0]*m[1][2]),   m[2][2]*m[0][0]-m[2][0]*m[0][2] , -(m[1][2]*m[0][0]-m[1][0]*m[0][2])],
                      [  m[2][1]*m[1][0]-m[2][0]*m[1][1],  -(m[2][1]*m[0][0]-m[2][0]*m[0][1]),   m[1][1]*m[0][0]-m[1][0]*m[0][1] ])
        return temp.scalarMult(1/self.det())


class TransformRecognizer(Recognizer):

    def __init__(self, node, eventSource=avg.TOUCH, startHandler=None,
            moveHandler=None, upHandler=None, stopHandler=None, initialEvent=None,
            friction=-1):
        self.__startHandler = optionalCallback(startHandler, lambda:None)
        self.__moveHandler = optionalCallback(moveHandler, lambda trans, rot, scale:None)
        self.__stopHandler = optionalCallback(stopHandler, lambda:None)
        self.__upHandler = optionalCallback(upHandler, lambda offset:None)
        self.__friction = friction
        self.__baseTransform = Mat3x3()
        self.__transform = Mat3x3()
        Recognizer.__init__(self, node, eventSource, None, initialEvent)

    def _handleDown(self, event):
        numContacts = len(self._contacts)
        self.__newPhase()
        if numContacts == 1:
            self.__startHandler()

    def _handleUp(self, event):
        numContacts = len(self._contacts)
        if numContacts == 1:
            self.__newPhase()
            totalTransform = self.__baseTransform.applyMat(self.__transform)
            self.__upHandler(totalTransform)
        else:
            # TODO: Calculate motion before up
            self.__newPhase()

    def _handleChange(self):
        numContacts = len(self._contacts)
        if numContacts == 1:
            contact = self._contacts.keys()[0]
            contactInfo = self._contacts[contact]
            self.__transform = Mat3x3.translate(
                    contact.events[-1].pos - contactInfo.startPos)
        elif numContacts == 2:
            contact0, contact0Info = self._contacts.items()[0]
            contact1, contact1Info = self._contacts.items()[1]
            start0 = contact0Info.startPos
            cur0 = contact0.events[-1].pos
            start1 = contact1Info.startPos
            cur1 = contact1.events[-1].pos
            self.__transform = self.__calcAffineTransform(start0, cur0, start1, cur1,
                    self.__start2)
        totalTransform = self.__baseTransform.applyMat(self.__transform)
        self.__moveHandler(totalTransform)

    def __calcAffineTransform(self, start0, cur0, start1, cur1, start2):
        # Algorithm from http://mike.teczno.com/notes/two-fingers.html

        def solveLinearEquations(x1, y1, z1,  x2, y2, z2,  x3, y3, z3):
            # Solves a system of linear equations.
            #
            #   z1 = (a * x1) + (b * y1) + c
            #   z2 = (a * x2) + (b * y2) + c
            #   z3 = (a * x3) + (b * y3) + c
            a = ((((z2 - z3) * (y1 - y2)) - ((z1 - z2) * (y2 - y3)))
                     / (((x2 - x3) * (y1 - y2)) - ((x1 - x2) * (y2 - y3))))
            b = ((((z2 - z3) * (x1 - x2)) - ((z1 - z2) * (x2 - x3)))
                     / (((y2 - y3) * (x1 - x2)) - ((y1 - y2) * (x2 - x3))))
            c = z1 - (x1 * a) - (y1 * b)
            return [a, b, c]

        cur2 = self.__findThirdPoint(cur0, cur1)
        xt = solveLinearEquations(
                start0.x, start0.y, cur0.x,
                start1.x, start1.y, cur1.x,
                start2.x, start2.y, cur2.x)
        yt = solveLinearEquations(
                start0.x, start0.y, cur0.y,
                start1.x, start1.y, cur1.y,
                start2.x, start2.y, cur2.y)
        return Mat3x3(xt, yt)

    def __findThirdPoint(self, f1, f2):
        diff = f2 - f1
        diffAngle = atan2(diff.y, diff.x)
        diffLen = diff.getNorm()

        vec1 = avg.Point2D.fromPolar(diffAngle+pi/4, diffLen)
        vec2 = avg.Point2D.fromPolar(diffAngle+3*pi/4, diffLen)
        slope1 = vec1.y/vec1.x
        intercept1 = f1.y - slope1*f1.x
        slope2 = vec2.y/vec2.x
        intercept2 = f2.y - slope2*f2.x
        x = (intercept2-intercept1) / (slope1-slope2)
        return avg.Point2D(x, slope1*x + intercept1)


    def __newPhase(self):
        self.__baseTransform = self.__baseTransform.applyMat(self.__transform)
        self.__transform = Mat3x3()

        for contact, contactInfo in self._contacts.iteritems():
            contactInfo.startPos = contact.events[-1].pos

        numContacts = len(self._contacts)
        if numContacts == 1:
            self.__baseTransform = Mat3x3()
            self.__transform = Mat3x3()
        elif numContacts == 2:
            contact0, contact0Info = self._contacts.items()[0]
            contact1, contact1Info = self._contacts.items()[1]
            start0 = contact0Info.startPos
            start1 = contact1Info.startPos
            self.__start2 = self.__findThirdPoint(start0, start1)
        
