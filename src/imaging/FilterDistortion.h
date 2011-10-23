//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2011 Ulrich von Zadow
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
//  Original author of this file is igor@c-base.org 
//

#ifndef _FilterDistortion_H_
#define _FilterDistortion_H_

#include "../api.h"
#include "CoordTransformer.h"

#include "../graphics/Filter.h"

#include "../base/GLMHelper.h"
#include "../base/Rect.h"

#include <boost/shared_ptr.hpp>

namespace avg {

// TODO: This class doesn't have a unit test.
class AVG_API FilterDistortion: public Filter 
{
    public:
        FilterDistortion(const IntPoint& srcSize, CoordTransformerPtr pTransformer);
        virtual ~FilterDistortion();
        BitmapPtr apply(BitmapPtr pBmpSource);
    private:
        IntPoint m_SrcSize;
        CoordTransformerPtr m_pTransformer;
        IntPoint* m_pMap;
};

typedef boost::shared_ptr<FilterDistortion> FilterDistortionPtr;

}

#endif
