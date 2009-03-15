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

from libavg import avg

class Cursor(object):
    def __init__(self):
        self._pos = None
        self.resetMotion()

    def resetMotion(self):
        self._startPos = self._pos

    def getDelta (self):
        return self._pos - self._startPos

    def getStartPos (self):
        return self._startPos

    def getPos (self):
        return self._pos

    def setPos (self, newPos):
        self._pos = newPos
        if not self._startPos:
            self._startPos = self._pos

    def getX(self):
        return self._pos.x
    x = property(getX)

    def getY(self):
        return self._pos.y
    y = property(getY)

    def __unicode__ (self):
        print "cursor %s->%s" % (self.getStartPos(), self.getPos())

class EventCursor(Cursor):
    def __init__(self, event):
        super(EventCursor, self).__init__()
        self.__cursorid = event.cursorid
        self.update (event)

    def update (self, event):
        assert event.cursorid == self.__cursorid
        self.setPos(event.pos)
        self.__speed = event.speed

    def getSpeed (self):
        return self.__speed

    def getCursorID (self):
        return self.__cursorid

class EventList:
    """Keep track of cursors pressed on a Node"""
    def __init__(self,
            node,
            source,
            onDown = lambda x: None,
            onUp = lambda x: None,
            onMotion = lambda x: None,
            resetMotion = lambda: None,
            maxEvents = None,
            captureEvents = True):

        self.__node = node
        self.__source = source
        self.__callback = {
                'onDown': onDown,
                'onUp': onUp,
                'onMotion': onMotion,
                'resetMotion': resetMotion,
        }

        self.__maxEvents = maxEvents
        self.__captureEvents = captureEvents

        self.__capturedCursors = []
        self.__node.setEventHandler (avg.CURSORDOWN, self.__source, self.__onDown)
        self.__node.setEventHandler (avg.CURSORUP, self.__source, self.__onUp)
        self.__node.setEventHandler (avg.CURSORMOTION, self.__source, self.__onMotion)

        self.__eventCursors = []

    def handleInitialDown(self, event):
        self.__onDown(event)

    def delete(self):
        for cursorid in self.__capturedCursors:
            self.__node.releaseEventCapture (cursorid)
        self.__capturedCursors = []

        for type_ in avg.CURSORDOWN, avg.CURSORMOTION, avg.CURSORUP:
            self.__node.setEventHandler(type_, self.__source, None)
        self.__node = None

    def __setitem__(self, key, val):
        raise Exception

    def __findEventCursor(self, event):
        found = filter (lambda ec: ec.getCursorID() == event.cursorid, self.__eventCursors)
        return found[0] if found else None

    def __resetMotion(self):
        map(EventCursor.resetMotion, self.__eventCursors)
        self.__callback['resetMotion']()

    def __onUp(self, event):
        def __removeCursor (cursor):
            def __releaseCapture (cursorid):
                if cursorid in self.__capturedCursors:
                    try:
                        self.__node.releaseEventCapture (cursorid)
                        self.__capturedCursors.remove (cursorid)
                    except RuntimeError: # XXX
                        print "warning: could not release event capture for cursor %u" % cursorid

            __releaseCapture (cursor.getCursorID())
            self.__eventCursors.remove (cursor)
            self.__resetMotion ()
        ec = self.__findEventCursor (event)
        if ec:
            ec.update (event)
            __removeCursor (ec)
            self.__callback['onUp'](ec)

    def __onDown(self, event):
        if self.__maxEvents and len(self.__eventCursors) >= self.__maxEvents:
            return 

        if self.__captureEvents:
            try:
                self.__node.setEventCapture(event.cursorid)
                self.__capturedCursors.append(event.cursorid)
            except RuntimeError:
                print "warning: could capture events for cursor %u" % event.cursorid

        ec = EventCursor (event)
        self.__eventCursors.append (ec)
        self.__resetMotion ()
        self.__callback['onDown'](ec)

    def __onMotion(self, event):
        ec = self.__findEventCursor (event)
        if ec:
            ec.update (event)
            self.__callback['onMotion'](ec)

    def getCursors(self):
        return self.__eventCursors

    def __len__(self):
        return len(self.__eventCursors)
