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

uniform float alpha;
uniform sampler2D texture;
uniform float hKey;
uniform float hTolerance;
uniform float hSoftTolerance;
uniform float sTolerance;
uniform float sSoftTolerance;
uniform float sKey;
uniform float lTolerance;
uniform float lSoftTolerance;
uniform float spillThreshold;
uniform float lKey;
uniform bool bIsLast;
       
#include "helper.frag"
        
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
    v[0] = texture2D(texture, texCoord);
    v[1] = texture2D(texture, texCoord+vec2(0,-dy));
    v[2] = texture2D(texture, texCoord+vec2(0,dy));
    v[3] = texture2D(texture, texCoord+vec2(-dx,0));
    v[4] = texture2D(texture, texCoord+vec2(dx,0));
    for (int i = 0; i < 5; ++i) {
        v[i].a = (v[i].r+v[i].g+v[i].b)/3.0;
    }

    vec4 temp;
    mnmx4(v[0], v[1], v[2], v[3]);
    mnmx3(v[1], v[2], v[4]);
    return v[2];
}

void main(void)
{
    vec4 tex = getMedian(gl_TexCoord[0].st);
    float h;
    float s;
    float l;
    float alpha;
    rgb2hsl(tex, h, s, l);
    float hDiff = abs(h-hKey);
    float sDiff = abs(s-sKey);
    float lDiff = abs(l-lKey);
    if (hDiff < hSoftTolerance && sDiff < sSoftTolerance 
            && lDiff < lSoftTolerance)
    {
        alpha = 0.0;
        if (hDiff > hTolerance) {
            alpha = (hDiff-hTolerance)/(hSoftTolerance-hTolerance);
        }        
        if (sDiff > sTolerance) {
            alpha = max(alpha,
                   (sDiff-sTolerance)/(sSoftTolerance-sTolerance));
        }
        if (lDiff > lTolerance) {
            alpha = max(alpha,
                   (lDiff-lTolerance)/(lSoftTolerance-lTolerance));
        }
    } else {
        alpha = 1.0;
    }
    tex = texture2D(texture, gl_TexCoord[0].st);
    if (alpha > 0.0 && hDiff < spillThreshold) {
        if (spillThreshold > hTolerance) {
            float factor = max(0.0, 1.0-(spillThreshold-hDiff)
                    /(spillThreshold-hTolerance));
            s = s*factor;
        }
        tex.rgb = hsl2rgb(h, s, l);
    }
    if (bIsLast) {
       gl_FragColor = vec4(tex.rgb*alpha, alpha);
    } else {
       gl_FragColor = vec4(tex.rgb, alpha);
    }
}

