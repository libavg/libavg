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
            if limit < 0:
                # Content larger than container
                if pos < limit:
                    pos = limit
                elif pos > 0:
                    pos = 0
            else:
                # Content smaller than container
                if pos < 0:
                    pos = 0
                elif pos > limit:
                    pos = limit
            return pos

        maxPos = self.getMaxContentPos()
        pos = avg.Point2D(pos)
        pos.x = constrain(pos.x, maxPos.x)
        pos.y = constrain(pos.y, maxPos.y)
        self._contentNode.pos = pos

    def getContentPos(self):
        return self._contentNode.pos
    contentpos = property(getContentPos, setContentPos)

    def getContentSize(self):
        return self._contentNode.size

    def setContentSize(self, size):
        pass
    contentsize = property(getContentSize, setContentSize)

    def getMaxContentPos(self):
        maxPos = self.size - self._contentNode.size
        return maxPos


class ScrollArea(avg.DivNode):

    def __init__(self, scrollPane, hScrollBar=None, vScrollBar=None, parent=None, 
            **kwargs):

        super(ScrollArea, self).__init__(**kwargs)
        self.registerInstance(self, parent)

        self._scrollPane = scrollPane
        self.appendChild(scrollPane)
        self._hScrollBar = hScrollBar
        self.appendChild(hScrollBar)
        self._vScrollBar = vScrollBar
        self.appendChild(vScrollBar)

        scrollPane.size = self.size - (vScrollBar.width, hScrollBar.height)

        if hScrollBar:
            hScrollBar.pos = (0, scrollPane.height)
            hScrollBar.extent = scrollPane.width
            hScrollBar.range = (0, scrollPane.contentsize.x)
            hScrollBar.thumbExtent = self.size.x
            hScrollBar.subscribe(slider.Slider.THUMB_POS_CHANGED, self.__onHThumbMove)

        if vScrollBar:
            vScrollBar.pos = (scrollPane.width, 0)
            vScrollBar.extent = scrollPane.height
            vScrollBar.range = (0, scrollPane.contentsize.y)
            vScrollBar.thumbExtent = self.size.y
            vScrollBar.subscribe(slider.Slider.THUMB_POS_CHANGED, self.__onVThumbMove)

        self.recognizer = gesture.DragRecognizer(
                eventNode=self._scrollPane, 
                detectedHandler=self.__onDragStart,
                moveHandler=self.__onDragMove,
                upHandler=self.__onDragMove,
                )

    def __onHThumbMove(self, thumbPos):
        self._scrollPane.contentpos = (-thumbPos, self._scrollPane.contentpos.y)

    def __onVThumbMove(self, thumbPos):
        self._scrollPane.contentpos = (self._scrollPane.contentpos.x, -thumbPos)

    def __onDragStart(self, event):
        self.__dragStartPos = self._scrollPane.contentpos

    def __onDragMove(self, event, offset):
        contentpos = self.__dragStartPos + offset
        self._scrollPane.contentpos = contentpos
        if self._hScrollBar:
            self._hScrollBar.thumbPos = -contentpos.x
        if self._vScrollBar:
            self._vScrollBar.thumbPos = -contentpos.y

