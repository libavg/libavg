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

uniform float u_Width;
uniform int u_Radius;
uniform sampler2D u_KernelTex;
uniform sampler2D u_HBlurTex;
uniform sampler2D u_OrigTex;
uniform vec4 u_Color;
uniform vec2 u_DestPos;
uniform vec2 u_DestSize;

void main(void)
{
    float sum = 0.;
    float dy = dFdy(gl_TexCoord[0].y);
    for (int i=-u_Radius; i<=u_Radius; ++i) {
        float a = texture2D(u_HBlurTex,
                gl_TexCoord[0].st+vec2(0,float(i)*dy)).a;
        float coeff = 
                texture2D(u_KernelTex, vec2((float(i+u_Radius)+0.5)/u_Width,0)).r;
        sum += a*coeff;
    }
    sum = min(1., sum);
    vec2 origCoord = gl_TexCoord[0].st;
    origCoord = u_DestPos + 
            vec2(origCoord.s*u_DestSize.x, origCoord.t*u_DestSize.y);
    vec4 origCol = texture2D(u_OrigTex, origCoord);
    gl_FragColor = origCol+(1.-origCol.a)*u_Color*sum;
}

