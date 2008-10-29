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

#ifndef _Filter_H_
#define _Filter_H_

#include "../api.h"
#include "Bitmap.h"

#include <boost/shared_ptr.hpp>

namespace avg {

// Base class for filters that operate on bitmaps. Derived classes need
// to override either the ApplyInPlace or the Apply function. The base-class
// versions of these functions simply implement one function in terms of the
// other.
class AVG_API Filter
{
public:
  Filter();
  virtual ~Filter();

  // In-Place Apply. Applies the filter to pBmp. The base-class
  // version copies the bitmap after calling Apply (pBmp, pTempBmp).
  virtual void applyInPlace(BitmapPtr pBmp);

  // Applies the Filter to pBmpSource and returns the result
  // The base-class version copies the bitmap before calling
  // applyInPlace.
  virtual BitmapPtr apply(BitmapPtr pBmpSource);
};

typedef boost::shared_ptr<Filter> FilterPtr;

} // namespace
#endif

