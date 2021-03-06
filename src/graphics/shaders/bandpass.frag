//
//  libavg - Media Playback Engine.
//  Copyright (C) 2003-2021 Ulrich von Zadow
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

uniform sampler2D u_MinTex;
uniform sampler2D u_MaxTex;
uniform float u_PostScale;
uniform bool u_bInvert;

#ifndef FRAGMENT_ONLY
varying vec2 v_TexCoord;
varying vec4 v_Color;
#endif

void main(void)
{
  vec4 min = texture2D(u_MinTex, v_TexCoord); 
  vec4 max = texture2D(u_MaxTex, v_TexCoord);
  gl_FragColor = vec4(0.502, 0.502, 0.502, 0) + (max-min)*u_PostScale;
  if (u_bInvert) {
    gl_FragColor = vec4(1.004,1.004,1.004,1) - gl_FragColor;
  }
  gl_FragColor.a = 1.0;
}

