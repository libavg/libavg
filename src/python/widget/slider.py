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

from libavg import avg, gesture
from base import SwitchNode, HStretchNode, VStretchNode, Orientation
from . import skin

import math


class ScrollBarTrack(SwitchNode):
    
    def __init__(self, bmp, endsExtent, disabledBmp, orientation=Orientation.HORIZONTAL, 
            **kwargs):
      
        super(ScrollBarTrack, self).__init__(nodeMap=None, **kwargs)
        if orientation == Orientation.HORIZONTAL:
            StretchNode = HStretchNode
        else:
            StretchNode = VStretchNode
        self.__enabledNode = StretchNode(src=bmp, endsExtent=endsExtent, parent=self)
        self.__disabledNode = StretchNode(src=disabledBmp, endsExtent=endsExtent,
                parent=self)
        
        self.setNodeMap({
            "ENABLED": self.__enabledNode, 
            "DISABLED": self.__disabledNode
        })
        self.visibleid = "ENABLED"
        

class ScrollBarThumb(SwitchNode):
    
    def __init__(self, upBmp, downBmp, disabledBmp, endsExtent,
            orientation=Orientation.HORIZONTAL, minExtent=-1, 
            **kwargs):
      
        super(ScrollBarThumb, self).__init__(nodeMap=None, **kwargs)

        if orientation == Orientation.HORIZONTAL:
            StretchNode = HStretchNode
        else:
            StretchNode = VStretchNode
        self.__upNode = StretchNode(src=upBmp, endsExtent=endsExtent, minExtent=minExtent)
        self.__downNode = StretchNode(src=downBmp, endsExtent=endsExtent,
                minExtent=minExtent)
        self.__disabledNode = StretchNode(src=disabledBmp, endsExtent=endsExtent,
                minExtent=minExtent)

        self.setNodeMap({
            "UP": self.__upNode, 
            "DOWN": self.__downNode, 
            "DISABLED": self.__disabledNode
        })
        self.visibleid = "UP"
        

class SliderThumb(SwitchNode):

    def __init__(self, upBmp, downBmp, disabledBmp, **kwargs):
        upNode = avg.ImageNode()
        upNode.setBitmap(upBmp)
        downNode = avg.ImageNode()
        downNode.setBitmap(downBmp)
        disabledNode = avg.ImageNode()
        disabledNode.setBitmap(disabledBmp)
        nodeMap = {"UP": upNode, "DOWN": downNode, "DISABLED": disabledNode}
        super(SliderThumb, self).__init__(nodeMap=nodeMap, visibleid="UP", **kwargs)


class ProgressBar(avg.DivNode):

    def __init__(self, orientation, skinObj=skin.Skin.default, height=0, width=0, 
            range=(0.,1.), value=0.0, parent=None, **kwargs):
        super(ProgressBar, self).__init__(**kwargs)
        self.registerInstance(self, parent)
        if orientation == Orientation.HORIZONTAL:
            cfg = skinObj.defaultProgressBarCfg["horizontal"]
        else:
            cfg = skinObj.defaultProgressBarCfg["vertical"]
    
        self._orientation = orientation

        trackBmp = cfg["trackBmp"]
        self._trackNode = ScrollBarTrack(bmp=trackBmp, disabledBmp=trackBmp,
                endsExtent=cfg["trackEndsExtent"], orientation=self._orientation)
        self.appendChild(self._trackNode)

        thumbUpBmp = cfg["thumbUpBmp"]
        endsExtent=cfg["thumbEndsExtent"]

        self.__thumbNode = ScrollBarThumb(orientation=self._orientation, 
                upBmp=thumbUpBmp, downBmp=thumbUpBmp, disabledBmp=thumbUpBmp,
                endsExtent=endsExtent)
        self.appendChild(self.__thumbNode)

        self.__range = range
        self.__value = value

        if orientation == Orientation.HORIZONTAL:
            self.size = (width, trackBmp.getSize().y)
        else:
            self.size = (trackBmp.getSize().x, height)
        self._positionNodes()

    def getRange(self):
        return self.__range

    def setRange(self, range):
        self.__range = (float(range[0]), float(range[1]))
        self._positionNodes()
    range = property(getRange, setRange)

    def getValue(self):
        return self.__value

    def setValue(self, value):
        self._positionNodes(value)
    value = property(getValue, setValue)

    def _positionNodes(self, newValue=None):
        if newValue is not None:
            self.__value = float(newValue)
        if self.__value < self.__range[0]:
            self.__value = self.__range[0]
        if self.__value > self.__range[1]:
            self.__value = self.__range[1]
        self._trackNode.size = self.size
                 
        effectiveRange = math.fabs(self.__range[1] - self.__range[0])
        normValue = ((self.__value-self.__range[0])/effectiveRange) 
        if self._orientation == Orientation.HORIZONTAL:
            self.__thumbNode.width = normValue*self.size.x
        else:
            self.__thumbNode.height = normValue*self.size.y
        

class SliderBase(avg.DivNode):

    THUMB_POS_CHANGED = avg.Publisher.genMessageID()
    PRESSED = avg.Publisher.genMessageID()
    RELEASED = avg.Publisher.genMessageID()

    def __init__(self, orientation, cfg, enabled=True, height=0, width=0, range=(0.,1.), 
            thumbPos=0.0, parent=None, **kwargs):
        super(SliderBase, self).__init__(**kwargs)
        self.registerInstance(self, parent)
        
        self.publish(SliderBase.THUMB_POS_CHANGED)
        self.publish(SliderBase.PRESSED)
        self.publish(SliderBase.RELEASED)

        self._orientation = orientation

        trackBmp = cfg["trackBmp"]
        trackDisabledBmp = cfg["trackDisabledBmp"]
        self._trackNode = ScrollBarTrack(bmp=trackBmp, disabledBmp=trackDisabledBmp, 
                endsExtent=cfg["trackEndsExtent"], orientation=self._orientation)
        self.appendChild(self._trackNode)

        self._initThumb(cfg)

        self._range = range
        self._thumbPos = thumbPos

        self.subscribe(self.SIZE_CHANGED, lambda newSize: self._positionNodes())
        if orientation == Orientation.HORIZONTAL:
            self.size = (width, trackBmp.getSize().y)
        else:
            self.size = (trackBmp.getSize().x, height)
        if not(enabled):
            self.setEnabled(False)

        self.__recognizer = gesture.DragRecognizer(self._thumbNode, friction=-1,
                    detectedHandler=self.__onDragStart, moveHandler=self.__onDrag, 
                    upHandler=self.__onUp)

    def getRange(self):
        return self._range

    def setRange(self, range):
        self._range = (float(range[0]), float(range[1]))
        self._positionNodes()

    # range[1] > range[0]: Reversed scrollbar.
    range = property(getRange, setRange)

    def getThumbPos(self):
        return self._thumbPos

    def setThumbPos(self, thumbPos):
        self._positionNodes(thumbPos)

    thumbPos = property(getThumbPos, setThumbPos)

    def getEnabled(self):
        return self._trackNode.visibleid != "DISABLED"

    def setEnabled(self, enabled):
        if enabled:
            if self._trackNode.visibleid == "DISABLED":
                self._trackNode.visibleid = "ENABLED"
                self._thumbNode.visibleid = "UP"
                self.__recognizer.enable(True)
        else:
            if self._trackNode.visibleid != "DISABLED":
                self._trackNode.visibleid = "DISABLED"
                self._thumbNode.visibleid = "DISABLED"
                self.__recognizer.enable(False)

    enabled = property(getEnabled, setEnabled)

    def _positionNodes(self, newSliderPos=None):
        if newSliderPos is not None:
            self._thumbPos = float(newSliderPos)
        self._trackNode.size = self.size
                 
        self._constrainSliderPos()

        pixelRange = self._getScrollRangeInPixels()
        if self._getSliderRange() == 0:
            thumbPixelPos = 0
        else:
            thumbPixelPos = (((self._thumbPos-self._range[0])/self._getSliderRange())*
                    pixelRange)
        if self._orientation == Orientation.HORIZONTAL:
            self._thumbNode.x = thumbPixelPos
        else:
            self._thumbNode.y = thumbPixelPos

    def __onDragStart(self):
        self._thumbNode.visibleid = "DOWN"
        self.__dragStartPos = self._thumbPos
        self.notifySubscribers(Slider.PRESSED, [])

    def __onDrag(self, offset):
        pixelRange = self._getScrollRangeInPixels()
        if pixelRange == 0:
            normalizedOffset = 0
        else:
            if self._orientation == Orientation.HORIZONTAL:
                normalizedOffset = offset.x/pixelRange
            else:
                normalizedOffset = offset.y/pixelRange
        oldThumbPos = self._thumbPos
        self._positionNodes(self.__dragStartPos + normalizedOffset*self._getSliderRange())
        if self._thumbPos != oldThumbPos:
            self.notifySubscribers(Slider.THUMB_POS_CHANGED, [self._thumbPos])

    def __onUp(self, offset):
        self.__onDrag(offset)
        self._thumbNode.visibleid = "UP"
        self.notifySubscribers(Slider.RELEASED, [])


class Slider(SliderBase):

    def __init__(self, orientation=Orientation.HORIZONTAL, skinObj=skin.Skin.default,
            **kwargs):
        if orientation == Orientation.HORIZONTAL:
            cfg = skinObj.defaultSliderCfg["horizontal"]
        else:
            cfg = skinObj.defaultSliderCfg["vertical"]
        super(Slider, self).__init__(orientation, cfg, **kwargs)
        
    def _initThumb(self, cfg):
        thumbUpBmp = cfg["thumbUpBmp"]
        thumbDownBmp = skin.getBmpFromCfg(cfg, "thumbDownBmp", "thumbUpBmp")
        thumbDisabledBmp = skin.getBmpFromCfg(cfg, "thumbDisabledBmp", "thumbUpBmp")
        self._thumbNode = SliderThumb(upBmp=thumbUpBmp, downBmp=thumbDownBmp,
                disabledBmp=thumbDisabledBmp)
        self.appendChild(self._thumbNode)
 
    def _getScrollRangeInPixels(self):
        if self._orientation == Orientation.HORIZONTAL:
            return self.size.x - self._thumbNode.size.x
        else:
            return self.size.y - self._thumbNode.size.y

    def _getSliderRange(self):
        return self._range[1] - self._range[0]

    def _constrainSliderPos(self):
        rangeMin = min(self._range[0], self._range[1])
        rangeMax = max(self._range[0], self._range[1])
        self._thumbPos = max(rangeMin, self._thumbPos)
        self._thumbPos = min(rangeMax, self._thumbPos)


class ScrollBar(SliderBase):
   
    def __init__(self,  orientation=Orientation.HORIZONTAL, skinObj=skin.Skin.default,
            thumbExtent=0.1, **kwargs):
        self.__thumbExtent = thumbExtent
        if orientation == Orientation.HORIZONTAL:
            cfg = skinObj.defaultScrollBarCfg["horizontal"]
        else:
            cfg = skinObj.defaultScrollBarCfg["vertical"]
        super(ScrollBar, self).__init__(orientation=orientation, cfg=cfg, **kwargs)
        
    def _initThumb(self, cfg):
        thumbUpBmp = cfg["thumbUpBmp"]
        thumbDownBmp = skin.getBmpFromCfg(cfg, "thumbDownBmp", "thumbUpBmp")
        thumbDisabledBmp = skin.getBmpFromCfg(cfg, "thumbDisabledBmp", "thumbUpBmp")
        endsExtent=cfg["thumbEndsExtent"]

        self._thumbNode = ScrollBarThumb(orientation=self._orientation, 
                upBmp=thumbUpBmp, downBmp=thumbDownBmp, 
                disabledBmp=thumbDisabledBmp, endsExtent=endsExtent)
        self.appendChild(self._thumbNode)

    def getThumbExtent(self):
        return self.__thumbExtent

    def setThumbExtent(self, thumbExtent):
        self.__thumbExtent = float(thumbExtent)
        self._positionNodes()

    thumbExtent = property(getThumbExtent, setThumbExtent)

    def _getScrollRangeInPixels(self):
        if self._orientation == Orientation.HORIZONTAL:
            return self.size.x - self._thumbNode.width
        else:
            return self.size.y - self._thumbNode.height

    def _positionNodes(self, newSliderPos=None):
        effectiveRange = math.fabs(self._range[1] - self._range[0])
        if self._orientation == Orientation.HORIZONTAL:
            thumbExtent = (self.__thumbExtent/effectiveRange)*self.size.x
            self._thumbNode.width = thumbExtent
        else:
            thumbExtent = (self.__thumbExtent/effectiveRange)*self.size.y
            self._thumbNode.height = thumbExtent
        super(ScrollBar, self)._positionNodes(newSliderPos)
        if self._range[1] < self._range[0]:
            # Reversed (upside-down) scrollbar
            if self._orientation == Orientation.HORIZONTAL:
                self._thumbNode.x -= thumbExtent
            else:
                self._thumbNode.y -= thumbExtent

    def _getSliderRange(self):
        if self._range[1] > self._range[0]:
            return self._range[1] - self._range[0] - self.__thumbExtent
        else:
            return self._range[1] - self._range[0] + self.__thumbExtent

    def _constrainSliderPos(self):
        rangeMin = min(self._range[0], self._range[1])
        rangeMax = max(self._range[0], self._range[1])
        self._thumbPos = max(rangeMin, self._thumbPos)
        self._thumbPos = min(rangeMax-self.__thumbExtent, self._thumbPos)

