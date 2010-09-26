//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2008 Ulrich von Zadow
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

#ifndef _Pixel32_H_
#define _Pixel32_H_

#include "../api.h"

#include "Pixeldefs.h"

#include <string>
#include <math.h>
#include <stdlib.h>

namespace avg {

class Pixel32
{

public:
    Pixel32();
    Pixel32(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
    Pixel32(unsigned char r, unsigned char g, unsigned char b);
    void set(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
    void set(unsigned char r, unsigned char g, unsigned char b);
    void setR(unsigned char r);
    void setG(unsigned char g);
    void setB(unsigned char b);
    void setA(unsigned char a);
    unsigned char getR() const;
    unsigned char getG() const;
    unsigned char getB() const;
    unsigned char getA() const;
    void flipRB();

    bool operator ==(const Pixel32 pix) const;
    bool operator !=(const Pixel32 pix) const;
    void operator +=(const Pixel32 pix);
    void operator -=(const Pixel32 pix);
    Pixel32 operator *(float f) const;
    operator unsigned int() const;

    // Simple and fast 'distance' between two pixels. Just adds the
    // distances between the color components and treats colors
    // equally.
    int boxDist(const Pixel32 pix) const;

    std::string AVG_API getColorString() const;

  private:
    unsigned char m_Data[4];
};

AVG_API std::ostream& operator <<(std::ostream& os, const Pixel32& pix);

AVG_API Pixel32 colorStringToColor(const std::string& s);

inline Pixel32::Pixel32()
{
}


inline Pixel32::Pixel32(unsigned char r, unsigned char g, unsigned char b, 
        unsigned char a)
{
  set (r, g, b, a);
}


inline Pixel32::Pixel32(unsigned char r, unsigned char g, unsigned char b)
{
  set (r, g, b, 255);
}


inline void Pixel32::set(unsigned char r, unsigned char g, unsigned char b, 
        unsigned char a)
{
  m_Data[REDPOS] = r;
  m_Data[GREENPOS] = g;
  m_Data[BLUEPOS] = b;
  m_Data[ALPHAPOS] = a;
}

inline void Pixel32::set(unsigned char r, unsigned char g, unsigned char b)
{
  m_Data[REDPOS] = r;
  m_Data[GREENPOS] = g;
  m_Data[BLUEPOS] = b;
}

inline void Pixel32::setR(unsigned char r)
{
  m_Data[REDPOS] = r;
}


inline void Pixel32::setG(unsigned char g)
{
  m_Data[GREENPOS] = g;
}


inline void Pixel32::setB(unsigned char b)
{
  m_Data[BLUEPOS] = b;
}

inline void Pixel32::setA(unsigned char a)
{
  m_Data[ALPHAPOS] = a;
}


inline unsigned char Pixel32::getR() const
{
  return m_Data[REDPOS];
}


inline unsigned char Pixel32::getG() const
{
  return m_Data[GREENPOS];
}


inline unsigned char Pixel32::getB() const
{
  return m_Data[BLUEPOS];
}


inline unsigned char Pixel32::getA() const
{
  return m_Data[ALPHAPOS];
}

inline void Pixel32::flipRB() 
{
    unsigned char tmp = m_Data[BLUEPOS];
    m_Data[BLUEPOS] = m_Data[REDPOS];
    m_Data[REDPOS] = tmp;
}

inline int Pixel32::boxDist(const Pixel32 pix) const
{
  return (abs ((int)getR()-pix.getR()) +
          abs ((int)getG()-pix.getG()) +
          abs ((int)getB()-pix.getB()));
}

inline bool Pixel32::operator ==(const Pixel32 pix) const
{
  return (*(const int *)this == *(const int*)&pix);
}

inline bool Pixel32::operator !=(const Pixel32 pix) const
{
  return (!(*this == pix));
}

inline void Pixel32::operator +=(const Pixel32 pix)
{
  m_Data[0] += pix.m_Data[0];
  m_Data[1] += pix.m_Data[1];
  m_Data[2] += pix.m_Data[2];
}

inline void Pixel32::operator -=(const Pixel32 pix)
{
  m_Data[0] -= pix.m_Data[0];
  m_Data[1] -= pix.m_Data[1];
  m_Data[2] -= pix.m_Data[2];
}

inline Pixel32 Pixel32::operator *(float f) const
{
  return Pixel32((unsigned char)(f*getR()), 
          (unsigned char)(f*getG()), 
          (unsigned char)(f*getB()));
}
    
inline Pixel32::operator unsigned int() const
{
    return *(unsigned int*)(m_Data);
}

}
#endif
