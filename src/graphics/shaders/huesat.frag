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

const vec3 lumCoeff = vec3(0.2125, 0.7154, 0.0721);
const vec3 white = vec3(1.0, 1.0, 1.0);
const vec3 black = vec3(0.0, 0.0, 0.0);
uniform sampler2D texture;
uniform float hue;
uniform float sat;
uniform float l_offset;
uniform bool b_colorize;

#include "helper.frag"            

void main(void)
{
    float tmp;
    float s;
    float l;
    float h;
    vec4 tex = texture2D(texture, gl_TexCoord[0].st);
    unPreMultiplyAlpha(tex);
    rgb2hsl(tex, tmp, s, l);
    if (b_colorize) {
       h = hue;
       s = sat;
    } else {
       h = hue+tmp;
    }
    vec4 rgbTex = vec4(hsl2rgb(mod(h, 360.0), s, l), tex.a);

    // Saturate in rgb - space to imitate photoshop filter
    if (!b_colorize) { 
      s = clamp(sat+s, 0.0, 2.0);
      vec3 intensity = vec3(dot(rgbTex.rgb, lumCoeff));
      rgbTex.rgb = mix(intensity, rgbTex.rgb, s);
    }

    // Brightness with black/white pixels to imitate photoshop lightness-offset
    if (l_offset >= 0.0) { 
       rgbTex = vec4(mix(rgbTex.rgb, white, l_offset), tex.a);
    } else if (l_offset < 0.0) { 
       rgbTex = vec4(mix(rgbTex.rgb, black, -l_offset), tex.a);
    }

    preMultiplyAlpha(rgbTex);
    gl_FragColor = rgbTex;
}
