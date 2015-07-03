#!/usr/bin/env python
# -*- coding: utf-8 -*-

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
