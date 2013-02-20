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

from libavg import avg
from . import slider, skin

import math

class TimeSlider(slider.Slider):

    def __init__(self, orientation=slider.Orientation.HORIZONTAL,
            skinObj=skin.Skin.default, **kwargs):
        self.__progressThumb = None
        super(TimeSlider, self).__init__(skinObj=skinObj, orientation=orientation,
                **kwargs)

        if self._orientation == slider.Orientation.HORIZONTAL:
            progressCfg= skinObj.defaultProgressBarCfg["horizontal"]
        else:
            progressCfg= skinObj.defaultProgressBarCfg["vertical"]
        
        thumbUpBmp = progressCfg["thumbUpBmp"]
        thumbDisabledBmp = skin.getBmpFromCfg(progressCfg, "thumbDisabledBmp",
                "thumbUpBmp")
        endsExtent = progressCfg["thumbEndsExtent"]
        self.__progressThumb = slider.ScrollBarThumb(orientation=orientation, 
                upBmp=thumbUpBmp, downBmp=thumbUpBmp, disabledBmp=thumbDisabledBmp,
                endsExtent=endsExtent)
        self.insertChildAfter(self.__progressThumb, self._trackNode)
        self._positionNodes()
 
    def _positionNodes(self, newSliderPos=None):
        super(TimeSlider, self)._positionNodes(newSliderPos)

        if self.__progressThumb:
            if self._orientation == slider.Orientation.HORIZONTAL:
                self.__progressThumb.width = self._thumbNode.x+self._thumbNode.width/2
            else:
                self.__progressThumb.height = self._thumbNode.y+self._thumbNode.height/2
        
