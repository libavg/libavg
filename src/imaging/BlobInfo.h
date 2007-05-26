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
//  Original author of this file is igor@c-base.org 
//

#ifndef _BlobInfo_H_
#define _BlobInfo_H_

#include "../graphics/Point.h"
#include "../graphics/Rect.h"

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include <vector>

namespace avg {

struct BlobInfo;

typedef boost::shared_ptr<BlobInfo> BlobInfoPtr;
typedef boost::weak_ptr<BlobInfo> BlobInfoWeakPtr;
typedef std::vector<BlobInfoWeakPtr> BlobInfoVector;

struct BlobInfo
{
    int m_ID;
    DPoint m_Center;
    double m_Area;
    IntRect m_BoundingBox;
    double m_Eccentricity;
    double m_Inertia;
    double m_Orientation;
    DPoint m_ScaledBasis[2];
    DPoint m_EigenVectors[2];
    DPoint m_EigenValues;

    BlobInfoVector m_RelatedBlobs; // For fingers, this contains the hand.
                                   // For hands, this contains the fingers.
    DPoint m_Direction; // Fingers only: contains vector from hand center to finger.
};


}

#endif
