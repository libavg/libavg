//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2020 Ulrich von Zadow
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
#include "../base/MathHelper.h"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdio.h>
#include <math.h>

using namespace std;

namespace avg {

Color::Color()
{
}

Color::Color(const std::string& s)
    : m_sOrig(s)
{
    int numChars = 0;
    int numItems;
    switch(s.length()) {
        case 6:
            numItems = sscanf(s.c_str(), "%2hhx%2hhx%2hhx%n", &m_R, &m_G, &m_B,
                    &numChars);
            break;
        case 3:
            // Three-character html-like color codes.
            numItems = sscanf(s.c_str(), "%1hhx%1hhx%1hhx%n", &m_R, &m_G, &m_B,
                    &numChars);
            m_R *= 17;
            m_G *= 17;
            m_B *= 17;
            break;
        default:
            numItems = 0;
    }
    if (s.length() != (unsigned)numChars || numItems != 3) {
        throw(Exception(AVG_ERR_INVALID_ARGS, string("Invalid color string '")+s+"'."));
    }
}

Color::Color(unsigned char r, unsigned char g, unsigned char b)
    : m_R(r),
      m_G(g),
      m_B(b)
{
}

Color::Color(const glm::vec3& v)
    : m_R((unsigned char)v[0]),
      m_G((unsigned char)v[1]),
      m_B((unsigned char)v[2])
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

Color::operator string() const
{
    if (m_sOrig == "") {
        stringstream s;
        s.fill('0');
        s << hex << std::uppercase << setw(2) << int(m_R) << setw(2) << int(m_G)
                << setw(2) << int(m_B);
        return s.str();
    } else {
        return m_sOrig;
    }
}

Color::operator glm::vec3() const
{
    return glm::vec3(m_R, m_G, m_B);
}

bool Color::operator ==(const Color& c) const
{
    return m_R == c.m_R && m_G == c.m_G && m_B == c.m_B;
}

bool Color::operator !=(const Color& c) const
{
    return m_R != c.m_R || m_G != c.m_G || m_B != c.m_B;
}

Color Color::mix(const Color& c1, const Color& c2, float ratio)
{
    LchColor lch1 = RGB2Lch(c1);
    LchColor lch2 = RGB2Lch(c2);

    // If one of the colors is grey, the hue is random. So, we set the hue to the 
    // other color's hue to avoid random colors.
    if (lch1.c < 1) {
        lch1.h = lch2.h;
    }
    if (lch2.c < 1) {
        lch2.h = lch1.h;
    }

    float hueDist = lch2.h - lch1.h;
    if (hueDist > 180) {
        lch2.h -= 360;
    } else if (hueDist < -180) {
        lch1.h -= 360;
    }

    LchColor lchMix(lch1.l*ratio + lch2.l*(1-ratio), lch1.c*ratio + lch2.c*(1-ratio),
            lch1.h*ratio + lch2.h*(1-ratio));
    return Lch2RGB(lchMix);
}

Color Color::fromLch(float L, float C, float H)
{
    LchColor lch(L, C, H);
    return Lch2RGB(lch);
}

std::ostream& operator <<(std::ostream& os, const Color& col)
{
    os << string(col);
    return os;
}


XYZColor::XYZColor(float X, float Y, float Z)
    : x(X),
      y(Y),
      z(Z)
{
}

LabColor::LabColor(float L, float A, float B)
    : l(L),
      a(A),
      b(B)
{
}

LchColor::LchColor(float L, float C, float H)
    : l(L),
      c(C),
      h(H)
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
    if (r < 0) {
        r = 0;
    }
    if (r > 1) {
        r = 1;
    }
    if (g > 0.0031308f) {
        g = 1.055f*pow(g, (1/2.4f)) - 0.055f;
    } else {
        g = 12.92f*g;
    }
    if (g < 0) {
        g = 0;
    }
    if (g > 1) {
        g = 1;
    }
    if (b > 0.0031308f) {
        b = 1.055f*pow(b, (1/2.4f)) - 0.055f;
    } else {
        b = 12.92f*b;
    }
    if (b < 0) {
        b = 0;
    }
    if (b > 1) {
        b = 1;
    }

    return Color((unsigned char)(r*255+0.5f), (unsigned char)(g*255+0.5f), 
            (unsigned char)(b*255+0.5f));
}

LabColor XYZ2Lab(const XYZColor& xyz)
{
    float x = xyz.x/95.047f;
    float y = xyz.y/100.000f;
    float z = xyz.z/108.883f;

    if (x > 0.008856f) {
        x = pow(x, 0.333333f);
    } else {
        x = (7.787f*x) + (16.f/116.f);
    }
    if (y > 0.008856f) {
        y = pow(y, 0.333333f);
    } else {
        y = (7.787f*y) + (16.f/116.f);
    }
    if (z > 0.008856f) {
        z = pow(z, 0.333333f);
    } else {
        z = (7.787f*z) + (16.f/116.f);
    }

    return LabColor((116*y)-16, 500*(x-y), 200*(y-z));
}

XYZColor Lab2XYZ(const LabColor& lab)
{
    float y = (lab.l+16)/116.f;
    float x = lab.a/500.f + y;
    float z = y - lab.b/200.f;

    if (pow(y, 3) > 0.008856) {
        y = pow(y, 3);
    } else {
        y = (y-16.f/116.f)/7.787f;
    }
    if (pow(x, 3) > 0.008856) {
        x = pow(x, 3);
    } else {
        x = (x-16.f/116.f)/7.787f;
    }
    if (pow(z, 3) > 0.008856) {
        z = pow(z, 3);
    } else {
        z = (z-16.f/116.f)/7.787f;
    }

    return XYZColor(x*95.047f, y*100.000f, z*108.883f);
}

LchColor Lab2Lch(const LabColor& lab)
{
    float h = atan2f(lab.b, lab.a);

    if (h > 0) {
        h = (h/float(M_PI))*180;
    } else {
        h = 360 - (fabsf(h)/float(M_PI))*180;
    }

    return LchColor(lab.l, sqrtf(lab.a*lab.a+lab.b*lab.b), h);
}

LabColor Lch2Lab(const LchColor& lch)
{
    float radH = lch.h*float(M_PI)/180.f;
    return LabColor(lch.l, cosf(radH)*lch.c, sinf(radH)*lch.c);
}

LchColor RGB2Lch(const Color& rgb)
{
    XYZColor xyz = RGB2XYZ(rgb);
    LabColor lab = XYZ2Lab(xyz);
    return Lab2Lch(lab);
}

Color Lch2RGB(const LchColor& lch)
{
    LabColor lab = Lch2Lab(lch);
    XYZColor xyz = Lab2XYZ(lab);
    return XYZ2RGB(xyz);
}


}
