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

void unPreMultiplyAlpha(inout vec4 color)
{
    if (color.a > 0.0) {
       color.rgb /= color.a;
    }
}

void preMultiplyAlpha(inout vec4 color)
{
    color.rgb *= color.a;
}

void rgb2hsl(vec4 rgba, out float h, out float s, out float l)
{
    float maxComp = max(rgba.r, max(rgba.g, rgba.b));
    float minComp = min(rgba.r, min(rgba.g, rgba.b));
    l = (maxComp+minComp)/2.0;
    if (maxComp == minComp) {
        s = 0.0;
        h = 0.0;
    } else {
        float delta = maxComp-minComp;
        if (l < 0.5) {
            s = delta/(maxComp+minComp);
        } else {
            s = delta/(2.0-(maxComp+minComp));
        }
        if (rgba.r == maxComp) {
            h = (rgba.g-rgba.b)/delta;
            if (h < 0.0) {
                h += 6.0;
            }
        } else if (rgba.g == maxComp) {
            h = 2.0+(rgba.b-rgba.r)/delta;
        } else {
            h = 4.0+(rgba.r-rgba.g)/delta;
        }
        h *= 60.0;
    }
}

vec3 hsl2rgb(float h, float s, float l)
{
    vec3 rgb = vec3(0.0, 0.0, 0.0);
    float v;
    if (l <= 0.5) {
        v = l*(1.0+s);
    } else {
        v = l+s-l*s;
    }
    if (v > 0.0) {
        float m = 2.0*l-v;
        float sv = (v-m)/v;
        h /= 60.0;
        int sextant = int(h);
        float fract = h-float(sextant);
        float vsf = v * sv * fract;
        float mid1 = m + vsf;
        float mid2 = v - vsf;
        if (sextant == 0) {
            rgb.r = v;
            rgb.g = mid1;
            rgb.b = m;
        } else if (sextant == 1) {
            rgb.r = mid2;
            rgb.g = v;
            rgb.b = m;
        } else if (sextant == 2) {
            rgb.r = m;
            rgb.g = v;
            rgb.b = mid1;
        } else if (sextant == 3) {
            rgb.r = m;
            rgb.g = mid2;
            rgb.b = v;
        } else if (sextant == 4) {
            rgb.r = mid1;
            rgb.g = m;
            rgb.b = v;
        } else if (sextant == 5) {
            rgb.r = v;
            rgb.g = m;
            rgb.b = mid2;
        }
    }
    return rgb;
}

