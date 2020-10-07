#!/usr/bin/env python
# -*- coding: utf-8 -*-

# libavg - Media Playback Engine.
# Copyright (C) 2020 Thomas Schott, <scotty at c-base dot org>
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

import logging
import random

from libavg import avg, player, Point2D

logger = logging.getLogger(__name__)


class ClickTest(object):
    def __init__(self, mouse=False, maxTouches=10, probabilities=(0.5, 0.1, 0.2)):
        self._mouse = bool(mouse)
        self._maxTouches = max(0, maxTouches)
        self._probabilities = tuple(min(1.0, max(0.0, p)) for p in probabilities)
        self._contacts = None
        self._frameHandlerID = None

    def start(self):
        if self._frameHandlerID is not None:
            return

        def onFrame():
            for contact in self._contacts:
                contact.update()

        self._contacts = [
            _TouchContact(self._probabilities) for _ in xrange(self._maxTouches)
        ]
        if self._mouse:
            self._contacts.append(_MouseContact(self._probabilities))
        self._frameHandlerID = player.subscribe(player.ON_FRAME, onFrame)
        logger.info('click test started')

    def stop(self):
        if self._frameHandlerID is None or not player.isPlaying():
            return

        player.unsubscribe(self._frameHandlerID)
        self._frameHandlerID = None
        for contact in self._contacts:
            contact.delete()
        self._contacts = None
        logger.info('click test stopped')


class _Contact(object):
    def __init__(self, probabilities):
        self._downProbability, self._upProbability, self._moveProbability = probabilities
        rootNode = player.getRootNode()
        self._maxX = rootNode.width - 1
        self._maxY = rootNode.height - 1
        self._posNode = avg.CircleNode(
            r=10, strokewidth=2, fillcolor='FF0000', fillopacity=0.5,
            active=False, sensitive=False, parent=rootNode
        )
        self._lineNode = avg.PolyLineNode(
            color='FF0000', active=False, sensitive=False, parent=rootNode
        )
        self._testHelper = player.getTestHelper()

    def delete(self):
        if self._posNode.active:
            self._up()
        self._posNode.unlink(True)
        self._lineNode.unlink(True)

    def update(self):
        if self._posNode.active:
            if random.random() <= self._upProbability:
                self._up()
            elif random.random() <= self._moveProbability:
                self._move()
        elif random.random() <= self._downProbability:
            self._down()

    def _down(self):
        assert not self._posNode.active
        self._posNode.pos = self._getRandomPos()
        self._lineNode.pos = [self._posNode.pos]
        self._posNode.active = True

    def _up(self):
        assert self._posNode.active
        self._posNode.active = False
        self._lineNode.active = False

    def _move(self):
        assert self._posNode.active
        self._posNode.pos = self._getRandomPos()
        self._lineNode.pos = self._lineNode.pos + [self._posNode.pos]
        self._lineNode.active = True

    def _getRandomPos(self):
        return Point2D(random.randint(0, self._maxX), random.randint(0, self._maxY))


class _MouseContact(_Contact):
    def _down(self):
        super(_MouseContact, self)._down()
        x, y = self._posNode.pos
        self._testHelper.fakeMouseEvent(
            avg.Event.CURSOR_DOWN, True, False, False, int(x), int(y), 1
        )

    def _up(self):
        super(_MouseContact, self)._up()
        x, y = self._posNode.pos
        self._testHelper.fakeMouseEvent(
            avg.Event.CURSOR_UP, True, False, False, int(x), int(y), 1
        )

    def _move(self):
        super(_MouseContact, self)._move()
        x, y = self._posNode.pos
        self._testHelper.fakeMouseEvent(
            avg.Event.CURSOR_MOTION, True, False, False, int(x), int(y), 0
        )


class _TouchContact(_Contact):
    __nextID = 0

    @classmethod
    def _getNextID(cls):
        nextID = cls.__nextID
        cls.__nextID += 1
        return nextID

    def __init__(self, probabilities):
        super(_TouchContact, self).__init__(probabilities)
        self._id = None

    def _down(self):
        assert self._id is None
        super(_TouchContact, self)._down()
        self._id = self._getNextID()
        self._testHelper.fakeTouchEvent(
            self._id, avg.Event.CURSOR_DOWN, avg.Event.TOUCH, self._posNode.pos
        )

    def _up(self):
        assert self._id is not None
        super(_TouchContact, self)._up()
        self._testHelper.fakeTouchEvent(
            self._id, avg.Event.CURSOR_UP, avg.Event.TOUCH, self._posNode.pos
        )
        self._id = None

    def _move(self):
        assert self._id is not None
        super(_TouchContact, self)._move()
        self._testHelper.fakeTouchEvent(
            self._id, avg.Event.CURSOR_MOTION, avg.Event.TOUCH, self._posNode.pos
        )


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)

    player.createMainCanvas(size=(800, 600))
    ct = ClickTest(mouse=True, maxTouches=3)
    ct.start()
    player.setTimeout(5000, ct.stop)
    player.play()
    ct.stop()
