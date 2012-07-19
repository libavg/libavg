#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import avg, AVGApp

BASE_STYLE = avg.Style(font="Arial", variant="Regular", fontsize=14, linespacing=2)
HEADER_STYLE = avg.Style(basestyle=BASE_STYLE, variant="Bold")

SAMPLE_TEXT="""
Lorem ipsum dolor sit amet, consectetur adipiscing elit. Proin vitae gravida urna. Nam ut nisi ac nulla venenatis tincidunt a in urna. Cras vel enim purus, sit amet adipiscing dolor. Aliquam tincidunt interdum velit sed hendrerit. Proin ut enim dolor, sit amet egestas mi. Aenean felis quam, sollicitudin sed tempus in, pharetra eget turpis.
"""

class HelloWorld(AVGApp):
    def init(self):
        avg.WordsNode(pos=(50,50), text="Lorem Ipsum", style=HEADER_STYLE,
                parent=self._parentNode)
        avg.WordsNode(pos=(50,70), text=SAMPLE_TEXT, width=400, style=BASE_STYLE, 
                parent=self._parentNode)


HelloWorld.start(resolution=(640, 480))

