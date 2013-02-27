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
from . import slider, button, skin

class TimeSlider(slider.Slider):

    def __init__(self, orientation=slider.Orientation.HORIZONTAL,
            skinObj=skin.Skin.default, **kwargs):
        self.__progressThumb = None
        super(TimeSlider, self).__init__(skinObj=skinObj, orientation=orientation,
                **kwargs)

        if self._orientation == slider.Orientation.HORIZONTAL:
            progressCfg = skinObj.defaultProgressBarCfg["horizontal"]
        else:
            progressCfg = skinObj.defaultProgressBarCfg["vertical"]
        
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


class MediaControl(avg.DivNode):

    PLAY_CLICKED = avg.Publisher.genMessageID()
    PAUSE_CLICKED = avg.Publisher.genMessageID()
    SEEK_PRESSED = avg.Publisher.genMessageID()
    SEEK_MOTION = avg.Publisher.genMessageID()
    SEEK_RELEASED = avg.Publisher.genMessageID()

    def __init__(self, skinObj=skin.Skin.default, duration=1000, time=0, parent=None,
            **kwargs):
        super(MediaControl, self).__init__(**kwargs)
        self.registerInstance(self, parent)

        cfg = skinObj.defaultMediaControlCfg

        # subscribe to button & slider changes
        self._playButton = button.ToggleButton(
                uncheckedUpNode=self.__createImageNode(cfg, "playUpBmp"),
                uncheckedDownNode=self.__createImageNode(cfg, "playDownBmp"),
                uncheckedDisabledNode=self.__createImageNode(cfg, "playDisabledBmp", 
                        "playUpBmp"),
                checkedUpNode=self.__createImageNode(cfg, "pauseUpBmp"),
                checkedDownNode=self.__createImageNode(cfg, "pauseDownBmp"),
                checkedDisabledNode=self.__createImageNode(cfg, "pauseDisabledBmp", 
                        "pauseUpBmp"),
                parent=self)
        self._playButton.subscribe(button.ToggleButton.TOGGLED, self.__onTogglePlay)

        sliderWidth = self.width + cfg["barRight"] - cfg["barPos"][0]
        self._timeSlider = TimeSlider(skinObj=skinObj, pos=cfg["barPos"],
                width=sliderWidth, parent=self)
        self._timeSlider.subscribe(TimeSlider.PRESSED, self.__onSliderPressed)
        self._timeSlider.subscribe(TimeSlider.RELEASED, self.__onSliderReleased)
        self._timeSlider.subscribe(TimeSlider.THUMB_POS_CHANGED, self.__onSliderMotion)

        self._timeNode = avg.WordsNode(pos=cfg["timePos"], fontstyle=cfg["font"],
                color="FFFFFF", parent=self)
        timeLeftPos = (self.width+cfg["timeLeftPos"][0], cfg["timeLeftPos"][1])
        self._timeLeftNode = avg.WordsNode(pos=timeLeftPos, fontstyle=cfg["font"],
                color="FFFFFF", parent=self)

        self.setDuration(duration)
        self.setTime(time)

        self.publish(MediaControl.PLAY_CLICKED)
        self.publish(MediaControl.PAUSE_CLICKED)
        self.publish(MediaControl.SEEK_PRESSED)
        self.publish(MediaControl.SEEK_MOTION)
        self.publish(MediaControl.SEEK_RELEASED)

    def play(self):
        self._playButton.checked = True

    def pause(self):
        self._playButton.checked = False

    def getDuration(self):
        return self._timeSlider.range[1]

    def setDuration(self, duration):
        self._timeSlider.range = (0, duration-100)
        self.__updateText()
    duration = property(getDuration, setDuration)

    def getTime(self):
        return self._timeSlider.thumbPos

    def setTime(self, curTime):
        self._timeSlider.thumbPos = curTime
        self.__updateText()
    time = property(getTime, setTime)

    def __onTogglePlay(self, play):
        if play:
            self.notifySubscribers(MediaControl.PLAY_CLICKED, [])
        else:
            self.notifySubscribers(MediaControl.PAUSE_CLICKED, [])

    def __onSliderPressed(self):
        self.notifySubscribers(MediaControl.SEEK_PRESSED, [])

    def __onSliderReleased(self):
        self.notifySubscribers(MediaControl.SEEK_RELEASED, [])

    def __onSliderMotion(self, curTime):
        self.__updateText()
        self.notifySubscribers(MediaControl.SEEK_MOTION, [curTime])

    def __updateText(self):
        self._timeNode.text = self.__msToMinSec(self._timeSlider.thumbPos)
        self._timeLeftNode.text = "-"+self.__msToMinSec(
                (self._timeSlider.range[1]-self._timeSlider.thumbPos))

    def __createImageNode(self, cfg, src, defaultSrc=None):
        bmp = skin.getBmpFromCfg(cfg, src, defaultSrc)
        node = avg.ImageNode()
        node.setBitmap(bmp)
        return node

    def __msToMinSec(self, ms):
        ms += 500
        minutes, ms = divmod(ms, 60000)
        seconds, ms = divmod(ms, 1000)
        return "%d:%02d"%(minutes, seconds)

