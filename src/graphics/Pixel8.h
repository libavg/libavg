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

#ifndef _Pixel8_H_
#define _Pixel8_H_

#include "Pixeldefs.h"
#include "Pixel32.h"

#include <stdlib.h>
#include <assert.h>

class Pixel8
{
  public:
    Pixel8 ();
    Pixel8 (unsigned char i);
    void set (unsigned char i);
    unsigned char get () const;
    unsigned char getR () const;
    unsigned char getG () const;
    unsigned char getB () const;
    void flipRB();

    template<class SrcPixel>
    Pixel8 operator = (const SrcPixel& Pix)
    {
        set ((Pix.getR()*54+Pix.getG()*183+Pix.getB()*19)/256);
        return *this;
    }
   
    Pixel8 operator = (const Pixel8& Pix)
    {
        set (Pix.get());
        return *this;
    }

    operator Pixel32 () const;

    bool operator ==(const Pixel8&) const;
    bool operator !=(const Pixel8&) const;
    void operator +=(const Pixel8&);
    void operator -=(const Pixel8&);

    // Simple and fast 'distance' between two pixels. Just adds the
    // distances between the color components and treats colors
    // equally.
    int boxDist (const Pixel8 Pix) const;

    // Returns a weighed average between two pixels. Factor must be 
    // between 0 and 256. Factor=256 means Pix1 is the result, Factor=0 
    // means Pix2 is the result.
    static Pixel8 Blend (int Factor, const Pixel8 Pix1, 
                            const Pixel8 Pix2);

  private:
    unsigned char m_i;
};

inline Pixel8::Pixel8()
{
}

inline Pixel8::Pixel8(unsigned char i)
{
  set (i);
}

inline void Pixel8::set(unsigned char i)
{
  m_i = i;
}

inline unsigned char Pixel8::get() const
{
  return m_i;
}

inline unsigned char Pixel8::getR() const
{
  return m_i;
}

inline unsigned char Pixel8::getG() const
{
  return m_i;
}

inline unsigned char Pixel8::getB() const
{
  return m_i;
}

inline int Pixel8::boxDist (const Pixel8 Pix) const
{
  return (abs ((int)get()-Pix.get()));
}

inline Pixel8 Pixel8::Blend (int Factor, const Pixel8 Pix1, const Pixel8 Pix2)
{
// The following causes a 'C1055: compiler limit : out of keys' error on MSVC 2003
//  assert(Factor >= 0 && Factor <= 256);

  return Pixel8 ((Pix1.get()*Factor+Pix2.get()*(256-Factor))>>8);
}

inline Pixel8::operator Pixel32 () const
{
  return Pixel32 (m_i, m_i, m_i, 255);
}

inline bool Pixel8::operator ==(const Pixel8& Pix) const
{
  return (get() == Pix.get());
}

inline bool Pixel8::operator !=(const Pixel8& Pix) const
{
  return (!(*this == Pix));
}

inline void Pixel8::operator += (const Pixel8& Pix)
{
  m_i += Pix.m_i;
}

inline void Pixel8::operator -= (const Pixel8& Pix)
{
  m_i -= Pix.m_i;
}

#endif
