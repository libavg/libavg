# -*- coding: utf-8 -*-
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

from libavg import avg
from libavg.ui import slider, gesture

class ScrollPane(avg.DivNode):

    def __init__(self, contentNode, parent=None, **kwargs):

        super(ScrollPane, self).__init__(crop=True, **kwargs)
        self.registerInstance(self, parent)
        
        self.appendChild(contentNode)
        self._contentNode = contentNode

    def setContentPos(self, pos):

        def constrain(pos, limit):
            if limit > 0:
                # Content larger than container
                if pos > limit:
                    pos = limit
                elif pos < 0:
                    pos = 0
            else:
                # Content smaller than container
                if pos > 0:
                    pos = 0
                elif pos < limit:
                    pos = limit
            return pos

        maxPos = self.getMaxContentPos()
        pos = avg.Point2D(pos)
        pos.x = constrain(pos.x, maxPos.x)
        pos.y = constrain(pos.y, maxPos.y)
        self._contentNode.pos = -pos

    def getContentPos(self):
        return -self._contentNode.pos
    contentpos = property(getContentPos, setContentPos)

    def getContentSize(self):
        return self._contentNode.size

    def setContentSize(self, size):
        self._contentNode.size = size
        self.setContentPos(-self._contentNode.pos) # Recheck constraints. 
    contentsize = property(getContentSize, setContentSize)

    def getMaxContentPos(self):
        maxPos = avg.Point2D(self._contentNode.size - self.size)
        if maxPos.x < 0:
            maxPos.x = 0
        if maxPos.y < 0:
            maxPos.y = 0
        return maxPos


class ScrollArea(avg.DivNode):

    PRESSED = avg.Publisher.genMessageID()
    RELEASED = avg.Publisher.genMessageID()
    CONTENT_POS_CHANGED = avg.Publisher.genMessageID()

    def __init__(self, contentNode, size, hScrollBar=None, vScrollBar=None, parent=None, 
            friction=None, **kwargs):

        super(ScrollArea, self).__init__(**kwargs)
        self.registerInstance(self, parent)
        self.publish(self.PRESSED)
        self.publish(self.RELEASED)
        self.publish(self.CONTENT_POS_CHANGED)

        self._hScrollBar = hScrollBar
        self._vScrollBar = vScrollBar

        if hScrollBar:
            self.appendChild(hScrollBar)
            hScrollBar.subscribe(slider.Slider.THUMB_POS_CHANGED, self.__onHThumbMove)
        if vScrollBar:
            self.appendChild(vScrollBar)
            vScrollBar.subscribe(slider.Slider.THUMB_POS_CHANGED, self.__onVThumbMove)

        self.__scrollPane = ScrollPane(contentNode=contentNode, parent=self)
        self.size = size
        
        self.recognizer = gesture.DragRecognizer(
                eventNode=self.__scrollPane, 
                detectedHandler=self.__onDragStart,
                moveHandler=self.__onDragMove,
                upHandler=self.__onDragUp,
                friction=friction
                )

    def getContentSize(self):
        return self.__scrollPane._contentNode.size

    def setContentSize(self, size):
        self.__scrollPane.contentsize = size
        self.__positionNodes()
    contentsize = property(getContentSize, setContentSize)

    def getContentPos(self):
        return self.__scrollPane.contentpos

    def setContentPos(self, pos):
        self.__scrollPane.contentpos = pos
        self.__positionNodes()
        self.__positionThumbs(avg.Point2D(pos))
        self.notifySubscribers(self.CONTENT_POS_CHANGED, [self.__scrollPane.contentpos])
    contentpos = property(getContentPos, setContentPos)

    def getSize(self):
        return self.__baseSize

    def setSize(self, size):
        self.__baseSize = size
        self.__positionNodes()
    __baseSize = avg.DivNode.size
    size = property(getSize, setSize)   

    def __onHThumbMove(self, thumbPos):
        self.__scrollPane.contentpos = (thumbPos, self.__scrollPane.contentpos.y)
        self.notifySubscribers(self.CONTENT_POS_CHANGED, [self.__scrollPane.contentpos])

    def __onVThumbMove(self, thumbPos):
        self.__scrollPane.contentpos = (self.__scrollPane.contentpos.x, thumbPos)
        self.notifySubscribers(self.CONTENT_POS_CHANGED, [self.__scrollPane.contentpos])

    def __onDragStart(self):
        self.__dragStartPos = self.__scrollPane.contentpos
        self.notifySubscribers(self.PRESSED, [])

    def __onDragMove(self, offset):
        contentpos = self.__dragStartPos - offset
        self.__scrollPane.contentpos = contentpos
        self.__positionThumbs(contentpos)
        self.notifySubscribers(self.CONTENT_POS_CHANGED, [self.__scrollPane.contentpos])

    def __onDragUp(self, offset):
        self.__onDragMove(offset)
        self.notifySubscribers(self.RELEASED, [])

    def __positionNodes(self):
        paneSize = self.__baseSize
        if self._hScrollBar:
            paneSize -= (0, self._hScrollBar.height)
        if self._vScrollBar:
            paneSize -= (self._vScrollBar.width, 0)
        self.__scrollPane.size = paneSize

        if self._hScrollBar:
            self._hScrollBar.pos = (0, self.__scrollPane.height)
            self._hScrollBar.width = self.__scrollPane.width

            if self.__scrollPane.contentsize.x <= self.__scrollPane.width:
                self._hScrollBar.range = (0, self.__scrollPane.width)
                self._hScrollBar.enabled = False
            else:
                self._hScrollBar.range = (0, self.__scrollPane.contentsize.x)
                self._hScrollBar.enabled = True
            self._hScrollBar.thumbextent = self.__scrollPane.width

        if self._vScrollBar:
            self._vScrollBar.pos = (self.__scrollPane.width, 0)
            self._vScrollBar.height = self.__scrollPane.height

            if self.__scrollPane.contentsize.y <= self.__scrollPane.height:
                self._vScrollBar.range = (0, self.__scrollPane.height)
                self._vScrollBar.enabled = False
            else:
                self._vScrollBar.range = (0, self.__scrollPane.contentsize.y)
                self._vScrollBar.enabled = True
            self._vScrollBar.thumbextent = self.__scrollPane.height

    def __positionThumbs(self, contentPos):
        if self._hScrollBar:
            self._hScrollBar.thumbpos = contentPos.x
        if self._vScrollBar:
            self._vScrollBar.thumbpos = contentPos.y

