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

    def __init__(self, contentNode, hScrollBar=None, vScrollBar=None, parent=None, 
            **kwargs):

        super(ScrollArea, self).__init__(**kwargs)
        self.registerInstance(self, parent)

        self._hScrollBar = hScrollBar
        self.appendChild(hScrollBar)
        self._vScrollBar = vScrollBar
        self.appendChild(vScrollBar)

        self.__scrollPane = ScrollPane(contentNode=contentNode, 
                size=self.size - (vScrollBar.width, hScrollBar.height),
                parent=self)

        self._positionScrollBars()
        
        if hScrollBar:
            hScrollBar.subscribe(slider.Slider.THUMB_POS_CHANGED, self.__onHThumbMove)
        if vScrollBar:
            vScrollBar.subscribe(slider.Slider.THUMB_POS_CHANGED, self.__onVThumbMove)
        self.recognizer = gesture.DragRecognizer(
                eventNode=self.__scrollPane, 
                detectedHandler=self.__onDragStart,
                moveHandler=self.__onDragMove,
                upHandler=self.__onDragMove,
                )

    def getContentSize(self):
        return self.__scrollPane._contentNode.size

    def setContentSize(self, size):
        self.__scrollPane.contentsize = size
        self._positionScrollBars()
    contentsize = property(getContentSize, setContentSize)

    def getContentPos(self):
        return self.__scrollPane.contentpos

    def setContentPos(self, pos):
        self.__scrollPane.contentpos = pos
        self._positionScrollBars()
    contentpos = property(getContentPos, setContentPos)

    def __onHThumbMove(self, thumbPos):
        self.__scrollPane.contentpos = (thumbPos, self.__scrollPane.contentpos.y)

    def __onVThumbMove(self, thumbPos):
        self.__scrollPane.contentpos = (self.__scrollPane.contentpos.x, thumbPos)

    def __onDragStart(self, event):
        self.__dragStartPos = self.__scrollPane.contentpos

    def __onDragMove(self, event, offset):
        contentpos = self.__dragStartPos - offset
        self.__scrollPane.contentpos = contentpos
        if self._hScrollBar:
            self._hScrollBar.thumbpos = contentpos.x
        if self._vScrollBar:
            self._vScrollBar.thumbpos = contentpos.y

    def _positionScrollBars(self):
        if self._hScrollBar:
            self._hScrollBar.pos = (0, self.__scrollPane.height)
            self._hScrollBar.extent = self.__scrollPane.width

            if self.__scrollPane.contentsize.x <= self.__scrollPane.width:
                self._hScrollBar.range = (0, self.__scrollPane.width)
                self._hScrollBar.enabled = False
            else:
                self._hScrollBar.range = (0, self.__scrollPane.contentsize.x)
                self._hScrollBar.enabled = True
            self._hScrollBar.thumbextent = self.__scrollPane.width

        if self._vScrollBar:
            self._vScrollBar.pos = (self.__scrollPane.width, 0)
            self._vScrollBar.extent = self.__scrollPane.height

            if self.__scrollPane.contentsize.y <= self.__scrollPane.height:
                self._vScrollBar.range = (0, self.__scrollPane.height)
                self._vScrollBar.enabled = False
            else:
                self._vScrollBar.range = (0, self.__scrollPane.contentsize.y)
                self._vScrollBar.enabled = True
            self._vScrollBar.thumbextent = self.__scrollPane.height
        
