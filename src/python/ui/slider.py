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
import gesture

# TODO:
# - assumes horizontal orientation for now.

class AccordionNode(avg.DivNode):
    
    def __init__(self, src, endswidth, orientation=0, minsize=-1, 
            parent=None, **kwargs):
        super(AccordionNode, self).__init__(**kwargs)
        self.registerInstance(self, parent)
       
        # Load src image, build startNode, centerNode, endNode using canvas.
        bmp = avg.Bitmap(src)
        self.__startImg = self.__createImageNode(bmp, 0, endswidth)
        self.__centerImg = self.__createImageNode(bmp, endswidth, 1)
        endOffset = bmp.getSize().x - endswidth
        self.__endImg = self.__createImageNode(bmp, endOffset, endswidth)
        
        self.__endsWidth = endswidth
        if minsize == -1:
            self.__minSize = self.__endsWidth*2+1
        else:
            self.__minSize == minsize
        
        if self.__divSize.x == 100000:
            # Default size, set to min. size.
            self.__divSize = (self.__minSize, bmp.getSize().y)

        self.__positionNodes(self.__divSize.x)

    def getSize(self):
        return self.__divSize

    def setSize(self, size):
        if size.x < self.__minSize:
            size = avg.Point2D(self.__minSize, size.y)
        self.__positionNodes(size.x)
        self.__divSize = size

    __divSize = avg.DivNode.size
    size = property(getSize, setSize)

    def getWidth(self):
        return self.__divSize.x

    def setWidth(self, width):
        if width < self.__minSize:
            width = self.__minSize
        self.__positionNodes(width)
        self.__divSize = (width, self.__divSize.y)

    width = property(getWidth, setWidth)

    def __positionNodes(self, width):
        self.__startImg.x = 0
        self.__centerImg.x = self.__endsWidth
        self.__centerImg.width = width - self.__endsWidth*2
        self.__endImg.x = width - self.__endsWidth

    def __createImageNode(self, srcBmp, offset, width):
        bmpSize = srcBmp.getSize()
        endsSize = avg.Point2D(width, bmpSize.y)
        canvas = player.createCanvas(id="accordion_canvas", size=endsSize)
        img = avg.ImageNode(pos=(-offset,0), parent=canvas.getRootNode())
        img.setBitmap(srcBmp)
        canvas.render()
        resultImg = avg.ImageNode(parent=self)
        resultImg.setBitmap(canvas.screenshot())
        player.deleteCanvas("accordion_canvas")
        return resultImg

