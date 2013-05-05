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

from libavg import avg

import os, copy
import xml.etree.ElementTree as ET

class Skin: 
   
    default = None

    def __init__(self, skinXmlFName, mediaDir=""):
        global defaultMediaDir
        self.__mediaDir = defaultMediaDir if mediaDir == "" else mediaDir
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

        self.textButtonCfg, self.defaultTextButtonCfg = self.__parseElement(
                xmlRoot, "textbutton", 
                bmpArgNames={"upSrc": "upBmp", "downSrc": "downBmp", 
                        "disabledSrc": "disabledBmp"},
                fontArgNames=("font", "downFont", "disabledFont"))

        self.checkBoxCfg, self.defaultCheckBoxCfg = self.__parseElement(
                xmlRoot, "checkbox",
                bmpArgNames={"uncheckedUpSrc":"uncheckedUpBmp", 
                        "uncheckedDownSrc":"uncheckedDownBmp",
                        "uncheckedDisabledSrc":"uncheckedDisabledBmp",
                        "checkedUpSrc":"checkedUpBmp", 
                        "checkedDownSrc":"checkedDownBmp",
                        "checkedDisabledSrc":"checkedDisabledBmp"},
                fontArgNames=("font", "downFont", "disabledFont"))

        self.sliderCfg, self.defaultSliderCfg = self.__initSliders(xmlRoot, "slider")
        self.scrollBarCfg, self.defaultScrollBarCfg = self.__initSliders(
                xmlRoot, "scrollbar")
        self.progressBarCfg, self.defaultProgressBarCfg = self.__initSliders(
                xmlRoot, "progressbar")

        self.scrollAreaCfg, self.defaultScrollAreaCfg = self.__parseElement(
                xmlRoot, "scrollarea", 
                pyArgNames=("friction","borderEndsExtent","margins",
                        "sensitiveScrollBars"),
                bmpArgNames={"borderSrc":"borderBmp"})

        self.mediaControlCfg, self.defaultMediaControlCfg = self.__parseElement(
                xmlRoot, "mediacontrol",
                bmpArgNames={"playUpSrc":"playUpBmp", 
                        "playDownSrc":"playDownBmp",
                        "playDisabledSrc":"playDisabledBmp",
                        "pauseUpSrc":"pauseUpBmp", 
                        "pauseDownSrc":"pauseDownBmp",
                        "pauseDisabledSrc":"pauseDisabledBmp"},
                pyArgNames=("timePos", "timeLeftPos", "barPos", "barRight"),
                fontArgNames=("font"))

    def __parseElement(self, xmlRoot, elementName, pyArgNames=(), bmpArgNames={}, 
            fontArgNames=()):
        cfgMap = {}
        defaultCfg = None
        for node in xmlRoot.findall(elementName):
            nodeid, attrs = self.__splitAttrs(node)
            kwargs = self.__extractArgs(attrs, pyArgNames=pyArgNames, 
                    bmpArgNames=bmpArgNames, fontArgNames=fontArgNames)
            cfgMap[nodeid] = kwargs
            if defaultCfg == None or nodeid == None:
                defaultCfg = kwargs
        return cfgMap, defaultCfg

    def __splitAttrs(self, xmlNode):
        attrs = xmlNode.attrib
        if "id" in attrs:
            nodeID = attrs["id"]
            del attrs["id"]
        else:
            nodeID = None
        return nodeID, attrs

    def __extractArgs(self, attrs, pyArgNames=(), bmpArgNames={}, fontArgNames=()):
        kwargs = {}
        for (key, value) in attrs.iteritems():
            if key in pyArgNames:
                kwargs[key] = eval(value)
            elif key in bmpArgNames.iterkeys():
                argkey = bmpArgNames[key]
                kwargs[argkey] = avg.Bitmap(os.path.join(self.__mediaDir, value))
            elif key in fontArgNames:
                kwargs[key] = self.fonts[value]
            else:
                kwargs[key] = value
        return kwargs

    def __initSliders(self, xmlRoot, typeName):
        sliderCfg = {}
        defaultSliderCfg = None
        for sliderXmlNode in xmlRoot.findall(typeName):
            (nodeID, bogus) = self.__splitAttrs(sliderXmlNode)
            sliderCfg[nodeID] = {}
            if defaultSliderCfg == None or nodeID == None:
                defaultSliderCfg = sliderCfg[nodeID]
            for xmlNode in sliderXmlNode.findall("*"):
                # Loop through orientations (horiz, vert)
                bogus, attrs = self.__splitAttrs(xmlNode)
                kwargs = self.__extractArgs(attrs,
                        pyArgNames=("trackEndsExtent", "thumbEndsExtent"),
                        bmpArgNames={"trackSrc": "trackBmp", 
                                "trackDisabledSrc": "trackDisabledBmp",
                                "thumbUpSrc": "thumbUpBmp",
                                "thumbDownSrc": "thumbDownBmp",
                                "thumbDisabledSrc": "thumbDisabledBmp"})
                sliderCfg[nodeID][xmlNode.tag] = kwargs

        return (sliderCfg, defaultSliderCfg)
    

def getBmpFromCfg(cfg, bmpName, defaultName=None):
    if bmpName in cfg:
        return cfg[bmpName]
    else:
        return cfg[defaultName]
    

defaultMediaDir = os.path.join(os.path.dirname(__file__), "..", 'data/')
Skin.default = Skin(defaultMediaDir+"SimpleSkin.xml", defaultMediaDir)
