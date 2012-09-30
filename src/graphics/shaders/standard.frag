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
uniform sampler2D u_CBTexture;
uniform sampler2D u_CRTexture;
uniform sampler2D u_ATexture;
uniform sampler2D u_MaskTexture;
uniform int u_ColorModel;  // 0=rgb, 1=yuv, 2=alpha, 3=yuva
uniform float u_Alpha;
uniform vec4 u_ColorCoeff0;
uniform vec4 u_ColorCoeff1;
uniform vec4 u_ColorCoeff2;
uniform vec4 u_ColorCoeff3;
uniform bool u_bUseColorCoeff;
uniform vec4 u_Gamma;
uniform bool u_bPremultipliedAlpha;
uniform bool u_bUseMask;
uniform vec2 u_MaskPos;
uniform vec2 u_MaskSize;

vec4 convertYCbCr(mat4 colorCoeff, vec4 tex)
{
    vec4 yuv;
    yuv = vec4(tex.r,
               texture2D(u_CBTexture, (gl_TexCoord[0].st)).r,
               texture2D(u_CRTexture, (gl_TexCoord[0].st)).r,
               1.0);
    vec4 rgb;
    rgb = colorCoeff*yuv;
    return vec4(rgb.rgb, u_Alpha);
}

void main(void)
{
    vec4 rgba;
    mat4 colorCoeff;
    colorCoeff[0] = u_ColorCoeff0;
    colorCoeff[1] = u_ColorCoeff1;
    colorCoeff[2] = u_ColorCoeff2;
    colorCoeff[3] = u_ColorCoeff3;
    vec4 tex = texture2D(u_Texture, gl_TexCoord[0].st);
    if (u_ColorModel == 0 || u_ColorModel == 2) {
        float a;
        if (u_ColorModel == 0) { // 0 = rgb
            rgba = tex;
            a = u_Alpha;
        } else {               // 2 = alpha
            rgba = gl_Color;
            a = tex.a*u_Alpha;
        }
        if (u_bUseColorCoeff) {
            rgba = colorCoeff*rgba;
        }
        rgba.a *= a;
#ifdef ENABLE_YUV_CONVERSION
    } else if (u_ColorModel == 1) { // yuv
        rgba = convertYCbCr(colorCoeff, tex);
    } else if (u_ColorModel == 3) { // yuva
        rgba = convertYCbCr(colorCoeff, tex);
        rgba.a *= texture2D(u_ATexture, gl_TexCoord[0].st).r;
#endif
    } else {
        rgba = vec4(1,1,1,1);
    }
    rgba = max(rgba, vec4(0.,0.,0.,0.));
    rgba = pow(rgba, u_Gamma);
    if (u_bUseMask) {
        float mask = texture2D(u_MaskTexture, (gl_TexCoord[0].st/u_MaskSize)-u_MaskPos).r;
        if (u_bPremultipliedAlpha) {
            rgba.rgb *= mask;
        }
        rgba.a *= mask;
    }
    gl_FragColor = rgba;
}

