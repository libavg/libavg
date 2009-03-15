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

from libavg import Point2D
from eventList import EventList, Cursor
from mathutil import getDistSquared


MAX_ITERATIONS=50

class Centroid (Cursor):
    def __init__(self):
        super(Centroid, self).__init__()
        self.__members = []

    def isBroken(self):
        return len(self.__members)==0

    def reposition (self):
        pointSum = Point2D(0,0)
        for point in self.__members:
            pointSum += point.getPos()
        center = pointSum / len(self.__members)
        oldPos = self._pos
        self.setPos(center)
        return oldPos != self._pos

    def removeMember (self, member):
        self.__members.remove(member)
        self.reposition()
        self.resetMotion()

    def addMember (self, member):
        self.__members.append(member)
        self.reposition()
        self.resetMotion()

    def hasMember (self, member):
        return member in self.__members

    def __len__(self):
        return len(self.__members)

    def __repr__(self):
        return "centroid at %s, startpos %s,  members %s" % (self._pos, self._startPos, self.__members)


class ClusteredEventList:
    """ implements a variant of k-means.
    same API as EventList, with the difference that ClusteredEventList
    will simulate a maximum of 2 cursors (if more actual cursors are
    on the node, they are clustered.
    In contrast to EventList, the callbacks provide no EventCursors.
    """
    def __init__(self,
            node,
            source,
            onDown = lambda x: None,
            onUp = lambda x: None,
            onMotion = lambda x: None,
            resetMotion = lambda: None,
            captureEvents = True):
        self.__centroids = []
        self.__centroidByEvent = {}
        self.__doNewMotion = False

        self.__callback = {
                'onDown': onDown,
                'onUp': onUp,
                'onMotion': onMotion,
                'resetMotion': resetMotion,
        }
        self.__eventList = EventList(
                node = node,
                source = source,
                onDown = self.__onDown,
                onUp = self.__onUp,
                onMotion = self.__onMotion,
                resetMotion = self.__resetMotion,
                captureEvents = captureEvents)

    def handleInitialDown(self,event):
        self.__eventList.handleInitialDown(event)

    def __onDown(self, eventCursor):
        #self.__callback['onDown'](eventCursor)
        centroids = list(self.__centroids) # copy
        self.calcClusters()
        self.__resetMotion()
        if len(centroids) != len(self.__centroids):
            if len(centroids) and self.__centroids[0] == centroids[0]:
                newCentroid = self.__centroids[1]
            else:
                newCentroid = self.__centroids[0]
            self.__callback['onDown']()

    def __onUp(self, eventCursor):
        if eventCursor in self.__centroidByEvent:
            centroid = self.__centroidByEvent[eventCursor]
            centroid.removeMember(eventCursor)
            if len(centroid) == 0:
                self.__callback['onUp']()
            del self.__centroidByEvent[eventCursor]
        else:
            print "eventCursor %s not found" % eventCursor

        self.calcClusters()
        self.__resetMotion()

    def __onMotion(self, eventCursor):
        oldPositions = {}
        for centroid in self.__centroids:
            oldPositions[centroid] = centroid.getPos()
        self.calcClusters()
        self.__callback['onMotion']()

    def __resetMotion(self):
        for centroid in self.__centroids:
            centroid.resetMotion()
        self.__callback['resetMotion']()

    def delete(self):
        self.__centroids = []
        self.__centroidByEvent = {}
        self.__eventList.delete()

    def __calcMemberships (self):
        """ returns True if a membership changed, else False."""
        changed = False
        if not len(self.__centroids):
            return changed
        for point in self.__eventList.getCursors():
            closestCentroid = self.__centroids[0]
            minDist = getDistSquared(point.getPos(), closestCentroid.getPos())
            for centroid in self.__centroids:
                distance = getDistSquared (point.getPos(), centroid.getPos())
                if distance < minDist:
                    minDist = distance
                    closestCentroid = centroid
            if not closestCentroid.hasMember(point):
                self.__doNewMotion = True
                if point in self.__centroidByEvent:
                    self.__centroidByEvent[point].removeMember(point)
                self.__centroidByEvent[point] = closestCentroid
                closestCentroid.addMember(point)
                changed = True
        return changed

    def __tryCalcClusters (self):
        def __calcInitialCentroids():
            self.__centroidByEvent = {}
            self.__centroids = []
            def createCentroid(point):
                centroid = Centroid()
                centroid.addMember(point)
                self.__centroidByEvent[point] = centroid
                self.__centroids.append(centroid)

            maxDist = 0
            points = None
            if len(self.__eventList)>1:
                cursors = self.__eventList.getCursors()
                for p in cursors:
                    for q in cursors:
                        dist = getDistSquared(p.getPos(),q.getPos())
                        if dist >= maxDist:
                            points = p,q
                            maxDist = dist

                assert(points)
                assert(points[0] != points[1])
                for point in points:
                    createCentroid(point)
            elif len(self.__eventList) == 1:
                createCentroid(self.__eventList.getCursors()[0])

        def __setCentroids():
            changed = False
            for centroid in self.__centroids:
                if centroid.reposition():
                    changed = True
            return changed

        if not len(self.__centroids):
            __calcInitialCentroids()
        self.__calcMemberships()

        changed = True
        iterations = 0
        while changed:
            changed = False
            if __setCentroids():
                changed = True
            if self.__calcMemberships():
                changed = True
            iterations+=1
            if iterations>MAX_ITERATIONS:
                #print "too many iterations(%u), aborting" % iterations
                __setCentroids()
                break

    def calcClusters (self):
        def __hasBrokenCentroids():
            if len(self.__eventList)>1 and len(self.__centroids)!=2:
                return True
            for centroid in self.__centroids:
                if centroid.isBroken():
                    return True
            if len(self.__centroids)==2:
                if self.__centroids[0].getPos() == self.__centroids[1].getPos():
                    return True
            return False

        self.__tryCalcClusters()

        if __hasBrokenCentroids():
            #print "i have broken centroids"
            self.__centroids=[]
            self.__tryCalcClusters()
            if __hasBrokenCentroids():
                print "ignoring fatal error: cannot fix broken centroids: %s" % self.__centroids
                print "self.__eventList =%s " % self.__eventList
        if self.__doNewMotion:
            self.__doNewMotion = False
            self.__resetMotion()

    def getCursors (self):
        return self.__centroids

    def __len__(self):
        return min(len(self.__eventList), len(self.__centroids))

