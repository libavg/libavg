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

