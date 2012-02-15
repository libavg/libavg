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

uniform sampler2D texture;
uniform sampler2D yTexture;
uniform sampler2D cbTexture;
uniform sampler2D crTexture;
uniform sampler2D aTexture;
uniform sampler2D maskTexture;
uniform int colorModel;  // 0=rgb, 1=yuv, 2=greyscale, 3=yuva
uniform vec4 colorCoeff0;
uniform vec4 colorCoeff1;
uniform vec4 colorCoeff2;
uniform vec4 colorCoeff3;
uniform bool bUseColorCoeff;
uniform vec4 gamma;
uniform bool bPremultipliedAlpha;
uniform bool bUseMask;
uniform vec2 maskPos;
uniform vec2 maskSize;

vec4 convertYCbCr(mat4 colorCoeff)
{
    vec4 yuv;
    yuv = vec4(texture2D(texture, gl_TexCoord[0].st).r,
               texture2D(cbTexture, (gl_TexCoord[0].st)).r,
               texture2D(crTexture, (gl_TexCoord[0].st)).r,
               1.0);
    vec4 rgb;
    rgb = colorCoeff*yuv;
    return vec4(rgb.rgb, gl_Color.a);
}

void main(void)
{
    vec4 rgba;
    mat4 colorCoeff;
    colorCoeff[0] = colorCoeff0;
    colorCoeff[1] = colorCoeff1;
    colorCoeff[2] = colorCoeff2;
    colorCoeff[3] = colorCoeff3;
    if (colorModel == 0) {
        rgba = texture2D(texture, gl_TexCoord[0].st);
        if (bUseColorCoeff) {
            rgba = colorCoeff*rgba;
        };
        rgba.a *= gl_Color.a;
    } else if (colorModel == 1) {
        rgba = convertYCbCr(colorCoeff);
    } else if (colorModel == 2) {
        rgba = gl_Color;
        if (bUseColorCoeff) {
           rgba = colorCoeff*rgba;
        };
        rgba.a *= texture2D(texture, gl_TexCoord[0].st).a;
    } else if (colorModel == 3) {
        rgba = convertYCbCr(colorCoeff);
        rgba.a *= texture2D(aTexture, gl_TexCoord[0].st).r;
    } else {
        rgba = vec4(1,1,1,1);
    }
    rgba = max(rgba, vec4(0.,0.,0.,0.));
    rgba = pow(rgba, gamma);
    if (bUseMask) {
        if (bPremultipliedAlpha) {
            rgba.rgb *= texture2D(maskTexture,
                    (gl_TexCoord[0].st/maskSize)-maskPos).r;
        }
        rgba.a *= texture2D(maskTexture,
                (gl_TexCoord[0].st/maskSize)-maskPos).r;
    }
    gl_FragColor = rgba;
}

