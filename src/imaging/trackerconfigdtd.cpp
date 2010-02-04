//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2008 Ulrich von Zadow
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

const char * g_pTrackerConfigDTD = 
"<!ENTITY % minmaxAttrs\n"
"  \"min CDATA #REQUIRED\n"
"    max CDATA #REQUIRED\" >\n"
 
"<!ENTITY % posAttrs\n"
"  \"x CDATA #REQUIRED\n"
"    y CDATA #REQUIRED\" >\n"
 
"<!ELEMENT trackerconfig (camera|tracker|transform)* >\n"
"<!ELEMENT camera (driver|device|fw800|format|size|framerate|brightness|gamma|gain|"
"        shutter|strobeduration)* >\n"
"<!ELEMENT tracker (mask|prescale|historyupdateinterval|brighterregions|eventonmove|"
"       contourprecision|historydelay|touch|track)* >\n"
"<!ELEMENT touch (threshold|similarity|areabounds|eccentricitybounds|bandpass|"
"       bandpasspostmult)* >\n"
"<!ELEMENT track (threshold|similarity|areabounds|eccentricitybounds)* >\n"
"<!ELEMENT transform (cameradisplacement|camerascale|distortionparams|trapezoid|"
"       angle|displaydisplacement|displayscale|activedisplaysize|displayroi)* >\n"

"<!ELEMENT driver EMPTY>\n"
"<!ATTLIST driver\n"
"   value CDATA #REQUIRED >\n"

"<!ELEMENT device EMPTY>\n"
"<!ATTLIST device\n"
"   value CDATA #REQUIRED >\n"

"<!ELEMENT fw800 EMPTY>\n"
"<!ATTLIST fw800\n"
"   value CDATA #REQUIRED >\n"

"<!ELEMENT format EMPTY>\n"
"<!ATTLIST format\n"
"   value CDATA #REQUIRED >\n"

"<!ELEMENT size EMPTY>\n"
"<!ATTLIST size\n"
"   %posAttrs; >\n"

"<!ELEMENT framerate EMPTY>\n"
"<!ATTLIST framerate\n"
"   value CDATA #REQUIRED >\n"

"<!ELEMENT brightness EMPTY>\n"
"<!ATTLIST brightness\n"
"   value CDATA #REQUIRED >\n"

"<!ELEMENT gamma EMPTY>\n"
"<!ATTLIST gamma\n"
"   value CDATA #REQUIRED >\n"

"<!ELEMENT gain EMPTY>\n"
"<!ATTLIST gain\n"
"   value CDATA #REQUIRED >\n"

"<!ELEMENT shutter EMPTY>\n"
"<!ATTLIST shutter\n"
"   value CDATA #REQUIRED >\n"

"<!ELEMENT strobeduration EMPTY>\n"
"<!ATTLIST strobeduration\n"
"   value CDATA #REQUIRED >\n"

"<!ELEMENT brighterregions EMPTY>\n"
"<!ATTLIST brighterregions\n"
"   value CDATA #REQUIRED >\n"

"<!ELEMENT eventonmove EMPTY>\n"
"<!ATTLIST eventonmove\n"
"   value CDATA #REQUIRED >\n"

"<!ELEMENT threshold EMPTY>\n"
"<!ATTLIST threshold\n"
"   value CDATA #REQUIRED >\n"

"<!ELEMENT mask EMPTY>\n"
"<!ATTLIST mask\n"
"   value CDATA #REQUIRED >\n"

"<!ELEMENT prescale EMPTY>\n"
"<!ATTLIST prescale\n"
"   value CDATA #REQUIRED >\n"

"<!ELEMENT historyupdateinterval EMPTY>\n"
"<!ATTLIST historyupdateinterval\n"
"   value CDATA #REQUIRED >\n"

"<!ELEMENT contourprecision EMPTY>\n"
"<!ATTLIST contourprecision\n"
"   value CDATA #REQUIRED >\n"

"<!ELEMENT historydelay EMPTY>\n"
"<!ATTLIST historydelay\n"
"   value CDATA #REQUIRED >\n"

"<!ELEMENT similarity EMPTY>\n"
"<!ATTLIST similarity\n"
"   value CDATA #REQUIRED >\n"

"<!ELEMENT areabounds EMPTY>\n"
"<!ATTLIST areabounds\n"
"   %minmaxAttrs; >\n"

"<!ELEMENT eccentricitybounds EMPTY>\n"
"<!ATTLIST eccentricitybounds\n"
"   %minmaxAttrs; >\n"

"<!ELEMENT bandpass EMPTY>\n"
"<!ATTLIST bandpass\n"
"   %minmaxAttrs; >\n"

"<!ELEMENT bandpasspostmult EMPTY>\n"
"<!ATTLIST bandpasspostmult\n"
"   value CDATA #REQUIRED >\n"

"<!ELEMENT distortionparams EMPTY>\n"
"<!ATTLIST distortionparams\n"
"   p2 CDATA #REQUIRED\n"
"   p3 CDATA #REQUIRED >\n"

"<!ELEMENT trapezoid EMPTY>\n"
"<!ATTLIST trapezoid\n"
"   value CDATA #REQUIRED >\n"

"<!ELEMENT angle EMPTY>\n"
"<!ATTLIST angle\n"
"   value CDATA #REQUIRED >\n"

"<!ELEMENT displaydisplacement EMPTY>\n"
"<!ATTLIST displaydisplacement\n"
"   %posAttrs; >\n"

"<!ELEMENT displayscale EMPTY>\n"
"<!ATTLIST displayscale\n"
"   %posAttrs; >\n"

"<!ELEMENT activedisplaysize EMPTY>\n"
"<!ATTLIST activedisplaysize\n"
"   %posAttrs; >\n"

"<!ELEMENT displayroi EMPTY>\n"
"<!ATTLIST displayroi\n"
"  x1 CDATA #REQUIRED\n"
"  y1 CDATA #REQUIRED\n"
"  x2 CDATA #REQUIRED\n"
"  y2 CDATA #REQUIRED >\n"

;

