#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
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

from libavg import avg, AVGApp, player
from libavg.ui import filter, simple

import os

class LabledSlider(avg.DivNode):
    def __init__(self, label, min, max, formatStr, onChange, parent=None, **kwargs):
        super(LabledSlider, self).__init__(**kwargs)
        self.registerInstance(self, parent)
        self.__onChange = onChange
        self.__formatStr = formatStr

        avg.WordsNode(text=label, parent=self)
        self.__slider = simple.Slider(size=(300,25), range=(min, max), 
                pos=(15,20), parent=self)
        self.__slider.subscribe(self.__slider.THUMB_POS_CHANGED, self.__onSliderMove)
        self.__valueDisplay = avg.WordsNode(pos=(320, 24), parent=self)
        self.__valueDisplay.text = self.__formatStr%self.__slider.thumbpos

    def getThumbPos(self):
        return self.__slider.thumbpos
    thumbpos = property(getThumbPos)

    def __onSliderMove(self, thumbPos):
        self.__onChange()
        self.__valueDisplay.text = self.__formatStr%thumbPos


class JitterFilter(AVGApp):
    multitouch=True

    def init(self):
        player.showCursor(True)
        self.__minCutoffSlider = LabledSlider(label="Minimum Cutoff", min=0.3, max=8.0,
                formatStr="%.1f", onChange=self.__onSliderMove, 
                pos=(10,10), parent=self._parentNode)
        self.__cutoffSlopeSlider = LabledSlider(label="Cutoff Slope", min=0.0, max=0.05,
                formatStr="%.3f", onChange=self.__onSliderMove, 
                pos=(10,50), parent=self._parentNode)
        self.__onSliderMove()

        self._parentNode.subscribe(avg.Node.CURSOR_DOWN, self.__onDown)
        self.__contact = None
        self.__rawContactCircle = avg.CircleNode(r=7*player.getPixelsPerMM(), 
                color="FF0000", opacity=0, parent=self._parentNode)
        self.__filteredContactCircle = avg.CircleNode(r=7*player.getPixelsPerMM(), 
                color="00FF00", opacity=0, parent=self._parentNode)
        self.__filters = None

    def __onSliderMove(self):
        self.__minCutoff = self.__minCutoffSlider.thumbpos
        self.__cutoffSlope = self.__cutoffSlopeSlider.thumbpos

    def __onDown(self, event):
        if self.__contact is None:
            self.__contact = event.contact
            event.contact.subscribe(avg.Contact.CURSOR_UP, self.__onUp)
            self.__rawContactCircle.opacity = 1
            self.__filteredContactCircle.opacity = 1
            self.__filters = [
                    filter.OneEuroFilter(self.__minCutoff,self.__cutoffSlope),
                    filter.OneEuroFilter(self.__minCutoff,self.__cutoffSlope)]
            self.__onFrame = player.setOnFrameHandler(self.__moveContact)

    def __onUp(self, event):
        self.__rawContactCircle.opacity = 0
        self.__filteredContactCircle.opacity = 0
        self.__contact = None
        self.__filters = None
        player.clearInterval(self.__onFrame)

    def __moveContact(self):
        time = player.getFrameTime()
        rawPos = self.__contact.events[-1].pos
        self.__rawContactCircle.pos = rawPos
        filteredPos = avg.Point2D(self.__filters[0].apply(rawPos.x, time),
                self.__filters[1].apply(rawPos.y, time))
        self.__filteredContactCircle.pos = filteredPos

if 'AVG_DEPLOY' in os.environ:
    resolution = player.getScreenResolution() 
else:
    resolution = (800, 600)

JitterFilter.start(resolution=resolution)

