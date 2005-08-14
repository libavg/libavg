//
// $Id$
//

#ifndef _Pixel16_H_
#define _Pixel16_H_

#include "Pixeldefs.h"

#include <stdlib.h>
#include <math.h>

// 16 bit pixel class. A pixel in this class contains 5 bits of
// red, 6 of green and 5 of blue (in that order). 
class Pixel16
{
  public:
    Pixel16 ();
    Pixel16 (unsigned char r, unsigned char g, unsigned char b);
    void Set (unsigned char r, unsigned char g, unsigned char b);
    void SetR (unsigned char r);
    void SetG (unsigned char g);
    void SetB (unsigned char b);
    unsigned char getR () const;
    unsigned char getG () const;
    unsigned char getB () const;

    Pixel16 operator = (const Pixel32& Pix);
    operator Pixel32 () const;
    Pixel16 operator = (const Pixel24& Pix);
    operator Pixel24 () const;
    
    bool operator ==(const Pixel16 Pix) const;

    bool operator !=(const Pixel16 Pix) const;

    // Simple and fast 'distance' between two pixels. Just adds the
    // distances between the color components and treats colors
    // equally.
    int BoxDist (const Pixel16 Pix) const;

  private:
    unsigned short m_Data;
};

inline Pixel16::Pixel16()
{
}


inline Pixel16::Pixel16(unsigned char r, unsigned char g, unsigned char b)
{
  Set (r, g, b);
}


inline void Pixel16::Set (unsigned char r, unsigned char g, unsigned char b)
{
#ifdef PIXEL_BGRA_ORDER
  m_Data = ((r&0xF8) << 8) | ((g&0xFC) << 3) | (b>>3);
#else
  m_Data = (b&0xF8) << 8 | ((g&0xFC) << 3) | (r>>3);
#endif  
}

inline void Pixel16::SetR(unsigned char r)
{
#ifdef PIXEL_BGRA_ORDER
  m_Data = (m_Data&0x07FF)|((r&0xF8)<<8);
#else
  m_Data = (m_Data&0xFFE0)|(r>>3);
#endif
}

inline void Pixel16::SetG(unsigned char g)
{
  m_Data = (m_Data&0xF81F)|((g&0xFC)<<3);
}


inline void Pixel16::SetB(unsigned char b)
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

inline Pixel16 Pixel16::operator = (const Pixel32& Pix)
{
  Set (Pix.getR(), Pix.getG(), Pix.getB());
  return *this;
}

inline Pixel16::operator Pixel32 () const
{
  // TODO: Make faster.
  return Pixel32 (getR(), getG(), getB(), 255);
}

inline Pixel16 Pixel16::operator = (const Pixel24& Pix)
{
  Set (Pix.getR(), Pix.getG(), Pix.getB());

  return *this;
}

inline Pixel16::operator Pixel24 () const
{
  // TODO: Make faster.  
  return Pixel24 (getR(), getG(), getB());
}

inline int Pixel16::BoxDist (const Pixel16 Pix) const
{
  return (abs ((int)getR()-Pix.getR()) +
          abs ((int)getG()-Pix.getG()) +
          abs ((int)getB()-Pix.getB()));
}

inline bool Pixel16::operator ==(const Pixel16 Pix) const
{
  return (*(const short *)this == *(const short*)&Pix);
}

inline bool Pixel16::operator !=(const Pixel16 Pix) const
{
  return (!(*this == Pix));
}


#endif
