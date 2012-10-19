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
uniform float u_HKey;
uniform float u_HTolerance;
uniform float u_HSoftTolerance;
uniform float u_SKey;
uniform float u_STolerance;
uniform float u_SSoftTolerance;
uniform float u_LKey;
uniform float u_LTolerance;
uniform float u_LSoftTolerance;
uniform float u_SpillThreshold;
uniform bool u_bIsLast;
       
#include "helper.frag"

#ifndef FRAGMENT_ONLY
varying vec2 v_TexCoord;
varying vec4 v_Color;
#endif
      
vec4 alphaMin(vec4 v1, vec4 v2)
{
    if (v1.a < v2.a) {
        return v1;
    } else {
        return v2;
    }
}

vec4 alphaMax(vec4 v1, vec4 v2)
{
    if (v1.a < v2.a) {
        return v2;
    } else {
        return v1;
    }
}

#define s2(a, b)    temp = a; a = alphaMin(a, b); b = alphaMax(temp, b);
#define mn3(a, b, c)            s2(a, b); s2(a, c);
#define mx3(a, b, c)            s2(b, c); s2(a, c);

#define mnmx3(a, b, c)          mx3(a, b, c); s2(a, b);
#define mnmx4(a, b, c, d)       s2(a, b); s2(c, d); s2(a, c); s2(b, d); 

// Based on McGuire, A fast, small-radius GPU median filter, in ShaderX6,
// February 2008. http://graphics.cs.williams.edu/papers/MedianShaderX6/ 
vec4 getMedian(vec2 texCoord)
{
    vec4 v[5];
    float dx = dFdx(texCoord.x);
    float dy = dFdy(texCoord.y);
    v[0] = texture2D(u_Texture, texCoord);
    v[1] = texture2D(u_Texture, texCoord+vec2(0,-dy));
    v[2] = texture2D(u_Texture, texCoord+vec2(0,dy));
    v[3] = texture2D(u_Texture, texCoord+vec2(-dx,0));
    v[4] = texture2D(u_Texture, texCoord+vec2(dx,0));
    for (int i = 0; i < 5; ++i) {
        v[i].a = 0.2989 * v[i].r + 0.5870 * v[i].g + 0.1140 * v[i].b;
    }

    vec4 temp;
    mnmx4(v[0], v[1], v[2], v[3]);
    mnmx3(v[1], v[2], v[4]);
    return v[2];
}

void main(void)
{
    vec4 tex = getMedian(v_TexCoord);
    float h;
    float s;
    float l;
    float alpha;
    rgb2hsl(tex, h, s, l);
    float hDiff = abs(h-u_HKey);
    float sDiff = abs(s-u_SKey);
    float lDiff = abs(l-u_LKey);
    if (hDiff < u_HSoftTolerance && sDiff < u_SSoftTolerance 
            && lDiff < u_LSoftTolerance)
    {
        alpha = 0.0;
        if (hDiff > u_HTolerance) {
            alpha = (hDiff-u_HTolerance)/(u_HSoftTolerance-u_HTolerance);
        }        
        if (sDiff > u_STolerance) {
            alpha = max(alpha,
                   (sDiff-u_STolerance)/(u_SSoftTolerance-u_STolerance));
        }
        if (lDiff > u_LTolerance) {
            alpha = max(alpha,
                   (lDiff-u_LTolerance)/(u_LSoftTolerance-u_LTolerance));
        }
    } else {
        alpha = 1.0;
    }
    tex = texture2D(u_Texture, v_TexCoord);
    if (alpha > 0.0 && hDiff < u_SpillThreshold) {
        rgb2hsl(tex, h, s, l);
        if (u_SpillThreshold > u_HTolerance) {
            float factor = max(0.0, 1.0-(u_SpillThreshold-hDiff)
                    /(u_SpillThreshold-u_HTolerance));
            s = s*factor;
        }
        tex.rgb = hsl2rgb(h, s, l);
    }
    if (u_bIsLast) {
       gl_FragColor = vec4(tex.rgb*alpha, alpha);
    } else {
       gl_FragColor = vec4(tex.rgb, alpha);
    }
}

