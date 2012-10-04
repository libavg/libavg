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


class StretchNodeBase(avg.DivNode):
    
    def __init__(self, parent=None, **kwargs):
        super(StretchNodeBase, self).__init__(**kwargs)
        self.registerInstance(self, parent)
        
    def getWidth(self):
        return self._baseWidth

    def setWidth(self, width):
        self._positionNodes(avg.Point2D(width, self._baseHeight))

    _baseWidth = avg.DivNode.width
    width = property(getWidth, setWidth)

    def getHeight(self):
        return self._baseHeight

    def setHeight(self, height):
        self._positionNodes(avg.Point2D(self._baseWidth, height))

    _baseHeight = avg.DivNode.height
    height = property(getHeight, setHeight)

    def getSize(self):
        return self._baseSize

    def setSize(self, size):
        self._positionNodes(size)

    _baseSize = avg.DivNode.size
    size = property(getSize, setSize)

    def _bmpFromSrc(self, src):
        if isinstance(src, basestring):
            return avg.Bitmap(src)
        elif isinstance(src, avg.Bitmap):
            return src
        else:
            raise RuntimeError("src must be a string or a Bitmap.")
        
    def _setSizeFromBmp(self, bmp):
        if self._baseSize.x == 0:
            self._baseWidth = bmp.width
        if self._baseSize.y == 0:
            self._baseHeight = bmp.height

    def _checkExtents(self, endsExtent, minExtent):
        if endsExtent < 0:
            raise RuntimeError(
                    "Illegal value for endsExtent: %i. Must be >= 0"%endsExtent)
        elif endsExtent == 0:
            # 1 has same effect as 0 - we just create one-pixel wide start and end images.
            endsExtent = 1

        if minExtent == -1:
            minExtent = endsExtent*2+1
        else:
            minExtent = minExtent
        return (endsExtent, minExtent)

    def _createImageNode(self, srcBmp, size):
        resultImg = avg.ImageNode(parent=self, size=size)
        resultImg.setBitmap(srcBmp)
        return resultImg

    def _renderImage(self, srcBmp, node, pos):
        canvas = player.createCanvas(id="stretch_canvas", size=node.size)
        img = avg.ImageNode(pos=pos, parent=canvas.getRootNode())
        img.setBitmap(srcBmp)
        canvas.render()
        node.setBitmap(canvas.screenshot())
        player.deleteCanvas("stretch_canvas")


class HStretchNode(StretchNodeBase):

    def __init__(self, src, endsExtent, minExtent=-1, **kwargs):
        super(HStretchNode, self).__init__(**kwargs)

        (self.__endsExtent, self.__minExtent) = self._checkExtents(endsExtent, minExtent)

        self.__bmp = self._bmpFromSrc(src)
        height = self.__bmp.getSize().y
        self.__startImg = self._createImageNode(self.__bmp, (self.__endsExtent, height))
        self.__centerImg = self._createImageNode(self.__bmp, (1, height))
        self.__endImg = self._createImageNode(self.__bmp, (self.__endsExtent, height))
        
        self._setSizeFromBmp(self.__bmp)
        self._positionNodes(self._baseSize)

        if player.isPlaying():
            self._renderImages()
        else:
            player.subscribe(avg.Player.PLAYBACK_START, self._renderImages)
    
    def _positionNodes(self, newSize):
        if newSize.x < self.__minExtent:
            newSize = avg.Point2D(self.__minExtent, newSize.y)

        self.__centerImg.x = self.__endsExtent
        self.__centerImg.width = newSize.x - self.__endsExtent*2
        self.__endImg.x = newSize.x - self.__endsExtent
        
        self._baseSize = newSize

    def _renderImages(self):
        self._renderImage(self.__bmp, self.__startImg, (0,0))
        self._renderImage(self.__bmp, self.__centerImg, (-self.__endsExtent,0))
        endOffset = self.__bmp.getSize().x - self.__endsExtent
        self._renderImage(self.__bmp, self.__endImg, (-endOffset,0))
    

class VStretchNode(StretchNodeBase):

    def __init__(self, src, endsExtent, minExtent=-1, **kwargs):
        super(VStretchNode, self).__init__(**kwargs)

        (self.__endsExtent, self.__minExtent) = self._checkExtents(endsExtent, minExtent)

        self.__bmp = self._bmpFromSrc(src)
        width = self.__bmp.getSize().x
        self.__startImg = self._createImageNode(self.__bmp, (width, self.__endsExtent))
        self.__centerImg = self._createImageNode(self.__bmp, (width, 1))
        self.__endImg = self._createImageNode(self.__bmp, (width, self.__endsExtent))
        
        self._setSizeFromBmp(self.__bmp)
        self._positionNodes(self._baseSize)

        if player.isPlaying():
            self._renderImages()
        else:
            player.subscribe(avg.Player.PLAYBACK_START, self._renderImages)
    
    def _positionNodes(self, newSize):
        if newSize.y < self.__minExtent:
            newSize = avg.Point2D(newSize.x, self.__minExtent)

        self.__centerImg.y = self.__endsExtent
        self.__centerImg.height = newSize.y - self.__endsExtent*2
        self.__endImg.y = newSize.y - self.__endsExtent
        
        self._baseSize = newSize

    def _renderImages(self):
        self._renderImage(self.__bmp, self.__startImg, (0,0))
        self._renderImage(self.__bmp, self.__centerImg, (0,-self.__endsExtent))
        endOffset = self.__bmp.getSize().x - self.__endsExtent
        self._renderImage(self.__bmp, self.__endImg, (0,-endOffset))


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
                    # Hack to support min. size in StretchNodes
                    self.__baseSize = node.size
