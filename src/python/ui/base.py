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


class SwitchNode(avg.DivNode):

    def __init__(self, nodeMap=None, visibleid=None, parent=None, **kwargs):
        super(SwitchNode, self).__init__(**kwargs)
        self.registerInstance(self, parent)
  
        self.__nodeMap = nodeMap
        if nodeMap:
            self.setNodeMap(nodeMap)
        if visibleid:
            self.setVisibleID(visibleid)

    def setNodeMap(self, nodeMap):
        self.__nodeMap = nodeMap
        for node in self.__nodeMap.itervalues():
            if node:
                # Only insert child if it hasn't been inserted yet.
                try:
                    self.indexOf(node)
                except RuntimeError:
                    self.appendChild(node)
        if self.size != (0,0):
            size = self.size
        else:
            key = list(self.__nodeMap.keys())[0]
            size = self.__nodeMap[key].size
        self.setSize(size)

    def getVisibleID(self):
        return self.__visibleid

    def setVisibleID(self, visibleid):
        if not (visibleid in self.__nodeMap):
            raise RuntimeError("'%s' is not a registered id." % visibleid)
        self.__visibleid = visibleid
        for node in self.__nodeMap.itervalues():
            node.active = False
        self.__nodeMap[visibleid].active = True

    visibleid = property(getVisibleID, setVisibleID)

    def getWidth(self):
        return self.__baseWidth

    def setWidth(self, width):
        self.__baseWidth = width
        self.__setChildSizes()

    __baseWidth = avg.DivNode.width
    width = property(getWidth, setWidth)

    def getHeight(self):
        return self.__baseHeight

    def setHeight(self, height):
        self.__baseHeight = height
        self.__setChildSizes()

    __baseHeight = avg.DivNode.height
    height = property(getHeight, setHeight)

    def getSize(self):
        return self.__baseSize

    def setSize(self, size):
        self.__baseSize = size
        self.__setChildSizes()
        
    __baseSize = avg.DivNode.size
    size = property(getSize, setSize)   

    def __setChildSizes(self):
        if self.__nodeMap:
            for node in self.__nodeMap.itervalues():
                if node:
                    node.size = self.__baseSize
                    # Hack to support min. size in AccordionNodes
                    self.__baseSize = node.size
