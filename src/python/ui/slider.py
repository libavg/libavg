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

from libavg import avg, player
from . import SwitchNode
import gesture

import math

class Orientation():
    VERTICAL = 0
    HORIZONTAL = 1


class AccordionNode(avg.DivNode):
    
    def __init__(self, src, endsExtent, orientation=Orientation.HORIZONTAL,
            minExtent=-1, parent=None, **kwargs):
        super(AccordionNode, self).__init__(**kwargs)
        self.registerInstance(self, parent)
        if endsExtent < 0:
            raise RuntimeError(
                    "Illegal value for endsExtent: %i. Must be >= 0"%endsExtent)
        elif endsExtent == 0:
            # 1 has same effect as 0 - we just create one-pixel wide start and end images.
            endsExtent = 1

        self.__bmp = avg.Bitmap(src)
        self._orientation = orientation

        # XXX: Check if bmp is smaller than min size

        self.__startImg = self.__createImageNode(self.__bmp, endsExtent)
        self.__centerImg = self.__createImageNode(self.__bmp, 1)
        self.__endImg = self.__createImageNode(self.__bmp, endsExtent)
        
        self.__endsExtent = endsExtent
        if minExtent == -1:
            self.__minExtent = self.__endsExtent*2+1
        else:
            self.__minExtent = minExtent
        
        if orientation == Orientation.HORIZONTAL:
            if self.__baseSize.x != 0:
                self.__baseWidth = self.__baseSize.x
            if self.__baseSize.y == 0:
                self.__baseHeight = self.__startImg.height
        else:
            if self.__baseSize.y != 0:
                self.__baseHeight = self.__baseSize.y
            if self.__baseSize.x == 0:
                self.__baseWidth = self.__startImg.width
        self.__positionNodes(self.__baseSize)

        if player.isPlaying():
            self.__renderImages()
        else:
            player.subscribe(avg.Player.PLAYBACK_START, self.__renderImages)

    def getWidth(self):
        return self.__baseWidth

    def setWidth(self, width):
        self.__positionNodes(avg.Point2D(width, self.__baseHeight))

    __baseWidth = avg.DivNode.width
    width = property(getWidth, setWidth)

    def getHeight(self):
        return self.__baseHeight

    def setHeight(self, height):
        self.__positionNodes(avg.Point2D(self.__baseWidth, height))

    __baseHeight = avg.DivNode.height
    height = property(getHeight, setHeight)

    def getSize(self):
        return self.__baseSize

    def setSize(self, size):
        self.__positionNodes(size)

    __baseSize = avg.DivNode.size
    size = property(getSize, setSize)

    def __positionNodes(self, newSize):
        if self._orientation == Orientation.HORIZONTAL and newSize.x < self.__minExtent:
                newSize = avg.Point2D(self.__minExtent, newSize.y)
        elif self._orientation == Orientation.VERTICAL and newSize.y < self.__minExtent:
                newSize = avg.Point2D(newSize.x, self.__minExtent)
        self.__baseSize = newSize

        if self._orientation == Orientation.HORIZONTAL:
            self.__centerImg.x = self.__endsExtent
            self.__centerImg.width = newSize.x - self.__endsExtent*2
            self.__endImg.x = newSize.x - self.__endsExtent
        else:
            self.__centerImg.y = self.__endsExtent
            self.__centerImg.height = newSize.y - self.__endsExtent*2
            self.__endImg.y = newSize.y - self.__endsExtent

    def __createImageNode(self, srcBmp, extent):
        bmpSize = srcBmp.getSize()
        if self._orientation == Orientation.HORIZONTAL:
            endsSize = avg.Point2D(extent, bmpSize.y)
        else:
            endsSize = avg.Point2D(bmpSize.x, extent)
        resultImg = avg.ImageNode(parent=self, size=endsSize)
        resultImg.setBitmap(srcBmp)
        return resultImg

    def __renderImages(self):
        self.__renderImage(self.__bmp, self.__startImg, 0)
        self.__renderImage(self.__bmp, self.__centerImg, self.__endsExtent)
        if self._orientation == Orientation.HORIZONTAL:
            endOffset = self.__bmp.getSize().x - self.__endsExtent
        else:
            endOffset = self.__bmp.getSize().y - self.__endsExtent
        self.__renderImage(self.__bmp, self.__endImg, endOffset)

    def __renderImage(self, srcBmp, node, offset):
        if self._orientation == Orientation.HORIZONTAL:
            pos = (-offset,0)
        else:
            pos = (0, -offset)
        canvas = player.createCanvas(id="accordion_canvas", size=node.size)
        img = avg.ImageNode(pos=pos, parent=canvas.getRootNode())
        img.setBitmap(srcBmp)
        canvas.render()
        node.setBitmap(canvas.screenshot())
        player.deleteCanvas("accordion_canvas")


class ScrollBarTrack(SwitchNode):
    
    def __init__(self, enabledSrc, endsExtent, disabledSrc=None, 
            orientation=Orientation.HORIZONTAL, minExtent=-1, 
            **kwargs):
      
        super(ScrollBarTrack, self).__init__(nodeMap=None, **kwargs)
        self.__enabledNode = AccordionNode(src=enabledSrc, endsExtent=endsExtent,
                orientation=orientation, minExtent=minExtent,
                parent=self)
        if disabledSrc == None:
            disabledSrc = enabledSrc
        self.__disabledNode = AccordionNode(src=disabledSrc, endsExtent=endsExtent,
                orientation=orientation, minExtent=minExtent,
                parent=self)
        
        self.setNodeMap({
            "ENABLED": self.__enabledNode, 
            "DISABLED": self.__disabledNode
        })
        self.visibleid = "ENABLED"
        

class ScrollBarThumb(SwitchNode):
    
    def __init__(self, upSrc, downSrc, endsExtent, disabledSrc=None, 
            orientation=Orientation.HORIZONTAL, minExtent=-1, 
            **kwargs):
      
        super(ScrollBarThumb, self).__init__(nodeMap=None, **kwargs)
        self.__upNode = AccordionNode(src=upSrc, endsExtent=endsExtent,
                orientation=orientation, minExtent=minExtent)
        self.__downNode = AccordionNode(src=downSrc, endsExtent=endsExtent,
                orientation=orientation, minExtent=minExtent)
        if disabledSrc == None:
            disabledSrc = upSrc
        self.__disabledNode = AccordionNode(src=disabledSrc, endsExtent=endsExtent,
                orientation=orientation, minExtent=minExtent)

        self.setNodeMap({
            "UP": self.__upNode, 
            "DOWN": self.__downNode, 
            "DISABLED": self.__disabledNode
        })
        self.visibleid = "UP"
        

class SliderThumb(SwitchNode):

    def __init__(self, upSrc, downSrc, disabledSrc=None, **kwargs):
        upNode = avg.ImageNode(href=upSrc)
        if disabledSrc == None:
            disabledSrc = upSrc
        nodeMap = {
            "UP": upNode,
            "DOWN": avg.ImageNode(href=downSrc),
            "DISABLED": avg.ImageNode(href=disabledSrc)
        }
        super(SliderThumb, self).__init__(nodeMap=nodeMap, visibleid="UP", **kwargs)


class Slider(avg.DivNode):

    THUMB_POS_CHANGED = avg.Publisher.genMessageID()
    PRESSED = avg.Publisher.genMessageID()
    RELEASED = avg.Publisher.genMessageID()

    def __init__(self, trackNode, thumbNode,
            enabled=True, orientation=Orientation.HORIZONTAL, range=(0.,1.), 
            thumbpos=0.0, parent=None, **kwargs):
        super(Slider, self).__init__(**kwargs)
        self.registerInstance(self, parent)
        
        self.publish(Slider.THUMB_POS_CHANGED)
        self.publish(Slider.PRESSED)
        self.publish(Slider.RELEASED)

        self._orientation = orientation

        self._trackNode = trackNode
        self.appendChild(self._trackNode)

        self._thumbNode = thumbNode
        self.appendChild(self._thumbNode)

        self._range = range
        self._thumbPos = thumbpos

        self._positionNodes()

        self.__recognizer = gesture.DragRecognizer(self._thumbNode, friction=-1,
                    detectedHandler=self.__onDragStart, moveHandler=self.__onDrag, 
                    upHandler=self.__onUp)

        if not(enabled):
            self.setEnabled(False)

    def getWidth(self):
        return self.__baseWidth

    def setWidth(self, width):
        self.__baseWidth = width
        self._positionNodes()

    __baseWidth = avg.DivNode.width
    width = property(getWidth, setWidth)

    def getHeight(self):
        return self.__baseHeight

    def setHeight(self, height):
        self.__baseHeight = height
        self._positionNodes()

    __baseHeight = avg.DivNode.height
    height = property(getHeight, setHeight)

    def getSize(self):
        return self.__baseSize

    def setSize(self, size):
        self.__baseSize = size
        self._positionNodes()

    __baseSize = avg.DivNode.size
    size = property(getSize, setSize)

    def getRange(self):
        return self._range

    def setRange(self, range):
        self._range = (float(range[0]), float(range[1]))
        self._positionNodes()

    # range[1] > range[0]: Reversed scrollbar.
    range = property(getRange, setRange)

    def getThumbPos(self):
        return self._thumbPos

    def setThumbPos(self, thumbpos):
        self._positionNodes(thumbpos)

    thumbpos = property(getThumbPos, setThumbPos)

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
        self._positionNodes(self.__dragStartPos + normalizedOffset*self._getSliderRange())

    def __onUp(self, offset):
        self.__onDrag(offset)
        self._thumbNode.visibleid = "UP"
        self.notifySubscribers(Slider.RELEASED, [])

    def _getScrollRangeInPixels(self):
        if self._orientation == Orientation.HORIZONTAL:
            return self.size.x - self._thumbNode.size.x
        else:
            return self.size.y - self._thumbNode.size.y

    def _positionNodes(self, newSliderPos=None):
        oldThumbPos = self._thumbPos
        if newSliderPos is not None:
            self._thumbPos = float(newSliderPos)
        self._trackNode.size = self.size
                 
        self._constrainSliderPos()
        if self._thumbPos != oldThumbPos:
            self.notifySubscribers(Slider.THUMB_POS_CHANGED, [self._thumbPos])

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

    def _getSliderRange(self):
        return self._range[1] - self._range[0]

    def _constrainSliderPos(self):
        rangeMin = min(self._range[0], self._range[1])
        rangeMax = max(self._range[0], self._range[1])
        self._thumbPos = max(rangeMin, self._thumbPos)
        self._thumbPos = min(rangeMax, self._thumbPos)


class BmpSlider(Slider):

    def __init__(self, trackSrc, trackEndsExtent, 
            thumbUpSrc, thumbDownSrc, trackDisabledSrc=None, thumbDisabledSrc=None,
            orientation=Orientation.HORIZONTAL, **kwargs):
        trackNode = ScrollBarTrack(orientation=orientation, enabledSrc=trackSrc, 
                disabledSrc=trackDisabledSrc, endsExtent=trackEndsExtent)
        thumbNode = SliderThumb(upSrc=thumbUpSrc, 
                downSrc=thumbDownSrc, disabledSrc=thumbDisabledSrc)

        super(BmpSlider, self).__init__(trackNode=trackNode,
                orientation=orientation, thumbNode=thumbNode, **kwargs)
    
    def __calcTrackMargin(self, orientation, margin, thumbSize):
        if orientation == Orientation.HORIZONTAL:
            if margin[0] == 0 and margin[2] == 0:
                margin = (thumbSize.x/2, margin[1], thumbSize.x/2, margin[3])
        else:
            if margin[1] == 0 and margin[3] == 0:
                margin = (margin[0], thumbSize.y/2, margin[2], thumbSize.y/2)
        return margin

class ScrollBar(Slider):
   
    def __init__(self, thumbextent=0.1, **kwargs):
        self.__thumbExtent = thumbextent
        super(ScrollBar, self).__init__(**kwargs)

    def getThumbExtent(self):
        return self.__thumbExtent

    def setThumbExtent(self, thumbExtent):
        self.__thumbExtent = float(thumbExtent)
        self._positionNodes()

    thumbextent = property(getThumbExtent, setThumbExtent)

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


class BmpScrollBar(ScrollBar):

    def __init__(self, trackSrc, trackEndsExtent, thumbUpSrc, thumbDownSrc, 
            thumbEndsExtent, thumbDisabledSrc=None, trackDisabledSrc=None, 
            orientation=Orientation.HORIZONTAL, **kwargs):
        trackNode = ScrollBarTrack(orientation=orientation, enabledSrc=trackSrc, 
                disabledSrc=trackDisabledSrc, endsExtent=trackEndsExtent)
        thumbNode = ScrollBarThumb(orientation=orientation, 
                upSrc=thumbUpSrc, downSrc=thumbDownSrc, 
                disabledSrc=thumbDisabledSrc, endsExtent=thumbEndsExtent)
        
        super(BmpScrollBar, self).__init__(trackNode=trackNode, 
                orientation=orientation, thumbNode=thumbNode, **kwargs)

