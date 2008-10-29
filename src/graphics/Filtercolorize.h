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

#ifndef _Filtercolorize_H_
#define _Filtercolorize_H_

#include "../api.h"
#include "Filter.h"

namespace avg {

// Filter that colorizes a bitmap given a hue and saturation. Corresponds
// loosely to the photoshop hue/saturation control when set to 'colorize'.
// The range of hue is 0..359, the range of saturation is 0..100.
class AVG_API FilterColorize : public Filter
{
public:
  FilterColorize(double Hue, double Saturation);
  virtual ~FilterColorize();
  virtual void applyInPlace(BitmapPtr pBmp) ;

private:
  double m_Hue;
  double m_Saturation;
};

}  // namespace

#endif

