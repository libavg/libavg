# -*- coding: utf-8 -*-
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

from libavg import avg, statemachine, player, filter

import weakref

import math

class Recognizer(avg.Publisher):

    POSSIBLE = avg.Publisher.genMessageID()
    DETECTED = avg.Publisher.genMessageID()
    FAILED = avg.Publisher.genMessageID()
    MOTION = avg.Publisher.genMessageID()
    UP = avg.Publisher.genMessageID()
    END = avg.Publisher.genMessageID()

    def __init__(self, node, isContinuous, maxContacts, initialEvent,
            possibleHandler=None, failHandler=None, detectedHandler=None,
            endHandler=None):
        super(Recognizer, self).__init__()

        if node:
            self.__node = weakref.ref(node)
        else:
            self.__node = None
        self.__isContinuous = isContinuous
        self.__maxContacts = maxContacts

        self.__downHandlerID = None
        self.__moveHandlerID = {}
        self.__upHandlerID = {}

        self.__setEventHandler() 
        self.__isEnabled = True
        self._contacts = set() 
        self.__dirty = False

        self.publish(Recognizer.POSSIBLE)
        self.publish(Recognizer.DETECTED)
        self.publish(Recognizer.FAILED)
        self.publish(Recognizer.END)
        self.__stateMachine = statemachine.StateMachine(str(type(self)), "IDLE")
        if self.__isContinuous:
            self.publish(Recognizer.MOTION)
            self.publish(Recognizer.UP)
            self.__stateMachine.addState("IDLE", ("POSSIBLE", "RUNNING"))
            self.__stateMachine.addState("POSSIBLE", ("IDLE", "RUNNING"))
            self.__stateMachine.addState("RUNNING", ("IDLE",))
        else:
            self.__stateMachine.addState("IDLE", ("POSSIBLE",))
            self.__stateMachine.addState("POSSIBLE", ("IDLE",))

        self.subscribe(Recognizer.POSSIBLE, possibleHandler)
        self.subscribe(Recognizer.FAILED, failHandler)
        self.subscribe(Recognizer.DETECTED, detectedHandler)
        self.subscribe(Recognizer.END, endHandler)
        # self.__stateMachine.traceChanges(True)
        self.__frameHandlerID = None

        if initialEvent:
            self.__onDown(initialEvent)

    @property
    def contacts(self):
        return list(self._contacts)

    def abort(self):
        if self.__isEnabled:
            self.__abort()
            self.__setEventHandler()

    def enable(self, isEnabled):
        if bool(isEnabled) != self.__isEnabled:
            self.__isEnabled = bool(isEnabled)
            if isEnabled:
                self.__setEventHandler()
            else:
                self.__abort()

    def isEnabled(self):
        return self.__isEnabled

    def getState(self):
        return self.__stateMachine.state

    def _setPossible(self, event):
        self.__stateMachine.changeState("POSSIBLE")
        self.notifySubscribers(Recognizer.POSSIBLE, [])

    def _setFail(self, event):
        assert(self.__stateMachine.state != "RUNNING")
        if self.__stateMachine.state != "IDLE":
            self.__stateMachine.changeState("IDLE")
        self._disconnectContacts()
        self.notifySubscribers(Recognizer.FAILED, [])

    def _setDetected(self, event):
        if self.__isContinuous:
            self.__stateMachine.changeState("RUNNING")
        else:
            self.__stateMachine.changeState("IDLE")
        self.notifySubscribers(Recognizer.DETECTED, [])

    def _setEnd(self, event):
        assert(self.__stateMachine.state != "POSSIBLE")
        if self.__stateMachine.state != "IDLE":
            self.__stateMachine.changeState("IDLE")
        self.notifySubscribers(Recognizer.END, [])

    def __onDown(self, event):
        nodeGone = self._handleNodeGone()
        if event.contact and not(nodeGone):
            if (self.__maxContacts == None or len(self._contacts) <
                    self.__maxContacts):
                self.__moveHandlerID[event.contact] = event.contact.subscribe(
                        avg.Contact.CURSOR_MOTION, self.__onMotion)
                self.__upHandlerID[event.contact] = event.contact.subscribe(
                        avg.Contact.CURSOR_UP, self.__onUp)
                self._contacts.add(event.contact)
                if len(self._contacts) == 1:
                    self.__frameHandlerID = player.subscribe(player.ON_FRAME, 
                            self._onFrame)
                self.__dirty = True
                return self._handleDown(event)

    def __onMotion(self, event):
        nodeGone = self._handleNodeGone()
        if event.contact and not(nodeGone):
            self.__dirty = True
            self._handleMove(event)

    def __onUp(self, event):
        nodeGone = self._handleNodeGone()
        if event.contact and not(nodeGone):
            self.__dirty = True
            self._contacts.remove(event.contact)
            if len(self._contacts) == 0:
                player.unsubscribe(player.ON_FRAME, self.__frameHandlerID)
                self.__frameHandlerID = None
            self._handleUp(event)

    def __abort(self):
        if self.__stateMachine.state != "IDLE":
            self.__stateMachine.changeState("IDLE")
        if len(self._contacts) != 0:
            self._disconnectContacts()
        if self.__node and self.__node():
            self.__node().unsubscribe(avg.Node.CURSOR_DOWN, self.__downHandlerID)
            self.__downHandlerID = None

    def _disconnectContacts(self):
        for contact in self._contacts:
            contact.unsubscribe(avg.Contact.CURSOR_MOTION, self.__moveHandlerID[contact])
            contact.unsubscribe(avg.Contact.CURSOR_UP, self.__upHandlerID[contact])
        self.__moveHandlerID = {}
        self.__upHandlerID = {}
        self._contacts = set()
        if self.__frameHandlerID:
            player.unsubscribe(player.ON_FRAME, self.__frameHandlerID)
            self.__frameHandlerID = None

    def _handleDown(self, event):
        pass

    def _handleMove(self, event):
        pass

    def _handleUp(self, event):
        pass

    def _handleChange(self):
        pass

    def _onFrame(self):
        nodeGone = self._handleNodeGone()
        if not(nodeGone) and self.__dirty:
            self._handleChange()
            self.__dirty = False

    def _handleNodeGone(self):
        if self.__node and not(self.__node()):
            self.enable(False)
            return True
        else:
            return False

    def __setEventHandler(self):
        if self.__node and self.__node():
            self.__downHandlerID = self.__node().subscribe(
                    avg.Node.CURSOR_DOWN, self.__onDown)


class TapRecognizer(Recognizer):

    MAX_TAP_DIST = None 

    def __init__(self, node, maxTime=None, maxDist=None, initialEvent=None,
            possibleHandler=None, failHandler=None, detectedHandler=None):
        self.__maxTime = maxTime
        if maxDist == None:
            maxDist = TapRecognizer.MAX_TAP_DIST
        self.__maxDist = maxDist
        super(TapRecognizer, self).__init__(node, False, 1, initialEvent,
                possibleHandler, failHandler, detectedHandler)

    def _handleDown(self, event):
        self._setPossible(event)
        self.__startTime = player.getFrameTime()

    def _handleMove(self, event):
        if self.getState() != "IDLE": 
            if (event.contact.distancefromstart > 
                    self.__maxDist*player.getPixelsPerMM()):
                self._setFail(event)

    def _handleUp(self, event):
        if self.getState() == "POSSIBLE":
            if (event.contact.distancefromstart > 
                    self.__maxDist*player.getPixelsPerMM()):
                self._setFail(event)
            else:
                self._setDetected(event)

    def _onFrame(self):
        downTime = player.getFrameTime() - self.__startTime
        if self.getState() == "POSSIBLE":
            if self.__maxTime and downTime > self.__maxTime:
                self._setFail(None)
        super(TapRecognizer, self)._onFrame()


class DoubletapRecognizer(Recognizer):

    MAX_DOUBLETAP_TIME = None

    def __init__(self, node, maxTime=None, maxDist=None, initialEvent=None,
            possibleHandler=None, failHandler=None, detectedHandler=None):
        if maxTime == None:
            maxTime = DoubletapRecognizer.MAX_DOUBLETAP_TIME
        self.__maxTime = maxTime
        if maxDist == None:
            maxDist = TapRecognizer.MAX_TAP_DIST
        self.__maxDist = maxDist

        self.__stateMachine = statemachine.StateMachine("DoubletapRecognizer", "IDLE")
        self.__stateMachine.addState("IDLE", ("DOWN1",), enterFunc=self.__enterIdle)
        self.__stateMachine.addState("DOWN1", ("UP1", "IDLE"))
        self.__stateMachine.addState("UP1", ("DOWN2", "IDLE"))
        self.__stateMachine.addState("DOWN2", ("IDLE",))
        #self.__stateMachine.traceChanges(True)
        self.__frameHandlerID = None
        super(DoubletapRecognizer, self).__init__(node, False, 1, 
                initialEvent, possibleHandler, failHandler, detectedHandler)

    def abort(self):
        if self.__stateMachine.state != "IDLE":
            self.__stateMachine.changeState("IDLE")
        super(DoubletapRecognizer, self).abort()

    def enable(self, isEnabled):
        if self.__stateMachine.state != "IDLE":
            self.__stateMachine.changeState("IDLE")
        super(DoubletapRecognizer, self).enable(isEnabled)

    def _handleDown(self, event):
        self.__startTime = player.getFrameTime()
        if self.__stateMachine.state == "IDLE":
            self.__frameHandlerID = player.subscribe(player.ON_FRAME, self.__onFrame)
            self.__stateMachine.changeState("DOWN1")
            self.__startPos = event.pos
            self._setPossible(event)
        elif self.__stateMachine.state == "UP1":
            if ((event.pos - self.__startPos).getNorm() > 
                    self.__maxDist*player.getPixelsPerMM()):
                self.__stateMachine.changeState("IDLE")
                self._setFail(event)
            else:
                self.__stateMachine.changeState("DOWN2")
        else:
            assert(False), self.__stateMachine.state

    def _handleMove(self, event):
        if self.__stateMachine.state != "IDLE": 
            if ((event.pos - self.__startPos).getNorm() > 
                    self.__maxDist*player.getPixelsPerMM()):
                self.__stateMachine.changeState("IDLE")
                self._setFail(event)

    def _handleUp(self, event):
        if self.__stateMachine.state == "DOWN1":
            self.__startTime = player.getFrameTime()
            self.__stateMachine.changeState("UP1")
        elif self.__stateMachine.state == "DOWN2":
            if ((event.pos - self.__startPos).getNorm() >
                    self.__maxDist*player.getPixelsPerMM()):
                self._setFail(event)
            else:
                self._setDetected(event)
            self.__stateMachine.changeState("IDLE")
        elif self.__stateMachine.state == "IDLE":
            pass
        else:
            assert(False), self.__stateMachine.state

    def __onFrame(self):
        downTime = player.getFrameTime() - self.__startTime
        if downTime > self.__maxTime:
            self._setFail(None)
            self.__stateMachine.changeState("IDLE")

    def __enterIdle(self):
        player.unsubscribe(player.ON_FRAME, self.__frameHandlerID)


class SwipeRecognizer(Recognizer):

    LEFT = 1
    RIGHT = 2
    UP = 3
    DOWN = 4

    SWIPE_DIRECTION_TOLERANCE = math.pi/8
    MIN_SWIPE_DIST = 50
    MAX_SWIPE_CONTACT_DIST = 100

    def __init__(self, node, direction, numContacts=1, initialEvent=None, 
            directionTolerance=SWIPE_DIRECTION_TOLERANCE, minDist=MIN_SWIPE_DIST, 
            maxContactDist=MAX_SWIPE_CONTACT_DIST,
            possibleHandler=None, failHandler=None, detectedHandler=None):

        self.__numContacts = numContacts
        self.__angleWanted = self.__angleFromDirection(direction)
        self.__directionTolerance = directionTolerance
        self.__minDist = minDist*player.getPixelsPerMM()
        self.__maxInterContactDist = maxContactDist*player.getPixelsPerMM()
        super(SwipeRecognizer, self).__init__(node, False, numContacts, 
                initialEvent, possibleHandler=possibleHandler, failHandler=failHandler, 
                detectedHandler=detectedHandler)

    def _handleDown(self, event):
        if len(self._contacts) == 1:
            self.__startPos = event.pos
        else:
            if (event.pos-self.__startPos).getNorm() > self.__maxInterContactDist:
                self._setFail(event)
                return
        if len(self._contacts) == self.__numContacts:
            self._setPossible(event)

    def _handleMove(self, event):
        pass

    def _handleUp(self, event):
        if self.getState() == "POSSIBLE":
            if (event.contact.distancefromstart < self.__minDist or
                    not(self.__isValidAngle(event.contact.motionangle))):
                self._setFail(event)
            elif len(self._contacts) == 0:
                self._setDetected(event)

    def __angleFromDirection(self, direction):
        if direction == SwipeRecognizer.RIGHT:
            return 0
        elif direction == SwipeRecognizer.DOWN:
            return math.pi/2
        elif direction == SwipeRecognizer.LEFT:
            return math.pi
        elif direction == SwipeRecognizer.UP:
            return 3*math.pi/2
        else:
            raise RuntimeError("%s is not a valid direction."%direction)

    def __isValidAngle(self, angle):
        if angle < 0:
            angle += 2*math.pi
        minAngle = self.__angleWanted - self.__directionTolerance
        maxAngle = self.__angleWanted + self.__directionTolerance
        if minAngle >= 0:
            return angle > minAngle and angle < maxAngle
        else:
            # Valid range spans 0
            return angle > minAngle+2*math.pi or angle < maxAngle


class HoldRecognizer(Recognizer):

    HOLD_DELAY = None

    def __init__(self, node, delay=None, maxDist=None, initialEvent=None,
            possibleHandler=None, failHandler=None,
            detectedHandler=None, stopHandler=None):
        if delay == None:
            delay = HoldRecognizer.HOLD_DELAY
        self.__delay = delay
        if maxDist == None:
            maxDist = TapRecognizer.MAX_TAP_DIST
        self.__maxDist = maxDist

        self.__lastEvent = None
        super(HoldRecognizer, self).__init__(node, True, 1, initialEvent,
                possibleHandler, failHandler, detectedHandler, stopHandler)

    def _handleDown(self, event):
        self.__lastEvent = event
        self._setPossible(event)
        self.__startTime = player.getFrameTime()

    def _handleMove(self, event):
        self.__lastEvent = event
        if self.getState() == "POSSIBLE": 
            if (event.contact.distancefromstart > 
                    self.__maxDist*player.getPixelsPerMM()):
                self._setFail(event)

    def _handleUp(self, event):
        self.__lastEvent = event
        if self.getState() == "POSSIBLE":
            self._setFail(event)
        elif self.getState() == "RUNNING":
            self._setEnd(event)

    def _onFrame(self):
        downTime = player.getFrameTime() - self.__startTime
        if self.getState() == "POSSIBLE":
            if downTime > self.__delay:
                self._setDetected(self.__lastEvent)
        super(HoldRecognizer, self)._onFrame()


class DragRecognizer(Recognizer):

    ANY_DIRECTION = 0
    VERTICAL = 1
    HORIZONTAL = 2

    DIRECTION_TOLERANCE = math.pi/4
    MIN_DRAG_DIST = None
    FRICTION = None

    def __init__(self, eventNode, coordSysNode=None, initialEvent=None, 
            direction=ANY_DIRECTION, directionTolerance=DIRECTION_TOLERANCE,
            friction=None, minDragDist=None,
            possibleHandler=None, failHandler=None, detectedHandler=None,
            moveHandler=None, upHandler=None, endHandler=None):

        if coordSysNode != None:
            self.__coordSysNode = weakref.ref(coordSysNode)
        else:
            self.__coordSysNode = weakref.ref(eventNode)
        self.__direction = direction
        self.__directionTolerance = directionTolerance

        if minDragDist != None:
            self.__minDragDist = minDragDist
        else:
            if self.__direction == DragRecognizer.ANY_DIRECTION:
                self.__minDragDist = 0
            else:
                self.__minDragDist = DragRecognizer.MIN_DRAG_DIST

        if friction == None:
            self.__friction = DragRecognizer.FRICTION
        else:
            self.__friction = friction

        self.__isSliding = False
        self.__inertiaHandler = None
        super(DragRecognizer, self).__init__(eventNode, True, 1, 
                initialEvent, possibleHandler=possibleHandler, failHandler=failHandler, 
                detectedHandler=detectedHandler, endHandler=endHandler)
        self.subscribe(Recognizer.MOTION, moveHandler)
        self.subscribe(Recognizer.UP, upHandler)

    def abort(self):
        if self.__inertiaHandler:
            self.__inertiaHandler.abort()
        self.__inertiaHandler = None
        super(DragRecognizer, self).abort()

    def _handleDown(self, event):
        if not self._handleCoordSysNodeUnlinked():
            if self.__inertiaHandler:
                self.__inertiaHandler.abort()
                self._setEnd(event)
            if self.__friction != -1:
                self.__inertiaHandler = InertiaHandler(self.__friction,
                        self.__onInertiaMove, self.__onInertiaStop)
            if self.__minDragDist == 0:
                self._setDetected(event)
            else:
                self._setPossible(event)
            pos = self.__relEventPos(event)
            self.__dragStartPos = pos
            self.__lastPos = pos

    def _handleMove(self, event):
        if not self._handleCoordSysNodeUnlinked():
            if self.getState() != "IDLE":
                pos = self.__relEventPos(event)
                offset = pos - self.__dragStartPos
                if self.getState() == "RUNNING":
                    self.notifySubscribers(Recognizer.MOTION, [offset]);
                else:
                    if offset.getNorm() > self.__minDragDist*player.getPixelsPerMM():
                        if self.__angleFits(offset):
                            self._setDetected(event)
                            self.notifySubscribers(Recognizer.MOTION, [offset]);
                        else:
                            self.__fail(event)
                if self.__inertiaHandler:
                    self.__inertiaHandler.onDrag(Transform(pos - self.__lastPos))
                self.__lastPos = pos

    def _handleUp(self, event):
        if not self._handleCoordSysNodeUnlinked():
            if self.getState() != "IDLE":
                pos = self.__relEventPos(event)
                if self.getState() == "RUNNING":
                    self.__offset = pos - self.__dragStartPos
                    self.notifySubscribers(Recognizer.UP, [self.__offset]);
                    if self.__friction != -1:
                        self.__isSliding = True
                        self.__inertiaHandler.onDrag(Transform(pos - self.__lastPos))
                        self.__inertiaHandler.onUp()
                    else:
                        self._setEnd(event)
                else:
                    self.__fail(event)

    def _handleCoordSysNodeUnlinked(self):
        if self.__coordSysNode().getParent():
            return False
        else:
            self.abort()
            return True

    def __fail(self, event):
        self._setFail(event)
        if self.__inertiaHandler:
            self.__inertiaHandler.abort()
        self.__inertiaHandler = None

    def __onInertiaMove(self, transform):
        self.__offset += transform.trans 
        self.notifySubscribers(Recognizer.MOTION, [self.__offset]);

    def __onInertiaStop(self):
        self.__inertiaHandler = None
        self.__isSliding = False
        if self.getState() == "POSSIBLE":
            self._setFail(None)
        else:
            self._setEnd(None)

    def __relEventPos(self, event):
        return self.__coordSysNode().getParent().getRelPos(event.pos)

    def __angleFits(self, offset):
        angle = offset.getAngle()
        if angle < 0:
            angle = -angle
        if self.__direction == DragRecognizer.VERTICAL:
            return (angle > math.pi/2-self.__directionTolerance 
                    and angle < math.pi/2+self.__directionTolerance)
        elif self.__direction == DragRecognizer.HORIZONTAL:
            return (angle < self.__directionTolerance 
                    or angle > math.pi-self.__directionTolerance)
        else:
            return True

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
        return Mat3x3([math.cos(a), -math.sin(a), 0],
                      [math.sin(a), math.cos(a), 0])

    @classmethod
    def pivotRotate(cls, t, a):
        rot = Mat3x3.rotate(a)
        trans = Mat3x3.translate(t)
        return trans.applyMat(rot.applyMat(trans.inverse()))

    @classmethod
    def scale(cls, s):
        return Mat3x3([s[0], 0, 0],
                      [0, s[1], 0])

    @classmethod
    def fromNode(cls, node):
        return Mat3x3.translate(node.pos).applyMat(
                Mat3x3.translate(node.pivot).applyMat(
                Mat3x3.rotate(node.angle).applyMat(
                Mat3x3.translate(-node.pivot).applyMat(
                Mat3x3.scale(node.size)))))

    def setNodeTransform(self, node):
        v = self.applyVec([1,0,0])
        rot = avg.Point2D(v[0], v[1]).getAngle()
        node.angle = rot
        if self.getScale().x < 9999 and self.getScale().y < 9999:
            node.size = self.getScale()
        else:
            node.size = (0,0)
        node.pivot = node.size/2 
        v = self.applyVec([0,0,1])
        node.pos = (avg.Point2D(v[0], v[1]) + (node.pivot).getRotated(node.angle) - 
                node.pivot)

    def getScale(self):
        v = self.applyVec([1,0,0])
        xscale = avg.Point2D(v[0], v[1]).getNorm()
        v = self.applyVec([0,1,0])
        yscale = avg.Point2D(v[0], v[1]).getNorm()
        return avg.Point2D(xscale, yscale)

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


def getCentroid(indexes, pts):
    c = avg.Point2D(0, 0)
    for i in indexes:
        c += pts[i]
    return c/len(indexes)

def calcKMeans(pts):

    # in: List of points
    # out: Two lists, each containing indexes into the input list
    assert(len(pts) > 1)
    p1 = pts[0]
    p2 = pts[1]
    oldP1 = None
    oldP2 = None
    j = 0
    while not(p1 == oldP1 and p2 == oldP2) and j < 50:
        l1 = []
        l2 = []
        # Group points
        for i, pt in enumerate(pts):
            dist1 = (pt-p1).getNorm()
            dist2 = (pt-p2).getNorm()
            if dist1 < dist2:
                l1.append(i)
            else:
                l2.append(i)
        oldP1 = p1
        oldP2 = p2
        p1 = getCentroid(l1, pts)
        p2 = getCentroid(l2, pts)
        j += 1
    return l1, l2


class Transform():
    def __init__(self, trans, rot=0, scale=1, pivot=(0,0)):
        self.trans = avg.Point2D(trans)
        self.rot = rot
        self.scale = scale
        self.pivot = avg.Point2D(pivot)

    def moveNode(self, node):
        transMat =  Mat3x3.translate(self.trans)
        rotMat = Mat3x3.rotate(self.rot)
        scaleMat = Mat3x3.scale((self.scale, self.scale))
        pivotMat = Mat3x3.translate(self.pivot)
        invPivotMat = pivotMat.inverse()
        startTransform = Mat3x3.fromNode(node)
        newTransform = pivotMat.applyMat(
                rotMat.applyMat(
                scaleMat.applyMat(
                invPivotMat.applyMat(
                transMat.applyMat(
                startTransform)))))
        newTransform.setNodeTransform(node)

    def __repr__(self):
        return "Transform"+str((self.trans, self.rot, self.scale, self.pivot))


class TransformRecognizer(Recognizer):

    FILTER_MIN_CUTOFF = None
    FILTER_BETA = None

    def __init__(self, eventNode, coordSysNode=None, initialEvent=None, friction=None, 
            detectedHandler=None, moveHandler=None, upHandler=None, endHandler=None):
        if coordSysNode != None:
            self.__coordSysNode = weakref.ref(coordSysNode)
        else:
            self.__coordSysNode = weakref.ref(eventNode)
        
        if friction == None:
            self.__friction = DragRecognizer.FRICTION
        else:
            self.__friction = friction

        self.__baseTransform = Mat3x3()
        self.__lastPosns = []
        self.__posns = []
        self.__inertiaHandler = None
        self.__filters = {}
        self.__frameHandlerID = None
        super(TransformRecognizer, self).__init__(eventNode, True, None, 
                initialEvent, detectedHandler=detectedHandler, endHandler=endHandler)
        self.subscribe(Recognizer.MOTION, moveHandler)
        self.subscribe(Recognizer.UP, upHandler)

    def enable(self, isEnabled):
        if bool(isEnabled) != self.isEnabled() and not(isEnabled):
            self.__abort()
        super(TransformRecognizer, self).enable(isEnabled)

    def abort(self):
        self.__abort()
        super(TransformRecognizer, self).abort()

    def _handleDown(self, event):
        numContacts = len(self._contacts)
        self.__newPhase()
        if self.__isFiltered():
            self.__filters[event.contact] = [
                    filter.OneEuroFilter(mincutoff=TransformRecognizer.FILTER_MIN_CUTOFF, 
                            beta=TransformRecognizer.FILTER_BETA),
                    filter.OneEuroFilter(mincutoff=TransformRecognizer.FILTER_MIN_CUTOFF,
                            beta=TransformRecognizer.FILTER_BETA)]
        if numContacts == 1:
            if self.__inertiaHandler:
                self.__inertiaHandler.abort()
                self._setEnd(event)
            self._setDetected(event)
            self.__frameHandlerID = player.subscribe(player.ON_FRAME, self.__onFrame)
            if self.__friction != -1:
                self.__inertiaHandler = InertiaHandler(self.__friction, 
                        self.__onInertiaMove, self.__onInertiaStop)

    def _handleUp(self, event):
        numContacts = len(self._contacts)
        if numContacts == 0:
            contact = event.contact
            transform = Transform(self.__filteredRelContactPos(contact)
                    - self.__lastPosns[0])
            player.unsubscribe(player.ON_FRAME, self.__frameHandlerID)
            self.__frameHandlerID = None
            if self.__friction != -1:
                self.__inertiaHandler.onDrag(transform)
                self.__inertiaHandler.onUp()
            else:
                self._setEnd(event)
            self.notifySubscribers(Recognizer.UP, [transform]);
        elif numContacts == 1:
            self.__newPhase()
        else:
            self.__newPhase()
        if self.__isFiltered():
            del self.__filters[event.contact]

    def _handleNodeGone(self):
        if ((self.__coordSysNode and not(self.__coordSysNode())) or
                self.__coordSysNode().getParent() == None):
            self.enable(False)
            return True
        else:
            return super(TransformRecognizer, self)._handleNodeGone()

    def __onFrame(self):
        nodeGone = self._handleNodeGone()
        if not(nodeGone):
            self.__move()

    def __move(self):
        numContacts = len(self._contacts)
        contactPosns = [self.__filteredRelContactPos(contact)
                for contact in self._contacts]
        if numContacts == 1:
            transform = Transform(contactPosns[0] - self.__lastPosns[0])
            if self.__friction != -1:
                self.__inertiaHandler.onDrag(transform)
            self.notifySubscribers(Recognizer.MOTION, [transform]);
            self.__lastPosns = contactPosns
        else:
            if numContacts == 2:
                self.__posns = contactPosns
            else:
                self.__posns = [getCentroid(self.__clusters[i], contactPosns) for
                        i in range(2)]

            startDelta = self.__lastPosns[1]-self.__lastPosns[0]
            curDelta = self.__posns[1]-self.__posns[0]

            pivot = (self.__posns[0]+self.__posns[1])/2

            rot = avg.Point2D.angle(curDelta, startDelta)

            if self.__lastPosns[0] == self.__lastPosns[1]:
                scale = 1
            else:
                scale = ((self.__posns[0]-self.__posns[1]).getNorm() / 
                        (self.__lastPosns[0]-self.__lastPosns[1]).getNorm())

            trans = ((self.__posns[0]+self.__posns[1])/2 - 
                    (self.__lastPosns[0]+self.__lastPosns[1])/2)
            transform = Transform(trans, rot, scale, pivot)
            if self.__friction != -1:
                self.__inertiaHandler.onDrag(transform)
            self.notifySubscribers(Recognizer.MOTION, [transform]);
            self.__lastPosns = self.__posns

    def __newPhase(self):
        self.__lastPosns = []
        numContacts = len(self._contacts)
        contactPosns = [self.__relContactPos(contact) 
                for contact in self._contacts]
        if numContacts == 1:
            self.__lastPosns.append(contactPosns[0])
        else:
            if numContacts == 2:
                self.__lastPosns = contactPosns
            else:
                self.__clusters = calcKMeans(contactPosns)
                self.__lastPosns = [getCentroid(self.__clusters[i], contactPosns) for
                        i in range(2)]

    def __onInertiaMove(self, transform):
        self.notifySubscribers(Recognizer.MOTION, [transform]);

    def __onInertiaStop(self):
        self.__inertiaHandler = None
        self._setEnd(None)

    def __filteredRelContactPos(self, contact):
        rawPos = self.__relContactPos(contact)
        if self.__isFiltered():
            f = self.__filters[contact]
            return avg.Point2D(f[0].apply(rawPos.x, player.getFrameTime()),
                    f[1].apply(rawPos.y, player.getFrameTime()))
        else:
            return rawPos

    def __relContactPos(self, contact):
        return self.__coordSysNode().getParent().getRelPos(contact.events[-1].pos)

    def __isFiltered(self):
        return TransformRecognizer.FILTER_MIN_CUTOFF != None

    def __abort(self):
        if self.__frameHandlerID:
            player.unsubscribe(player.ON_FRAME, self.__frameHandlerID)
            self.__frameHandlerID = None
        if self.__inertiaHandler:
            self.__inertiaHandler.abort()
            self.__inertiaHandler = None


class InertiaHandler(object):
    def __init__(self, friction, moveHandler, stopHandler):
        self.__friction = friction
        self.__moveHandler = moveHandler
        self.__stopHandler = stopHandler

        self.__transVel = avg.Point2D(0, 0)
        self.__curPivot = avg.Point2D(0, 0)
        self.__angVel = 0
        self.__sizeVel = avg.Point2D(0, 0)
        self.__frameHandlerID = player.subscribe(player.ON_FRAME, self.__onDragFrame)

    def abort(self):
        player.unsubscribe(player.ON_FRAME, self.__frameHandlerID)
        self.__stopHandler = None
        self.__moveHandler = None

    def onDrag(self, transform):
        frameDuration = player.getFrameDuration()
        if frameDuration > 0:
            self.__transVel += 0.1*transform.trans/frameDuration
        if transform.pivot != avg.Point2D(0,0):
            self.__curPivot = transform.pivot
        if transform.rot > math.pi:
            transform.rot -= 2*math.pi
        if frameDuration > 0:
            self.__angVel += 0.1*transform.rot/frameDuration

    def onUp(self):
        player.unsubscribe(player.ON_FRAME, self.__frameHandlerID)
        self.__frameHandlerID = player.subscribe(player.ON_FRAME, self.__onInertiaFrame)
        self.__onInertiaFrame()

    def __onDragFrame(self):
        self.__transVel *= 0.9
        self.__angVel *= 0.9

    def __onInertiaFrame(self):
        transNorm = self.__transVel.getNorm()
        if transNorm - self.__friction > 0:
            direction = self.__transVel.getNormalized()
            self.__transVel = direction * (transNorm-self.__friction)
            curTrans = self.__transVel * player.getFrameDuration()
        else:
            curTrans = avg.Point2D(0, 0)

        if self.__angVel != 0:
            angSign = self.__angVel/math.fabs(self.__angVel)
            self.__angVel = self.__angVel - angSign*self.__friction/200
            newAngSign = self.__angVel/math.fabs(self.__angVel)
            if newAngSign != angSign:
                self.__angVel = 0
        curAng = self.__angVel * player.getFrameDuration()
        self.__curPivot += curTrans

        if transNorm - self.__friction > 0 or self.__angVel != 0:
            if self.__moveHandler:
                self.__moveHandler(Transform(curTrans,  curAng, 1, self.__curPivot))
        else:
            self.__stop()

    def __stop(self):
        player.unsubscribe(player.ON_FRAME, self.__frameHandlerID)
        self.__stopHandler()
        self.__stopHandler = None
        self.__moveHandler = None

    
def initConfig():
    def getFloatOption(name):
        return float(player.getConfigOption("gesture", name))

    TapRecognizer.MAX_TAP_DIST = getFloatOption("maxtapdist")
    DoubletapRecognizer.MAX_DOUBLETAP_TIME = getFloatOption("maxdoubletaptime")
    SwipeRecognizer.MIN_SWIPE_DIST = getFloatOption("minswipedist")
    SwipeRecognizer.SWIPE_DIRECTION_TOLERANCE = getFloatOption("swipedirectiontolerance")
    SwipeRecognizer.MAX_SWIPE_CONTACT_DIST = getFloatOption("maxswipecontactdist")
    HoldRecognizer.HOLD_DELAY = getFloatOption("holddelay")
    DragRecognizer.MIN_DRAG_DIST = getFloatOption("mindragdist")
    DragRecognizer.FRICTION = getFloatOption("friction")
    TransformRecognizer.FILTER_MIN_CUTOFF = getFloatOption("filtermincutoff")
    if TransformRecognizer.FILTER_MIN_CUTOFF == -1:
        TransformRecognizer.FILTER_MIN_CUTOFF = None
    TransformRecognizer.FILTER_BETA = getFloatOption("filterbeta")


initConfig()
