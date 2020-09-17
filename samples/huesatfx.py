#!/usr/bin/env python
# -*- coding: utf-8 -*-

# libavg - Media Playback Engine.
# Copyright (C) 2017-2020 Ulrich von Zadow
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

from libavg import avg, app, widget


class HueSatFx(app.MainDiv):
    def onInit(self):
        # images
        orig = avg.ImageNode(href='../test/media/hsl.png',
                parent=self)
        huesat = avg.ImageNode(href='../test/media/hsl.png',
                pos=(orig.size.x + 10, 0), parent=self)

        # effect
        self._fx = avg.HueSatFXNode(colorize=False)
        huesat.setEffect(self._fx)

        y = orig.size.y + 10

        # hue slider
        self._hueTxt = avg.WordsNode(text='hue: %d' % self._fx.hue,
                fontsize=12, pos=(0, y), parent=self)
        y += self._hueTxt.height
        self._hueSld = widget.Slider(range=(-180, 180), thumbPos=self._fx.hue,
                width=self.width, pos=(0, y), parent=self)
        self._hueSld.subscribe(widget.Slider.THUMB_POS_CHANGED, self._onSliderHue)

        y += self._hueSld.height

        # saturation slider
        self._satTxt = avg.WordsNode(text='saturation: %d' % self._fx.saturation,
                fontsize=12, pos=(0, y), parent=self)
        y += self._satTxt.height
        self._satSld = widget.Slider(range=(-100, 100), thumbPos=self._fx.saturation,
                width=self.width, pos=(0, y), parent=self)
        self._satSld.subscribe(widget.Slider.THUMB_POS_CHANGED, self._onSliderSat)

        y += self._satSld.height

        # lightness slider
        self._lightTxt = avg.WordsNode(text='lightness: %d' % self._fx.lightness,
                fontsize=12, pos=(0, y), parent=self)
        y += self._lightTxt.height
        self._lightSld = widget.Slider(range=(-100, 100), thumbPos=self._fx.lightness,
                width=self.width, pos=(0, y), parent=self)
        self._lightSld.subscribe(widget.Slider.THUMB_POS_CHANGED, self._onSliderLight)

        y += self._lightSld.height

        # colorize check box
        widget.Skin.default.defaultCheckBoxCfg['font'].color = 'FFFFFF'
        self._colCbox = widget.CheckBox(text='colorize', checked=self._fx.colorize,
                pos=(0, y), parent=self)
        self._colCbox.subscribe(widget.CheckBox.TOGGLED, self._onCheckBoxCol)

    def _onSliderHue(self, value):
        self._fx.hue = int(value)
        self._hueTxt.text = 'hue: %d' % self._fx.hue

    def _onSliderSat(self, value):
        self._fx.saturation = int(value)
        self._satTxt.text = 'saturation: %d' % self._fx.saturation

    def _onSliderLight(self, value):
        self._fx.lightness = int(value)
        self._lightTxt.text = 'lightness: %d' % self._fx.lightness

    def _onCheckBoxCol(self, checked):
        self._fx.colorize = checked
        if checked:
            self._hueSld.range = (0, 360)
            self._satSld.range = (0, 100)
        else:
            self._hueSld.range = (-180, 180)
            self._satSld.range = (-100, 100)
        self._hueSld.thumbPos = self._fx.hue
        self._satSld.thumbPos = self._fx.saturation
        self._onSliderHue(self._hueSld.thumbPos)
        self._onSliderSat(self._satSld.thumbPos)


if __name__ == '__main__':
    app.App().run(HueSatFx(), app_resolution='200x200')
