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
        self.__orientation = orientation

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
            self.__minExtent == minExtent
        
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
        if self.__orientation == Orientation.HORIZONTAL:
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
        if self.__orientation == Orientation.HORIZONTAL:
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

        self.setNodeMap({
            "ENABLED": self.__enabledNode, 
            "DISABLED": self.__disabledNode
        })
        self.visibleID = "ENABLED"
        
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
        self.visibleID = "UP"
        self.size = self.__upNode.size
        
    def getExtent(self):
        return self.__upNode.extent

    def setExtent(self, extent):
        self.__upNode.extent = extent
        self.__downNode.extent = extent
        self.__disabledNode.extent = extent
        self.size = self.__upNode.size

    extent = property(getExtent, setExtent)


class ScrollBar(avg.DivNode):
   
    THUMB_POS_CHANGED = avg.Node.LAST_MESSAGEID

    def __init__(self, trackNode, thumbNode, enabled=True, 
            orientation=Orientation.HORIZONTAL, range=(0.,1.), 
            thumbPos=0.0, thumbExtent=0.1, thumbPosChangedHandler=None,
            parent=None, **kwargs):
        super(ScrollBar, self).__init__(**kwargs)
        self.registerInstance(self, parent)
        
        self.__orientation = orientation

        self.__trackNode = trackNode
        self.appendChild(self.__trackNode)

        self.__thumbNode = thumbNode
        self.appendChild(self.__thumbNode)

        self.__range = range
        self.__thumbPos = thumbPos
        self.__thumbExtent = thumbExtent

        self.__positionNodes()

        self.__recognizer = gesture.DragRecognizer(self.__thumbNode, 
                    detectedHandler=self.__onDragStart, moveHandler=self.__onDrag, 
                    upHandler=self.__onDrag)
        self.publish(ScrollBar.THUMB_POS_CHANGED)
        if thumbPosChangedHandler:
            self.subscribe(ScrollBar.THUMB_POS_CHANGED, thumbPosChangedHandler)

        if not(enabled):
            self.setEnabled(False)

    def getRange(self):
        return self.__range

    def setRange(self, range):
        self.__range = (float(range[0]), float(range[1]))
        self.__positionNodes()

    range = property(getRange, setRange)

    def getThumbPos(self):
        return self.__thumbPos

    def setThumbPos(self, thumbPos):
        self.__positionNodes(thumbPos)

    thumbPos = property(getThumbPos, setThumbPos)

    def getThumbExtent(self):
        return self.__thumbExtent

    def setThumbExtent(self, thumbExtent):
        self.__thumbExtent = float(thumbExtent)
        self.__positionNodes()

    thumbExtent = property(getThumbExtent, setThumbExtent)

    def getEnabled(self):
        return self.__trackNode.visibleID != "DISABLED"

    def setEnabled(self, enabled):
        if enabled:
            if self.__trackNode.visibleID == "DISABLED":
                self.__trackNode.visibleID = "ENABLED"
                self.__thumbNode.visibleID = "UP"
                self.__recognizer.enable(True)
        else:
            if self.__trackNode.visibleID != "DISABLED":
                self.__trackNode.visibleID = "DISABLED"
                self.__thumbNode.visibleID = "DISABLED"
                self.__recognizer.enable(False)

    enabled = property(getEnabled, setEnabled)

    def __onDragStart(self, event):
        self.__thumbNode.visibleID = "DOWN"
        self.__dragStartPos = self.__thumbPos

    def __onDrag(self, event, offset):
        effectiveRange = self.__range[1] - self.__range[0]
        if self.__orientation == Orientation.HORIZONTAL:
            normalizedOffset = (offset.x/(self.size.x-self.__thumbNode.extent))
        else:
            normalizedOffset = (offset.y/(self.size.y-self.__thumbNode.extent))
        self.__positionNodes(self.__dragStartPos + normalizedOffset*effectiveRange)
        if event.type == avg.CURSORUP:
            self.__thumbNode.visibleID = "UP"

    def __positionNodes(self, newSliderPos=None):
        oldThumbPos = self.__thumbPos
        if newSliderPos is not None:
            self.__thumbPos = float(newSliderPos)
        if self.__orientation == Orientation.HORIZONTAL:
            self.__trackNode.extent = self.width
        else:
            self.__trackNode.extent = self.height
        self.__constrainSliderPos()
        if self.__thumbPos != oldThumbPos:
            self.notifySubscribers(ScrollBar.THUMB_POS_CHANGED, [self.__thumbPos])

        effectiveRange = self.__range[1] - self.__range[0]
        if self.__orientation == Orientation.HORIZONTAL:
            self.__thumbNode.extent = (self.__thumbExtent/effectiveRange)*self.size.x
            self.__thumbNode.x = ((self.__thumbPos/effectiveRange)*
                    (self.size.x-self.__thumbNode.extent))
        else:
            self.__thumbNode.y = ((self.__thumbPos/effectiveRange)*
                    (self.size.y-self.__thumbNode.extent))
            self.__thumbNode.extent = (self.__thumbExtent/effectiveRange)*self.size.y
        self.size = self.__trackNode.size

    def __constrainSliderPos(self):
        self.__thumbPos = max(self.__range[0], self.__thumbPos)
        self.__thumbPos = min(self.__range[1], self.__thumbPos)


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

