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

uniform float width;
uniform int radius;
uniform sampler2D kernelTex;
//        + getStdShaderCode()

uniform sampler2D texture;
uniform vec2 offset;
void main(void)
{
    float sum = 0.;
    float dx = dFdx(gl_TexCoord[0].x);
    for (int i=-radius; i<=radius; ++i) {
        float a = texture2D(texture,
                gl_TexCoord[0].st-offset+vec2(float(i)*dx,0)).a;
        float coeff = 
                texture2D(kernelTex, vec2((float(i+radius)+0.5)/width,0)).r;
        sum += a*coeff;
    }
    gl_FragColor = vec4(sum, sum, sum, sum);
}
