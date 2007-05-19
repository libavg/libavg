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

const char * g_pTrackerConfigDTD = 
"<!ENTITY % minmaxAttrs\n"
"  \"min CDATA #REQUIRED\n"
"    max CDATA #REQUIRED\" >\n"
 
"<!ENTITY % posAttrs\n"
"  \"x CDATA #REQUIRED\n"
"    y CDATA #REQUIRED\" >\n"
 
"<!ELEMENT trackerconfig (camera|tracker|transform)* >\n"
"<!ELEMENT camera (fps|brightness|exposure|gamma|gain|shutter)* >\n"
"<!ELEMENT tracker (historyupdateinterval|touch|track)* >\n"
"<!ELEMENT touch (threshold|similarity|areabounds|eccentricitybounds)* >\n"
"<!ELEMENT track (threshold|similarity|areabounds|eccentricitybounds)* >\n"
"<!ELEMENT transform (cameradisplacement|camerascale|distortionparams|trapezoid|"
"       angle|displaydisplacement|displayscale)* >\n"

"<!ELEMENT fps EMPTY>\n"
"<!ATTLIST fps\n"
"   value CDATA #REQUIRED >\n"

"<!ELEMENT brightness EMPTY>\n"
"<!ATTLIST brightness\n"
"   value CDATA #REQUIRED >\n"

"<!ELEMENT exposure EMPTY>\n"
"<!ATTLIST exposure\n"
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

"<!ELEMENT threshold EMPTY>\n"
"<!ATTLIST threshold\n"
"   value CDATA #REQUIRED >\n"

"<!ELEMENT historyupdateinterval EMPTY>\n"
"<!ATTLIST historyupdateinterval\n"
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

"<!ELEMENT cameradisplacement EMPTY>\n"
"<!ATTLIST cameradisplacement\n"
"   %posAttrs; >\n"

"<!ELEMENT camerascale EMPTY>\n"
"<!ATTLIST camerascale\n"
"   %posAttrs; >\n"

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


;

