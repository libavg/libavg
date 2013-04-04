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

from libavg import avg, gesture
from . import slider
from base import HVStretchNode, Orientation
from . import skin

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

    def __init__(self, contentNode, size, skinObj=skin.Skin.default, enabled=True,
            scrollBars=(Orientation.HORIZONTAL, Orientation.VERTICAL),
            parent=None, **kwargs):

        super(ScrollArea, self).__init__(**kwargs)
        self.registerInstance(self, parent)
        self.cfg = skinObj.defaultScrollAreaCfg

        self.publish(self.PRESSED)
        self.publish(self.RELEASED)
        self.publish(self.CONTENT_POS_CHANGED)

        self.__scrollPane = ScrollPane(contentNode=contentNode, parent=self)

        if "borderBmp" in self.cfg:
            endsExtent = self.cfg["borderEndsExtent"]
            self._borderNode = HVStretchNode(src=self.cfg["borderBmp"], 
                    endsExtent=endsExtent, sensitive=False, parent=self)

        sensitiveScrollBars = self.cfg["sensitiveScrollBars"]

        if Orientation.HORIZONTAL in scrollBars:
            self._hScrollBar = slider.ScrollBar(sensitive=sensitiveScrollBars, 
                    parent=self, skinObj=skinObj)
            self._hScrollBar.subscribe(slider.Slider.THUMB_POS_CHANGED,
                    self.__onHThumbMove)
        else:
            self._hScrollBar = None

        if Orientation.VERTICAL in scrollBars:
            self._vScrollBar = slider.ScrollBar(orientation=Orientation.VERTICAL,
                    sensitive=sensitiveScrollBars, parent=self, skinObj=skinObj)
            self._vScrollBar.subscribe(slider.Slider.THUMB_POS_CHANGED,
                    self.__onVThumbMove)
        else:
            self._vScrollBar = None

        self.subscribe(self.SIZE_CHANGED, self.__positionNodes)
        self.size = size
       
        self.__enabled = True
        if not(enabled):
            self.setEnabled(False)

        self.recognizer = gesture.DragRecognizer(
                eventNode=self.__scrollPane, 
                detectedHandler=self.__onDragStart,
                moveHandler=self.__onDragMove,
                upHandler=self.__onDragUp,
                friction=self.cfg["friction"]
                )

    def getContentSize(self):
        return self.__scrollPane._contentNode.size

    def setContentSize(self, size):
        self.__scrollPane.contentsize = size
        self.__positionNodes(self.size)
    contentsize = property(getContentSize, setContentSize)

    def getContentPos(self):
        return self.__scrollPane.contentpos

    def setContentPos(self, pos):
        self.__scrollPane.contentpos = pos
        self.__positionNodes(self.size)
        self.__positionThumbs(avg.Point2D(pos))
        self.notifySubscribers(self.CONTENT_POS_CHANGED, [self.__scrollPane.contentpos])
    contentpos = property(getContentPos, setContentPos)

    def getEnabled(self):
        return self.__enabled

    def setEnabled(self, enabled):
        if enabled and not(self.__enabled):
            self.recognizer.enable(True)
        elif not(enabled) and self.__enabled:
            self.recognizer.enable(False)

        if self._vScrollBar:
            self._vScrollBar.enabled = enabled
        if self._hScrollBar:
            self._hScrollBar.enabled = enabled
        self.__enabled = enabled
    enabled = property(getEnabled, setEnabled)

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

    def __positionNodes(self, size):
        paneSize = size
        self._borderNode.size = size

        margins = self.cfg["margins"]
        if self._hScrollBar:
            paneSize -= (0, margins[0]+margins[2])
        if self._vScrollBar:
            paneSize -= (margins[1]+margins[3], 0)
        self.__scrollPane.pos = (margins[0], margins[1])
        self.__scrollPane.size = paneSize


        if self._hScrollBar:
            self._hScrollBar.pos = (0, size.y-self._hScrollBar.height)
            self._hScrollBar.width = self.__scrollPane.width

            if self.__scrollPane.contentsize.x <= self.__scrollPane.width:
                self._hScrollBar.range = (0, self.__scrollPane.width)
                self._hScrollBar.enabled = False
            else:
                self._hScrollBar.range = (0, self.__scrollPane.contentsize.x)
                self._hScrollBar.enabled = True
            self._hScrollBar.thumbExtent = self.__scrollPane.width

        if self._vScrollBar:
            self._vScrollBar.pos = (size.x-self._vScrollBar.width, 0)
            self._vScrollBar.height = self.__scrollPane.height

            if self.__scrollPane.contentsize.y <= self.__scrollPane.height:
                self._vScrollBar.range = (0, self.__scrollPane.height)
                self._vScrollBar.enabled = False
            else:
                self._vScrollBar.range = (0, self.__scrollPane.contentsize.y)
                self._vScrollBar.enabled = True
            self._vScrollBar.thumbExtent = self.__scrollPane.height

    def __positionThumbs(self, contentPos):
        if self._hScrollBar:
            self._hScrollBar.thumbPos = contentPos.x
        if self._vScrollBar:
            self._vScrollBar.thumbPos = contentPos.y

