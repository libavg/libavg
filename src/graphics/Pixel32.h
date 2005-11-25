//
// $Id$
//

#ifndef _Pixel32_H_
#define _Pixel32_H_

#include "Pixeldefs.h"

#include <math.h>
#include <stdlib.h>

class Pixel32
{

public:
    Pixel32 ();
    Pixel32 (unsigned char r, unsigned char g, unsigned char b, unsigned char a);
    Pixel32 (unsigned char r, unsigned char g, unsigned char b);
    void set (unsigned char r, unsigned char g, unsigned char b, unsigned char a);
    void set (unsigned char r, unsigned char g, unsigned char b);
    void setR (unsigned char r);
    void setG (unsigned char g);
    void setB (unsigned char b);
    void setA (unsigned char a);
    unsigned char getR () const;
    unsigned char getG () const;
    unsigned char getB () const;
    unsigned char getA () const;
    void flipRB();

    bool operator ==(const Pixel32 Pix) const;
    bool operator !=(const Pixel32 Pix) const;
    void operator +=(const Pixel32 Pix);
    Pixel32 operator *(float f) const;

    // Simple and fast 'distance' between two pixels. Just adds the
    // distances between the color components and treats colors
    // equally.
    int boxDist (const Pixel32 Pix) const;

    // Returns a weighed average between two pixels. Factor must be 
    // between 0 and 256. Factor=256 means Pix1 is the result, Factor=0 
    // means Pix2 is the result.
    static Pixel32 blend (int Factor, const Pixel32 Pix1, 
                            const Pixel32 Pix2);

  private:
    unsigned char m_Data[4];
};

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

//!
inline void Pixel32::set (unsigned char r, unsigned char g, unsigned char b)
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

inline int Pixel32::boxDist (const Pixel32 Pix) const
{
  return (abs ((int)getR()-Pix.getR()) +
          abs ((int)getG()-Pix.getG()) +
          abs ((int)getB()-Pix.getB()));
}

inline Pixel32 Pixel32::blend (int Factor, const Pixel32 Pix1, const Pixel32 Pix2)
{
  return Pixel32 ((Pix1.getR()*Factor+Pix2.getR()*(256-Factor))>>8,
                    (Pix1.getG()*Factor+Pix2.getG()*(256-Factor))>>8,
                    (Pix1.getB()*Factor+Pix2.getB()*(256-Factor))>>8,
                    Pix1.getA());
}

inline bool Pixel32::operator ==(const Pixel32 Pix) const
{
  return (*(const int *)this == *(const int*)&Pix);
}

inline bool Pixel32::operator !=(const Pixel32 Pix) const
{
  return (!(*this == Pix));
}

inline void Pixel32::operator += (const Pixel32 Pix)
{
  m_Data[0] += Pix.m_Data[0];
  m_Data[1] += Pix.m_Data[1];
  m_Data[2] += Pix.m_Data[2];
}

inline Pixel32 Pixel32::operator *(float f) const
{
  return Pixel32((unsigned char)(f*getR()), 
          (unsigned char)(f*getG()), 
          (unsigned char)(f*getB()));
}

#endif
