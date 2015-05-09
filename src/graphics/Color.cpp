//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2014 Ulrich von Zadow
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

#include "Color.h"

#include "../base/Exception.h"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdio.h>

using namespace std;

namespace avg {

Color::Color(const std::string& s)
    : m_sOrig(s)
{
    int numChars;
    int numItems = sscanf(s.c_str(), "%2hhx%2hhx%2hhx%n", &m_R, &m_G, &m_B, &numChars);
    if (s.length() != 6 || numChars != 6 || numItems != 3) {
        throw(Exception(AVG_ERR_INVALID_ARGS, "colorstring cannot be parsed."));
    }
}

Color::Color(unsigned char r, unsigned char g, unsigned char b)
    : m_R(r),
      m_G(g),
      m_B(b)
{
}

Color::~Color()
{
}


unsigned char Color::getR() const
{
    return m_R;
}

unsigned char Color::getG() const
{
    return m_G;
}

unsigned char Color::getB() const
{
    return m_B;
}

void Color::setR(unsigned char r)
{
    m_R = r;
    m_sOrig = "";
}

void Color::setG(unsigned char g)
{
    m_G = g;
    m_sOrig = "";
}

void Color::setB(unsigned char b)
{
    m_B = b;
    m_sOrig = "";
}

Color::operator Pixel32() const
{
    return Pixel32(m_R, m_G, m_B);
}

Color::operator std::string() const
{
    if (m_sOrig == "") {
        stringstream s;
        s.fill('0');
        s << hex << setw(2) << m_R << setw(2) << m_G << setw(2) << m_B;
        return s.str();
    } else {
        return m_sOrig;
    }
}

bool Color::operator ==(const Color& c) const
{
    return m_R == c.m_R && m_G == c.m_G && m_B == c.m_B;
}

Color Color::mix(const Color& c1, const Color& c2, float ratio)
{
}

XYZColor::XYZColor(float X, float Y, float Z)
    : x(X),
      y(Y),
      z(Z)
{
}

// Color conversion formulas taken from EasyRGB.com
XYZColor RGB2XYZ(const Color& rgb)
{
    float r = (rgb.getR()/255.f);
    float g = (rgb.getG()/255.f);
    float b = (rgb.getB()/255.f);

    if (r > 0.04045f) {
        r = pow((r+0.055f)/1.055f, 2.4f);
    } else {
        r = r/12.92f;
    }
    if (g > 0.04045f) {
        g = pow((g+0.055f)/1.055f, 2.4f);
    } else {
        g = g/12.92f;
    }
    if (b > 0.04045f) {
        b = pow((b+0.055f)/1.055f, 2.4f);
    } else {
        b = b/12.92f;
    }

    return XYZColor (
            r*41.24f + g*35.76f + b*18.05f,
            r*21.26f + g*71.52f + b* 7.22f,
            r* 1.93f + g*11.92f + b*95.05f);
}

Color XYZ2RGB(const XYZColor& xyz)
{
    float r = xyz.x* 0.032406f + xyz.y*-0.015372f + xyz.z*-0.004986f;
    float g = xyz.x*-0.009689f + xyz.y* 0.018758f + xyz.z* 0.000415f;
    float b = xyz.x* 0.000557f + xyz.y*-0.002040f + xyz.z* 0.010570f;

    if (r > 0.0031308f) {
        r = 1.055f*pow(r, (1/2.4f)) - 0.055f;
    } else {
        r = 12.92f*r;
    }
    if (g > 0.0031308f) {
        g = 1.055f*pow(g, (1/2.4f)) - 0.055f;
    } else {
        g = 12.92f*g;
    }
    if (b > 0.0031308f) {
        b = 1.055f*pow(b, (1/2.4f)) - 0.055f;
    } else {
        b = 12.92f*b;
    }

    return Color((unsigned char)(r*255+0.5f), (unsigned char)(g*255+0.5f), 
            (unsigned char)(b*255+0.5f));
}

}
