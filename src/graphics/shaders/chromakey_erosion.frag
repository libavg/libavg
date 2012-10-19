//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2011 Ulrich von Zadow
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

uniform sampler2D u_Texture;
uniform bool u_bIsLast;

#ifndef FRAGMENT_ONLY
varying vec2 v_TexCoord;
varying vec4 v_Color;
#endif
      
void main(void)
{
    float minAlpha = 1.0;
    float dx = dFdx(v_TexCoord.x);
    float dy = dFdy(v_TexCoord.y);
    for (float y = -1.0; y <= 1.0; ++y) {
        for (float x = -1.0; x <= 1.0; ++x) {
           float a = texture2D(u_Texture, v_TexCoord+vec2(x*dx,y*dy)).a;
           minAlpha = min(minAlpha, a);
        }
    }
    vec4 tex = texture2D(u_Texture, v_TexCoord);
    if (u_bIsLast) {
       gl_FragColor = vec4(tex.rgb*minAlpha, minAlpha);
    } else {
       gl_FragColor = vec4(tex.rgb, minAlpha);
    }
}
