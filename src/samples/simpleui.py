#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import avg, AVGApp, ui
from libavg.ui import simple

class SimpleUIApp(AVGApp):
    def init(self):
        simple.ScrollBar(pos=(10,10), size=(150,20), parent=self._parentNode)
        simple.ScrollBar(pos=(10,40), size=(20,150), 
                orientation=ui.Orientation.VERTICAL, parent=self._parentNode)

SimpleUIApp.start(resolution=(640,480))
