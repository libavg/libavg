//
//  libavg - Media Playback Engine.
//  Copyright (C) 2003-2021 Ulrich von Zadow
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

#ifndef _Filtergrayscale_H_
#define _Filtergrayscale_H_

#include "../api.h"
#include "Filter.h"
#include "Bitmap.h"

namespace avg {

// Creates a grayscale version of a 32 bpp bitmap.
class AVG_API FilterGrayscale : public Filter
{
public:
  FilterGrayscale();
  virtual ~FilterGrayscale();
  virtual BitmapPtr apply(BitmapPtr pBmpSource);

private:
};

} // namespace

#endif

