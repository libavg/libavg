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

#ifndef _Pixel16_H_
#define _Pixel16_H_

#include "../api.h"
#include "Pixeldefs.h"
#include "Pixel24.h"
#include "Pixel32.h"

#include <stdlib.h>
#include <math.h>

namespace avg {

// 16 bit pixel class. A pixel in this class contains 5 bits of
// red, 6 of green and 5 of blue (in that order). 
class AVG_API Pixel16
{
public:
    Pixel16();
    Pixel16(unsigned char r, unsigned char g, unsigned char b);
    void set(unsigned char r, unsigned char g, unsigned char b);
    void setR(unsigned char r);
    void setG(unsigned char g);
    void setB(unsigned char b);
    unsigned char getR() const;
    unsigned char getG() const;
    unsigned char getB() const;

    Pixel16 operator =(const Pixel32& pix);
    operator Pixel32() const;
    Pixel16 operator =(const Pixel24& pix);
    operator Pixel24() const;
    
    bool operator ==(const Pixel16 pix) const;
    bool operator !=(const Pixel16 pix) const;

    // Simple and fast 'distance' between two pixels. Just adds the
    // distances between the color components and treats colors
    // equally.
    int boxDist(const Pixel16 pix) const;

private:
    unsigned short m_Data;
};

inline Pixel16::Pixel16()
{
}


inline Pixel16::Pixel16(unsigned char r, unsigned char g, unsigned char b)
{
    set(r, g, b);
}


inline void Pixel16::set(unsigned char r, unsigned char g, unsigned char b)
{
#ifdef PIXEL_BGRA_ORDER
    m_Data = ((r&0xF8) << 8) | ((g&0xFC) << 3) | (b>>3);
#else
    m_Data = (b&0xF8) << 8 | ((g&0xFC) << 3) | (r>>3);
#endif  
}

inline void Pixel16::setR(unsigned char r)
{
#ifdef PIXEL_BGRA_ORDER
    m_Data = (m_Data&0x07FF)|((r&0xF8)<<8);
#else
    m_Data = (m_Data&0xFFE0)|(r>>3);
#endif
}

inline void Pixel16::setG(unsigned char g)
{
    m_Data = (m_Data&0xF81F)|((g&0xFC)<<3);
}


inline void Pixel16::setB(unsigned char b)
{
#ifdef PIXEL_BGRA_ORDER
    m_Data = (m_Data&0xFFE0)|(b>>3);
#else
    m_Data = (m_Data&0x07FF)|((b&0xF8)<<8);
#endif
}

inline unsigned char Pixel16::getR() const
{
#ifdef PIXEL_BGRA_ORDER
    return (m_Data&0xF800)>>8;
#else
    return (m_Data&0x001F)<<3;
#endif  
}


inline unsigned char Pixel16::getG() const
{
    return (m_Data&0x07E0)>>3;
}

inline unsigned char Pixel16::getB() const
{
#ifdef PIXEL_BGRA_ORDER
    return (m_Data&0x001F)<<3;
#else
    return (m_Data&0xF800)>>8;
#endif  
}

inline Pixel16 Pixel16::operator =(const Pixel32& pix)
{
    set(pix.getR(), pix.getG(), pix.getB());
    return *this;
}

inline Pixel16::operator Pixel32() const
{
    // TODO: Make faster.
    return Pixel32 (getR(), getG(), getB(), 255);
}

inline Pixel16 Pixel16::operator =(const Pixel24& pix)
{
    set(pix.getR(), pix.getG(), pix.getB());

    return *this;
}

inline Pixel16::operator Pixel24() const
{
    // TODO: Make faster.  
    return Pixel24(getR(), getG(), getB());
}

inline int Pixel16::boxDist(const Pixel16 pix) const
{
    return (abs ((int)getR()-pix.getR()) + abs ((int)getG()-pix.getG()) +
            abs ((int)getB()-pix.getB()));
}

inline bool Pixel16::operator ==(const Pixel16 pix) const
{
    return (*(const short *)this == *(const short*)&pix);
}

inline bool Pixel16::operator !=(const Pixel16 pix) const
{
    return (!(*this == pix));
}


}
#endif
