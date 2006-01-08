//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2006 Ulrich von Zadow
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
/*
/--------------------------------------------------------------------
|
|      $Id$
|
|      Copyright (c) 1996-2002 Ulrich von Zadow
|
\--------------------------------------------------------------------
*/

#ifndef INCL_PLPIXEL8
#define INCL_PLPIXEL8

#include "plpixeldefs.h"

#include <stdlib.h>

//! 8 bit pixel class. A pixel in this class contains 8 bits of
//! image data. This class is meant to be
//! fast, so all methods are inlined.
class PLPixel8
{
  public:
    //!
    PLPixel8 ();
    //!
    PLPixel8 (PLBYTE val);
    //!
    void Set (PLBYTE val);
    //!
    PLBYTE Get () const;

    //!
    bool operator ==(const PLPixel8&);

    //!
    bool operator !=(const PLPixel8&);

    //! Simple and fast 'distance' between two pixels.
    int BoxDist (PLPixel8 Pix);

  private:
    PLBYTE m_Data;
};

inline PLPixel8::PLPixel8()
{
}


inline PLPixel8::PLPixel8(PLBYTE val)
{
  Set (val);
}


inline void PLPixel8::Set(PLBYTE val)
{
  m_Data = val;
}


inline PLBYTE PLPixel8::Get() const
{
  return m_Data;
}


inline int PLPixel8::BoxDist (PLPixel8 Pix)
{
  return abs ((int)Get()-Pix.Get());
}

inline bool PLPixel8::operator ==(const PLPixel8& Pix)
{
  return (Get() == Pix.Get());
}

inline bool PLPixel8::operator !=(const PLPixel8& Pix)
{
  return (!(*this == Pix));
}


#endif

/*
/--------------------------------------------------------------------
|
|      $Log$
|      Revision 1.2  2006/01/08 19:59:24  uzadow
|      Added correct license information to all files.
|
|      Revision 1.1  2005/08/14 21:56:25  uzadow
|      Added graphics/ directory. R<->B bugs still present.
|
|      Revision 1.4  2004/09/17 13:45:25  artcom
|      fixed some linux build glitches
|
|      Revision 1.3  2002/02/24 13:00:37  uzadow
|      Documentation update; removed buggy PLFilterRotate.
|
|      Revision 1.2  2001/10/06 22:03:26  uzadow
|      Added PL prefix to basic data types.
|
|      Revision 1.1  2001/09/16 19:03:22  uzadow
|      Added global name prefix PL, changed most filenames.
|
|      Revision 1.1  2001/09/13 20:45:35  uzadow
|      Added 8-bpp pixel class.
|
|
\--------------------------------------------------------------------
*/
