#!/usr/bin/env python
# -*- coding: utf-8 -*-

# libavg - Media Playback Engine.
# Copyright (C) 2003-2014 Ulrich von Zadow
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

import os
import xml.dom.minidom as dom

from libavg import avg, player


class SpriteInfo(object):
    def __init__(self, xmlNode):
        self.pos = avg.Point2D(int(xmlNode.attributes['x'].value),
                int(xmlNode.attributes['y'].value))
        self.size = avg.Point2D(int(xmlNode.attributes['width'].value),
                int(xmlNode.attributes['height'].value))


class Spritesheet(object):
    def __init__(self, dataFName):
        self.__dataFName = dataFName
        self.__spriteInfos = {}

        xml_tree = dom.parse(dataFName)
        self.__textureFName = xml_tree.firstChild.attributes["imagePath"].value
        absTextureFName = os.path.join(os.path.dirname(dataFName), self.__textureFName)
        # Load bitmap and discard it just to see if the file exists.
        avg.Bitmap(absTextureFName)

        for entry in xml_tree.firstChild.childNodes:
            if entry.nodeName == "SubTexture":
                spriteInfo = SpriteInfo(entry)
                name = entry.attributes['name'].value
                name = name.rstrip('0123456789')

                if name not in self.__spriteInfos.keys():
                    self.__spriteInfos[name] = []
                self.__spriteInfos[name].append(spriteInfo)

    @property
    def textureName(self):
        return self.__textureFName

    def getSpriteInfos(self, name):
        return self.__spriteInfos[name]


class Sprite(avg.DivNode):
    def __init__(self, spritesheet, spriteName, parent=None, **kwargs):
        super(Sprite, self).__init__(**kwargs)
        self.registerInstance(self, parent)
        self.crop = True
        self.__spritesheet = spritesheet
        self._info = spritesheet.getSpriteInfos(spriteName)

        self.__imageNode = avg.ImageNode(href=self.__spritesheet.textureName, parent=self)
        self._selectSprite(0)

    def _selectSprite(self, i):
        info = self._info[int(i)]
        self.__imageNode.pos = -info.pos
        self.size = info.size


class AnimatedSprite(Sprite):
    END_OF_ANIMATION = avg.Publisher.genMessageID()

    def __init__(self, spritesheet, spriteName, loop=False, fps=30, parent=None,
            **kwargs):
        super(AnimatedSprite, self).__init__(spritesheet, spriteName, parent, **kwargs)
        self.publish(self.END_OF_ANIMATION)

        self.__fps = fps
        self.__curFrameNum = 0
        self._selectSprite(0)
        self.__loop = loop
        self.__playing = False
        self.__frameHandlerID = None

        self.subscribe(avg.Node.KILLED, self.__onDelete)

    def __onDelete(self):
        if self.__frameHandlerID:
            player.unsubscribe(self.__frameHandlerID)
            self.__frameHandlerID = None

    @property
    def fps(self):
        return self.__fps

    @fps.setter
    def fps(self, fps):
        assert isinstance(fps, int)
        self.__fps = fps

    @property
    def numFrames(self):
        return len(self._info)

    @property
    def curFrameNum(self):
        return self.__curFrameNum

    @curFrameNum.setter
    def curFrameNum(self, frame):
        self.__curFrameNum = frame
        self._selectSprite(frame)

    @property
    def loop(self):
        return self.__loop

    @loop.setter
    def loop(self, loop=False):
        self.__loop = loop

    def isPlaying(self):
        return self.__playing

    def play(self):
        if not self.__playing:
            self.__playing = True
            self.__frameHandlerID = player.subscribe(player.ON_FRAME, self.__onFrame)

    def pause(self):
        if self.__playing:
            self.__playing = False
            player.unsubscribe(self.__frameHandlerID)
            self.__frameHandlerID = None

    def __onFrame(self):
        delta = player.getFrameDuration()

        oldCurFrame = self.__curFrameNum
        self.__curFrameNum += (delta * self.__fps) / 1000.
        if int(oldCurFrame) != int (self.__curFrameNum):
            if self.__curFrameNum > len(self._info)-1:
                if self.__loop:
                    self.__curFrameNum = 0
                else:
                    self.__curFrameNum = len(self._info)-1
                    self.pause()
                self.notifySubscribers(self.END_OF_ANIMATION, [])
            self._selectSprite(self.__curFrameNum)
