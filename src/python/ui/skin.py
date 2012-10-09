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

from libavg import avg, player

class Skin: 
   
    default = None # Standard-Style wird im __init__ geladen, override möglich.

    def __init__(self):
        self.fonts = {}

        self.fonts["stdfont"] = avg.FontStyle(font="Bitstream Vera Sans", variant="Roman",
                fontsize=12, color="808080", letterspacing=0, linespacing=-1)
        self.fonts["disabledFont"] = self.fonts["stdfont"]
        self.fonts["disabledFont"].color="444444"

        self.textButtonCfg = {
            "upBmp": avg.Bitmap("media/button_bg_up.png"),
            "downBmp": avg.Bitmap("media/button_bg_down.png"),
            "disabledBmp": None, #avg.Bitmap("media/button_bg_disabled.png"),
            "endsExtent": (7, 7),
            "font": self.fonts["stdfont"],
            "disabledFont": self.fonts["disabledFont"]
        }

Skin.default = Skin()
