#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import avg, player

BASE_STYLE = avg.FontStyle(font='Arial', variant='Regular', fontsize=14, linespacing=2)
HEADER_STYLE = avg.FontStyle(basestyle=BASE_STYLE, variant='Bold', color='FF0000')

SAMPLE_TEXT="""
Lorem ipsum dolor sit amet, consectetur adipiscing elit. Proin vitae gravida urna. Nam ut nisi ac nulla venenatis tincidunt a in urna. Cras vel enim purus, sit amet adipiscing dolor. Aliquam tincidunt interdum velit sed hendrerit. Proin ut enim dolor, sit amet egestas mi. Aenean felis quam, sollicitudin sed tempus in, pharetra eget turpis.
"""

canvas = player.createMainCanvas(size=(640, 480))
rootNode = canvas.getRootNode()

avg.WordsNode(pos=(50, 50), text='Lorem Ipsum', fontstyle=HEADER_STYLE, parent=rootNode)
avg.WordsNode(pos=(50, 70), text='Lorem Ipsum', fontstyle=HEADER_STYLE, variant='Italic',
        parent=rootNode)
avg.WordsNode(pos=(50, 90), text=SAMPLE_TEXT, width=400, fontstyle=BASE_STYLE,
        parent=rootNode)

player.play()

