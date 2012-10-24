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
   
    default = None

    def __init__(self, skinXmlFName, mediaDir=""):
        global defaultMediaDir
        self.__mediaDir = defaultMediaDir
        schemaFName = defaultMediaDir+"skin.xsd"
        schemaString = open(schemaFName, "r").read()
        xmlString = open(skinXmlFName, "r").read()
        avg.validateXml(xmlString, schemaString, skinXmlFName, schemaFName)

        xmlRoot = ET.fromstring(xmlString)

        self.fonts = {}
        for fontNode in xmlRoot.findall("fontdef"):
            fontid, attrs = self.__splitAttrs(fontNode)
            if "baseid" in attrs:
                self.fonts[fontid] = copy.copy(self.fonts[attrs["baseid"]])
                font = self.fonts[fontid]
                del attrs["baseid"]
                for (key, value) in attrs.iteritems():
                    setattr(font, key, value)
            else:
                kwargs = self.__extractArgs(attrs, 
                        ("fontsize", "letterspacing", "linespacing"))
                self.fonts[fontid] = avg.FontStyle(**kwargs)

        self.textButtonCfg = {}
        self.defaultTextButtonCfg = None
        for node in xmlRoot.findall("textbutton"):
            nodeid, attrs = self.__splitAttrs(node)
            kwargs = self.__extractArgs(attrs,
                    bmpArgNames={"upSrc": "upBmp", "downSrc": "downBmp", 
                            "disabledSrc": "disabledBmp"},
                    fontArgNames=("font", "downFont", "disabledFont"))
            self.textButtonCfg[nodeid] = kwargs
            if self.defaultTextButtonCfg == None or nodeid == None:
                self.defaultTextButtonCfg = kwargs

        self.sliderCfg = {}
        self.defaultSliderCfg = None
        for sliderXmlNode in xmlRoot.findall("slider"):
            (nodeID, bogus) = self.__splitAttrs(sliderXmlNode)
            self.sliderCfg[nodeID] = {}
            if self.defaultSliderCfg == None or nodeID == None:
                self.defaultSliderCfg = self.sliderCfg[nodeID]
            for xmlNode in sliderXmlNode.iterfind("*"):
                # Loop through orientations (horiz, vert)
                bogus, attrs = self.__splitAttrs(xmlNode)
                kwargs = self.__extractArgs(attrs,
                        floatArgNames=("trackEndsExtent",),
                        bmpArgNames={"trackSrc": "trackBmp", 
                                "trackDisabledSrc": "trackDisabledBmp", 
                                "thumbUpSrc": "thumbUpBmp",
                                "thumbDownSrc": "thumbDownBmp",
                                "thumbDisabledSrc": "thumbDisabledBmp"})
                self.sliderCfg[nodeID][xmlNode.tag] = kwargs

    def __splitAttrs(self, xmlNode):
        attrs = xmlNode.attrib
        if "id" in attrs:
            nodeID = attrs["id"]
            del attrs["id"]
        else:
            nodeID = None
        return nodeID, attrs

    def __extractArgs(self, attrs, floatArgNames=(), bmpArgNames={}, fontArgNames=()):
        kwargs = {}
        for (key, value) in attrs.iteritems():
            if key in floatArgNames:
                kwargs[key] = float(value)
            elif key in bmpArgNames.iterkeys():
                argkey = bmpArgNames[key]
                kwargs[argkey] = avg.Bitmap(self.__mediaDir+value)
            elif key in fontArgNames:
                kwargs[key] = self.fonts[value]
            else:
                kwargs[key] = value
        return kwargs


defaultMediaDir = os.path.join(os.path.dirname(__file__), "..", 'data/')
Skin.default = Skin(defaultMediaDir+"SimpleSkin.xml", defaultMediaDir)
