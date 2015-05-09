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

#ifndef _Color_H_
#define _Color_H_

#include "../api.h"

#include <Pixel32.h>
#include <string>

namespace avg {

class AVG_API Color
{
public:
    Color(const std::string& s);
    Color(unsigned char r, unsigned char g, unsigned char b);
    ~Color();

    unsigned char getR() const;
    unsigned char getG() const;
    unsigned char getB() const;

    void setR(unsigned char r);
    void setG(unsigned char g);
    void setB(unsigned char b);

    operator Pixel32() const;
    operator std::string() const;
    bool operator ==(const Color& c) const;

    static Color mix(const Color& c1, const Color& c2, float ratio);

private:
    unsigned char m_R;
    unsigned char m_G;
    unsigned char m_B;

    std::string m_sOrig;
};

struct AVG_API XYZColor
{
    XYZColor(float X, float Y, float Z);

    float x; // Range: 0 ... 95.047
    float y; // Range: 0 ... 100.000
    float z; // Range: 0 ... 108.883
};

struct AVG_API LabColor
{
    LabColor(float L, float A, float B);

    float l;
    float a;
    float b;
};

struct AVG_API LchColor
{
    LchColor(float L, float C, float H);

    float l;
    float c;
    float h;
};

XYZColor RGB2XYZ(const Color& rgb);
Color XYZ2RGB(const XYZColor& xyz);
LabColor XYZ2Lab(const XYZColor& xyz);
XYZColor Lab2XYZ(const LabColor& lab);
LchColor Lab2Lch(const LabColor& lab);
LabColor Lch2Lab(const LchColor& lch);

}
 
#endif
