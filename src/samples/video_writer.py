#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import avg, app, player, widget


class MyMainDiv(app.MainDiv):
    def onInit(self):
        # avg.VideoWriter requires a canvas as source
        # (can be the main canvas returned by player.getMainCanvas())
        canvas = player.createCanvas(id='source_canvas', size=self.size)
        root = canvas.getRootNode()

        # some nodes in source canvas
        self._text = avg.WordsNode(parent=root, text='000000', fontsize=42)
        self._text.pos = (self.size - self._text.size) / 2
        self._rect = avg.RectNode(parent=root, size=(200, 200))
        self._rect.pos = (self.size - self._rect.size) / 2

        # show source canvas on screen (not required for video recording)
        avg.ImageNode(parent=self, href='canvas:source_canvas')

        # start writing source canvas to video file
        fps = int(player.getFramerate())
        self._video_writer = avg.VideoWriter(canvas, 'video_writer.avi', fps)
        self._writing = True

        # these nodes are not included in the video (outside source canvas)
        avg.WordsNode(
            parent=self, text='writing to "%s"' % self._video_writer.filename
        )
        self._btn = widget.TextButton(
            parent=self, pos=(0, 20), size=(100, 25), text='PAUSE'
        )
        self._btn.subscribe(widget.Button.CLICKED, self._onButton)

    def onExit(self):
        # (asynchronously) finish video file writing
        self._video_writer.stop()
        # and wait for sync
        del self._video_writer
        self._video_writer = None

    def onFrame(self):
        self._text.text = '%06d' % player.getFrameTime()
        self._rect.angle += player.getFrameDuration() * 0.001

    def _onButton(self):
        if self._writing:
            self._video_writer.pause()
            self._btn.text = 'RECORD'
        else:
            self._video_writer.play()
            self._btn.text = 'PAUSE'
        self._writing = not self._writing


if __name__ == '__main__':
    app.App().run(MyMainDiv())
