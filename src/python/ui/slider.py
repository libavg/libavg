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


class Orientation():
    VERTICAL = 0
    HORIZONTAL = 1


class AccordionNode(avg.DivNode):
    
    def __init__(self, src, endsExtent, orientation=Orientation.HORIZONTAL, extent=-1,
            minExtent=-1, parent=None, **kwargs):
        super(AccordionNode, self).__init__(**kwargs)
        self.registerInstance(self, parent)
       
        bmp = avg.Bitmap(src)
        self._orientation = orientation

        # XXX: Check if bmp is smaller than min size

        self.__startImg = self.__createImageNode(bmp, 0, endsExtent)
        self.__centerImg = self.__createImageNode(bmp, endsExtent, 1)
        if orientation == Orientation.HORIZONTAL:
            endOffset = bmp.getSize().x - endsExtent
        else:
            endOffset = bmp.getSize().y - endsExtent
        self.__endImg = self.__createImageNode(bmp, endOffset, endsExtent)
        
        self.__endsExtent = endsExtent
        if minExtent == -1:
            self.__minExtent = self.__endsExtent*2+1
        else:
            self.__minExtent = minExtent
        
        if extent == -1:
            self.__extent = self.__minExtent
        else:
            self.__extent = extent
        self.__positionNodes(self.__extent)

    def getExtent(self):
        return self.__extent

    def setExtent(self, extent):
        if extent < self.__minExtent:
            extent = self.__minExtent
        self.__positionNodes(extent)

    extent = property(getExtent, setExtent)

    def __positionNodes(self, extent):
        self.__extent = extent
        if self._orientation == Orientation.HORIZONTAL:
            self.__centerImg.x = self.__endsExtent
            self.__centerImg.width = extent - self.__endsExtent*2
            self.__endImg.x = extent - self.__endsExtent
            self.size = (extent, self.__startImg.height)
        else:
            self.__centerImg.y = self.__endsExtent
            self.__centerImg.height = extent - self.__endsExtent*2
            self.__endImg.y = extent - self.__endsExtent
            self.size = (self.__startImg.width, extent)

    def __createImageNode(self, srcBmp, offset, extent):
        bmpSize = srcBmp.getSize()
        if self._orientation == Orientation.HORIZONTAL:
            pos = (-offset,0)
            endsSize = avg.Point2D(extent, bmpSize.y)
        else:
            pos = (0, -offset)
            endsSize = avg.Point2D(bmpSize.x, extent)
        canvas = player.createCanvas(id="accordion_canvas", size=endsSize)
        img = avg.ImageNode(pos=pos, parent=canvas.getRootNode())
        img.setBitmap(srcBmp)
        canvas.render()
        resultImg = avg.ImageNode(parent=self)
        resultImg.setBitmap(canvas.screenshot())
        player.deleteCanvas("accordion_canvas")
        return resultImg


class ScrollBarTrack(SwitchNode):
    
    def __init__(self, enabledSrc, disabledSrc, endsExtent, 
            orientation=Orientation.HORIZONTAL, extent=-1, minExtent=-1, 
            **kwargs):
      
        super(ScrollBarTrack, self).__init__(nodeMap=None, **kwargs)
        self.__enabledNode = AccordionNode(src=enabledSrc, endsExtent=endsExtent,
                orientation=orientation, extent=extent, minExtent=minExtent,
                parent=self)
        self.__disabledNode = AccordionNode(src=disabledSrc, endsExtent=endsExtent,
                orientation=orientation, extent=extent, minExtent=minExtent,
                parent=self)
        
        self.size = self.__enabledNode.size

        self.setNodeMap({
            "ENABLED": self.__enabledNode, 
            "DISABLED": self.__disabledNode
        })
        self.visibleid = "ENABLED"
        
    def getExtent(self):
        return self.__enabledNode.extent

    def setExtent(self, extent):
        self.__enabledNode.extent = extent
        self.__disabledNode.extent = extent
        self.size = self.__enabledNode.size

    extent = property(getExtent, setExtent)


class ScrollBarThumb(SwitchNode):
    
    def __init__(self, upSrc, downSrc, disabledSrc, endsExtent, 
            orientation=Orientation.HORIZONTAL, extent=-1, minExtent=-1, 
            **kwargs):
      
        super(ScrollBarThumb, self).__init__(nodeMap=None, **kwargs)
        self.__upNode = AccordionNode(src=upSrc, endsExtent=endsExtent,
                orientation=orientation, extent=extent, minExtent=minExtent)
        self.__downNode = AccordionNode(src=downSrc, endsExtent=endsExtent,
                orientation=orientation, extent=extent, minExtent=minExtent)
        self.__disabledNode = AccordionNode(src=disabledSrc, endsExtent=endsExtent,
                orientation=orientation, extent=extent, minExtent=minExtent)

        self.setNodeMap({
            "UP": self.__upNode, 
            "DOWN": self.__downNode, 
            "DISABLED": self.__disabledNode
        })
        self.visibleid = "UP"
        self.size = self.__upNode.size
        
    def getExtent(self):
        return self.__upNode.extent

    def setExtent(self, extent):
        self.__upNode.extent = extent
        self.__downNode.extent = extent
        self.__disabledNode.extent = extent
        self.size = self.__upNode.size

    extent = property(getExtent, setExtent)


class Slider(avg.DivNode):

    THUMB_POS_CHANGED = avg.Node.LAST_MESSAGEID

    def __init__(self, trackNode, thumbNode, enabled=True, 
            orientation=Orientation.HORIZONTAL, range=(0.,1.), thumbpos=0.0, 
            thumbPosChangedHandler=None, parent=None, **kwargs):
        super(Slider, self).__init__(**kwargs)
        self.registerInstance(self, parent)
        
        self._orientation = orientation

        self._trackNode = trackNode
        self.appendChild(self._trackNode)

        self._thumbNode = thumbNode
        self.appendChild(self._thumbNode)

        self._range = range
        self._thumbPos = thumbpos

        self._positionNodes()

        self.__recognizer = gesture.DragRecognizer(self._thumbNode, 
                    detectedHandler=self.__onDragStart, moveHandler=self.__onDrag, 
                    upHandler=self.__onDrag)
        self.publish(Slider.THUMB_POS_CHANGED)
        if thumbPosChangedHandler:
            self.subscribe(Slider.THUMB_POS_CHANGED, thumbPosChangedHandler)

        if not(enabled):
            self.setEnabled(False)

    def getExtent(self):
        if self._orientation == Orientation.HORIZONTAL:
            return self.width
        else:
            return self.height

    def setExtent(self, extent):
        if self._orientation == Orientation.HORIZONTAL:
            self.width = extent
        else:
            self.height = extent
        self._positionNodes()

    extent = property(getExtent, setExtent)

    def getRange(self):
        return self._range

    def setRange(self, range):
        self._range = (float(range[0]), float(range[1]))
        self._positionNodes()

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

    def __onDragStart(self, event):
        self._thumbNode.visibleid = "DOWN"
        self.__dragStartPos = self._thumbPos

    def __onDrag(self, event, offset):
        pixelRange = self._getScrollRangeInPixels()
        if pixelRange == 0:
            normalizedOffset = 0
        else:
            if self._orientation == Orientation.HORIZONTAL:
                normalizedOffset = offset.x/pixelRange
            else:
                normalizedOffset = offset.y/pixelRange
        self._positionNodes(self.__dragStartPos + normalizedOffset*self._getSliderRange())
        if event.type == avg.CURSORUP:
            self._thumbNode.visibleid = "UP"

    def _getScrollRangeInPixels(self):
        if self._orientation == Orientation.HORIZONTAL:
            return self.size.x
        else:
            return self.size.y

    def _positionNodes(self, newSliderPos=None):
        oldThumbPos = self._thumbPos
        if newSliderPos is not None:
            self._thumbPos = float(newSliderPos)
        if self._orientation == Orientation.HORIZONTAL:
            self._trackNode.extent = self.width
        else:
            self._trackNode.extent = self.height
        self._constrainSliderPos()
        if self._thumbPos != oldThumbPos:
            self.notifySubscribers(ScrollBar.THUMB_POS_CHANGED, [self._thumbPos])

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
#        print "--------"
#        print "range: ", self._range
#        print "thumbPos: ", self._thumbPos
#        print "self.size: ", self.size
#        print "_thumbNode.pos: ", self._thumbNode.pos
#        print "effectiveRange: ", effectiveRange
#        print "pixelRange: ", pixelRange

#        self.size = self._trackNode.size

    def _getSliderRange(self):
        return self._range[1] - self._range[0]

    def _constrainSliderPos(self):
        self._thumbPos = max(self._range[0], self._thumbPos)
        self._thumbPos = min(self._range[1], self._thumbPos)


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
            return self.size.x - self._thumbNode.extent
        else:
            return self.size.y - self._thumbNode.extent

    def _positionNodes(self, newSliderPos=None):
        effectiveRange = self._range[1] - self._range[0]
        if self._orientation == Orientation.HORIZONTAL:
            self._thumbNode.extent = (self.__thumbExtent/effectiveRange)*self.size.x
        else:
            self._thumbNode.extent = (self.__thumbExtent/effectiveRange)*self.size.y
        super(ScrollBar, self)._positionNodes(newSliderPos)
    
    def _getSliderRange(self):
        return self._range[1] - self._range[0] - self.__thumbExtent

    def _constrainSliderPos(self):
        self._thumbPos = max(self._range[0], self._thumbPos)
        self._thumbPos = min(self._range[1]-self.__thumbExtent, self._thumbPos)


class BmpScrollBar(ScrollBar):

    def __init__(self, trackSrc, trackDisabledSrc, trackEndsExtent,
            thumbUpSrc, thumbDownSrc, thumbDisabledSrc, thumbEndsExtent,
            orientation=Orientation.HORIZONTAL, **kwargs):
        trackNode = ScrollBarTrack(orientation=orientation, enabledSrc=trackSrc, 
                disabledSrc=trackDisabledSrc, endsExtent=trackEndsExtent)
        thumbNode = ScrollBarThumb(orientation=orientation, 
                upSrc=thumbUpSrc, downSrc=thumbDownSrc, 
                disabledSrc=thumbDisabledSrc, endsExtent=thumbEndsExtent)
        
        super(BmpScrollBar, self).__init__(trackNode=trackNode, 
                orientation=orientation, thumbNode=thumbNode, **kwargs)

