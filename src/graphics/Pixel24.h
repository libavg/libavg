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

#ifndef _Pixel24_H_
#define _Pixel24_H_

#include "Pixeldefs.h"
#include "Pixel32.h"
#include "Pixel8.h"

#include <stdlib.h>
#include <assert.h>

namespace avg {
    
class Pixel24
{
  public:
    Pixel24 ();
    Pixel24 (unsigned char r, unsigned char g, unsigned char b);
    void set (unsigned char r, unsigned char g, unsigned char b);
    void setR (unsigned char r);
    void setG (unsigned char g);
    void setB (unsigned char b);
    unsigned char getR () const;
    unsigned char getG () const;
    unsigned char getB () const;
    void flipRB();

    template<class SrcPixel>
    Pixel24 operator = (const SrcPixel& Pix)
    {
        setR (Pix.getR());
        setG (Pix.getG());
        setB (Pix.getB());

        return *this;
    }

    Pixel24 operator = (const Pixel8& Pix)
    {
        m_Data[0] = Pix.get();
        m_Data[1] = m_Data[0];
        m_Data[2] = m_Data[0];
        return *this;
    }
    

    operator Pixel32 () const;

    bool operator ==(const Pixel24&) const;
    bool operator !=(const Pixel24&) const;
    void operator +=(const Pixel24&);
    void operator -=(const Pixel24&);

    // Simple and fast 'distance' between two pixels. Just adds the
    // distances between the color components and treats colors
    // equally.
    int boxDist (const Pixel24 Pix) const;

    // Returns a weighed average between two pixels. Factor must be 
    // between 0 and 256. Factor=256 means Pix1 is the result, Factor=0 
    // means Pix2 is the result.
    static Pixel24 Blend (int Factor, const Pixel24 Pix1, 
                            const Pixel24 Pix2);

  private:
    unsigned char m_Data[3];
};

inline Pixel24::Pixel24()
{
}

inline Pixel24::Pixel24(unsigned char r, unsigned char g, unsigned char b)
{
  set (r, g, b);
}

inline void Pixel24::set(unsigned char r, unsigned char g, unsigned char b)
{
  m_Data[REDPOS] = r;
  m_Data[GREENPOS] = g;
  m_Data[BLUEPOS] = b;
}

inline void Pixel24::setR(unsigned char r)
{
  m_Data[REDPOS] = r;
}

inline void Pixel24::setG(unsigned char g)
{
  m_Data[GREENPOS] = g;
}

inline void Pixel24::setB(unsigned char b)
{
  m_Data[BLUEPOS] = b;
}

inline unsigned char Pixel24::getR() const
{
  return m_Data[REDPOS];
}

inline unsigned char Pixel24::getG() const
{
  return m_Data[GREENPOS];
}

inline unsigned char Pixel24::getB() const
{
  return m_Data[BLUEPOS];
}

inline void Pixel24::flipRB() 
{
    unsigned char tmp = m_Data[BLUEPOS];
    m_Data[BLUEPOS] = m_Data[REDPOS];
    m_Data[REDPOS] = tmp;
}

inline int Pixel24::boxDist (const Pixel24 Pix) const
{
  return (abs ((int)getR()-Pix.getR()) +
          abs ((int)getG()-Pix.getG()) +
          abs ((int)getB()-Pix.getB()));
}

inline Pixel24 Pixel24::Blend (int Factor, const Pixel24 Pix1, const Pixel24 Pix2)
{
  assert(Factor >= 0 && Factor <= 256);

  return Pixel24 ((Pix1.getR()*Factor+Pix2.getR()*(256-Factor))>>8,
                    (Pix1.getG()*Factor+Pix2.getG()*(256-Factor))>>8,
                    (Pix1.getB()*Factor+Pix2.getB()*(256-Factor))>>8);
}

inline Pixel24::operator Pixel32 () const
{
  return Pixel32 (getR(), getG(), getB(), 255);
}

inline bool Pixel24::operator ==(const Pixel24& Pix) const
{
  return (getR() == Pix.getR() && getG() == Pix.getG() && getB() == Pix.getB());
}

inline bool Pixel24::operator !=(const Pixel24& Pix) const
{
  return (!(*this == Pix));
}

inline void Pixel24::operator += (const Pixel24& Pix)
{
  m_Data[0] += Pix.m_Data[0];
  m_Data[1] += Pix.m_Data[1];
  m_Data[2] += Pix.m_Data[2];
}

inline void Pixel24::operator -= (const Pixel24& Pix)
{
  m_Data[0] -= Pix.m_Data[0];
  m_Data[1] -= Pix.m_Data[1];
  m_Data[2] -= Pix.m_Data[2];
}

}
#endif
