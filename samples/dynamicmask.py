#!/usr/bin/env python
# -*- coding: utf-8 -*-

# libavg - Media Playback Engine.
# Copyright (C) 2003-2021 Ulrich von Zadow
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
# Original author of this file: Thomas Schott <scotty at c-base dot org>

from libavg import avg, app, player

SIZE = avg.Bitmap('rgb24-64x64.png').getSize()


class MainDiv(app.MainDiv):
    MIN_DRAW_DIST = 10.0 # minimal length of draw line segments

    def onInit(self):
        # white background
        avg.RectNode(parent=self, size=self.size, fillopacity=1.0)
        # foreground image
        self._fg = avg.ImageNode(parent=self, href='rgb24-64x64.png',
                size=self.size, masksize=self.size)

        # mask drawing canvas
        self._maskCanvas = player.createCanvas(id='mask', size=self.size,
                handleevents=False, autorender=False)
        # image to put the canvas on-screen (invisible)
        self._maskImage = avg.ImageNode(parent=self, href='canvas:mask',
                opacity=0.0)
        # black mask on start (full transparency)
        avg.RectNode(parent=self._maskCanvas.getRootNode(), size=self.size,
                color='000000', fillcolor='000000', fillopacity=1.0)
        # poly line to draw white on black mask (adding opacity)
        self._drawLine = None

        self._updateMask()

        self._maskImage.subscribe(self._maskImage.CURSOR_DOWN, self._onDown)
        self._maskImage.subscribe(self._maskImage.CURSOR_MOTION, self._onMotion)
        self._maskImage.subscribe(self._maskImage.CURSOR_UP, self._onUp)

    def _onDown(self, event):
        self._drawLine = avg.PolyLineNode(parent=self._maskCanvas.getRootNode(),
                pos=[event.pos], color='FFFFFF', strokewidth=42.0, opacity=0.25)

    def _onMotion(self, event):
        if not self._drawLine:
            return
        pos = self._drawLine.pos
        if (pos[-1] - event.pos).getNorm() < MainDiv.MIN_DRAW_DIST:
            return
        pos.append(event.pos)
        self._drawLine.pos = pos
        self._updateMask()

    def _onUp(self, _):
        self._drawLine = None

    def _updateMask(self):
        self._maskCanvas.render()
        self._fg.setMaskBitmap(self._maskCanvas.screenshot())


if __name__ == '__main__':
    app.App().run(MainDiv())
