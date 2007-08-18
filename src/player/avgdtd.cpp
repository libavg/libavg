//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2006 Ulrich von Zadow
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  Current versions can be found at www.libavg.de
//

const char * g_pAVGDTD = 
"<!ENTITY % graphicsElementEvents\n"
"  \"oncursordown CDATA #IMPLIED\n"
"    oncursorup CDATA #IMPLIED\n"
"    oncursormove CDATA #IMPLIED\n"
"    oncursorover CDATA #IMPLIED\n"
"    oncursorout CDATA #IMPLIED\" >\n"

"<!ENTITY % nodeAttrs\n"
"   \"id ID #IMPLIED\n"
"    x CDATA #IMPLIED\n"
"    y CDATA #IMPLIED\n"
"    width CDATA #IMPLIED\n"
"    height CDATA #IMPLIED\n"
"    opacity CDATA #IMPLIED\n"
"    active CDATA #IMPLIED\n"
"    sensitive CDATA #IMPLIED\n"
"    %graphicsElementEvents;\">\n"

"<!ENTITY % rasterAttrs\n"
"   \"maxtilewidth CDATA #IMPLIED\n"
"    maxtileheight CDATA #IMPLIED\n"
"    angle CDATA #IMPLIED\n"
"    pivotx CDATA #IMPLIED\n"
"    pivoty CDATA #IMPLIED\n"
"    blendmode CDATA #IMPLIED\n"
"    %nodeAttrs;\" >\n"

"<!ELEMENT avg (div|excl|image|video|camera|audio|words|panoimage)* >\n"
"<!ATTLIST avg\n"
"   id ID #IMPLIED\n"
"   width CDATA #IMPLIED\n"
"   height CDATA #IMPLIED\n"
"   onkeydown CDATA #IMPLIED\n"
"   onkeyup CDATA #IMPLIED\n"
"   enablecrop CDATA #IMPLIED\n"
"   %graphicsElementEvents; >\n"

"<!ELEMENT div (div|excl|image|video|camera|audio|words|panoimage)* >\n"
"<!ATTLIST div\n"
"   %nodeAttrs; >\n"

"<!ELEMENT image EMPTY>\n"
"<!ATTLIST image\n"
"   %rasterAttrs;\n"
"   href CDATA #IMPLIED\n"
"   hue CDATA #IMPLIED\n"
"   saturation CDATA #IMPLIED >\n"

"<!ELEMENT video EMPTY>\n"
"<!ATTLIST video\n"
"   %rasterAttrs;\n"
"   href CDATA #REQUIRED\n"
"   loop CDATA #IMPLIED\n"
"   fps CDATA #IMPLIED\n"
"   threaded CDATA #IMPLIED>\n"

"<!ELEMENT camera EMPTY>\n"
"<!ATTLIST camera\n"
"   %rasterAttrs;\n"
"   device CDATA #REQUIRED\n"
"   framerate CDATA #IMPLIED\n"
"   brightness CDATA #IMPLIED\n"
"   exposure CDATA #IMPLIED\n"
"   sharpness CDATA #IMPLIED\n"
"   saturation CDATA #IMPLIED\n"
"   gamma CDATA #IMPLIED\n"
"   shutter CDATA #IMPLIED\n"
"   whitebalance CDATA #IMPLIED\n"
"   source CDATA #IMPLIED\n"
"   pixelformat CDATA #IMPLIED\n"
"   capturewidth CDATA #IMPLIED\n"
"   captureheight CDATA #IMPLIED\n"
"   channel CDATA #IMPLIED>\n"

"<!ELEMENT words (#PCDATA|span|b|big|i|s|sub|sup|small|tt|u)*>\n"
"<!ATTLIST words\n"
"  %rasterAttrs;\n"
"  size CDATA #IMPLIED\n"
"  font CDATA #IMPLIED\n"
"  text CDATA #IMPLIED\n"
"  parawidth CDATA #IMPLIED\n"
"  color CDATA #IMPLIED\n"
"  indent CDATA #IMPLIED\n"
"  linespacing CDATA #IMPLIED\n"
"  alignment CDATA #IMPLIED\n"
"  weight CDATA #IMPLIED\n"
"  italic CDATA #IMPLIED\n"
"  stretch CDATA #IMPLIED\n"
"  smallcaps CDATA #IMPLIED>\n"

"<!ELEMENT panoimage EMPTY>\n"
"<!ATTLIST panoimage\n"
"  %nodeAttrs;\n"
"  href CDATA #IMPLIED\n"
"  sensorwidth CDATA #IMPLIED\n"
"  sensorheight CDATA #IMPLIED\n"
"  focallength CDATA #IMPLIED\n"
"  hue CDATA #IMPLIED\n"
"  saturation CDATA #IMPLIED >\n"

"<!ELEMENT span (#PCDATA|span|b|big|i|s|sub|sup|small|tt|u)*>\n"
"<!ATTLIST span\n"
"  font_desc CDATA #IMPLIED\n"
"  font_family CDATA #IMPLIED\n"
"  face CDATA #IMPLIED\n"
"  size CDATA #IMPLIED\n"
"  style CDATA #IMPLIED\n"
"  weight CDATA #IMPLIED\n"
"  variant CDATA #IMPLIED\n"
"  stretch CDATA #IMPLIED\n"
"  foreground CDATA #IMPLIED\n"
"  background CDATA #IMPLIED\n"
"  underline CDATA #IMPLIED\n"
"  rise CDATA #IMPLIED\n"
"  strikethrough CDATA #IMPLIED\n"
"  fallback CDATA #IMPLIED\n"
"  lang CDATA #IMPLIED >\n"

"<!ELEMENT b (#PCDATA|span|b|big|i|s|sub|sup|small|tt|u)*>\n"
"<!ELEMENT big (#PCDATA|span|b|big|i|s|sub|sup|small|tt|u)*>\n"
"<!ELEMENT i (#PCDATA|span|b|big|i|s|sub|sup|small|tt|u)*>\n"
"<!ELEMENT s (#PCDATA|span|b|big|i|s|sub|sup|small|tt|u)*>\n"
"<!ELEMENT sub (#PCDATA|span|b|big|i|s|sub|sup|small|tt|u)*>\n"
"<!ELEMENT sup (#PCDATA|span|b|big|i|s|sub|sup|small|tt|u)*>\n"
"<!ELEMENT small (#PCDATA|span|b|big|i|s|sub|sup|small|tt|u)*>\n"
"<!ELEMENT tt (#PCDATA|span|b|big|i|s|sub|sup|small|tt|u)*>\n"
"<!ELEMENT u (#PCDATA|span|b|big|i|s|sub|sup|small|tt|u)*>\n";


