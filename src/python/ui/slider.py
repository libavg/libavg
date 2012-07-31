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

from libavg import avg, statemachine, player
from . import SwitchNode, Button
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


class ScrollBarBackground(SwitchNode):
    
    def __init__(self, enabledSrc, disabledSrc, endsExtent, 
            orientation=Orientation.HORIZONTAL, extent=-1, minExtent=-1, 
            **kwargs):
      
        super(ScrollBarBackground, self).__init__(nodeMap=None, **kwargs)
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


class ScrollBarSlider(SwitchNode):
    
    def __init__(self, upSrc, downSrc, disabledSrc, endsExtent, 
            orientation=Orientation.HORIZONTAL, extent=-1, minExtent=-1, 
            **kwargs):
      
        super(ScrollBarSlider, self).__init__(nodeMap=None, **kwargs)
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
        return self.__enabledNode.extent

    def setExtent(self, extent):
        self.__upNode.extent = extent
        self.__downNode.extent = extent
        self.__disabledNode.extent = extent
        self.size = self.__upNode.size

    extent = property(getExtent, setExtent)


class ScrollBar(avg.DivNode):
    
    def __init__(self, backgroundNode, sliderNode, enabled=True, range=(0.,1.), 
            sliderPos=0.0, sliderExtent=0.1,
            parent=None, **kwargs):
        super(ScrollBar, self).__init__(**kwargs)
        self.registerInstance(self, parent)

        self.__backgroundNode = backgroundNode
        self.appendChild(self.__backgroundNode)

        self.__sliderNode = sliderNode
        self.appendChild(self.__sliderNode)

        self.__range = range
        self.__sliderPos = sliderPos
        self.__sliderExtent = sliderExtent

        self.__positionNodes()

        self.__recognizer = gesture.DragRecognizer(self.__sliderNode, 
                    detectedHandler=self.__onDragStart, moveHandler=self.__onDrag, 
                    upHandler=self.__onDrag)

    def getRange(self):
        return self.__range

    def setRange(self, range):
        self.__range = range
        self.__positionNodes()

    range = property(getRange, setRange)

    def getSliderPos(self):
        return self.__sliderPos

    def setSliderPos(self, sliderPos):
        self.__sliderPos = sliderPos
        self.__positionNodes()

    sliderPos = property(getSliderPos, setSliderPos)

    def getSliderExtent(self):
        return self.__sliderExtent

    def setSliderExtent(self, sliderExtent):
        self.__sliderExtent = sliderExtent
        self.__positionNodes()

    sliderExtent = property(getSliderExtent, setSliderExtent)

    def __onDragStart(self, event):
        self.__sliderNode.visibleID = "DOWN"
        self.__dragStartPos = self.__sliderPos

    def __onDrag(self, event, offset):
        effectiveRange = self.__range[1] - self.__range[0]
        scaledOffset = (offset.x/(self.size.x*(1-self.__sliderExtent)))*effectiveRange
        self.__sliderPos = self.__dragStartPos + scaledOffset
        self.__positionNodes()
        if event.type == avg.CURSORUP:
            self.__sliderNode.visibleID = "UP"

    def __positionNodes(self):
        self.__backgroundNode.extent = self.width
        self.__constrainSliderPos()

        effectiveRange = self.__range[1] - self.__range[0]
        self.__sliderNode.x = ((self.__sliderPos/effectiveRange)*
                (self.size.x*(1-self.__sliderExtent)))
        self.size = self.__backgroundNode.size
        self.__sliderNode.extent = (self.__sliderExtent/effectiveRange)*self.size.x

    def __constrainSliderPos(self):
        self.__sliderPos = max(self.__range[0], self.__sliderPos)
        self.__sliderPos = min(self.__range[1], self.__sliderPos)


class BmpScrollBar(ScrollBar):

    def __init__(self, bkgdSrc, bkgdDisabledSrc, bkgdEndsExtent,
            sliderUpSrc, sliderDownSrc, sliderDisabledSrc, sliderEndsExtent,
            **kwargs):
        backgroundNode = ScrollBarBackground(enabledSrc=bkgdSrc, 
                disabledSrc=bkgdDisabledSrc, endsExtent=bkgdEndsExtent)
        sliderNode = ScrollBarSlider(upSrc=sliderUpSrc, downSrc=sliderDownSrc, 
                disabledSrc=sliderDisabledSrc, endsExtent=sliderEndsExtent)
        
        super(BmpScrollBar, self).__init__(backgroundNode=backgroundNode, 
                sliderNode=sliderNode, **kwargs)

