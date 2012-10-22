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

import os, copy
import xml.etree.ElementTree as ET

class Skin: 
   
    default = None # Standard-Style wird im __init__ geladen, override möglich.

    def __init__(self, skinXmlFName):
        schemaFName = mediadir+"/skin.xsd"
        schemaString = open(schemaFName, "r").read()
        xmlString = open(skinXmlFName, "r").read()
        avg.validateXml(xmlString, schemaString, skinXmlFName, schemaFName)

        xmlRoot = ET.fromstring(xmlString)

        self.fonts = {}
        for fontNode in xmlRoot.findall("fontdef"):
            attrs = fontNode.attrib
            if "baseid" in attrs:
                self.fonts[fontNode.get("id")] = copy.copy(self.fonts[attrs["baseid"]])
                font = self.fonts[fontNode.get("id")]
                del attrs["baseid"]
                del attrs["id"]
                for (key, value) in attrs.iteritems():
                    setattr(font, key, value)
            else:
                kwargs = {}
                attrs = fontNode.attrib
                fontid = attrs["id"]
                del attrs["id"]
                for (key, value) in attrs.iteritems():
                    if key in ("fontsize", "letterspacing", "linespacing"):
                        kwargs[key] = float(value)
                    else:
                        kwargs[key] = value
                self.fonts[fontid] = avg.FontStyle(**kwargs)

        self.textButtonCfg = {
            "upBmp": avg.Bitmap("media/button_bg_up.png"),
            "downBmp": avg.Bitmap("media/button_bg_down.png"),
            "disabledBmp": None, #avg.Bitmap("media/button_bg_disabled.png"),
            "endsExtent": (7, 7),
            "font": self.fonts["stdfont"],
            "disabledFont": self.fonts["disabledfont"]
        }

mediadir = os.path.join(os.path.dirname(__file__), "..", 'data')
Skin.default = Skin(mediadir+"/SimpleSkin.xml")
