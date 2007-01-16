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

#ifndef _TouchEvent_H_
#define _TouchEvent_H_

#include "CursorEvent.h"
#include "Node.h"

#include "../graphics/Point.h"
#include "../graphics/Bitmap.h"

#include "../imaging/ConnectedComps.h"

#include <math.h>

namespace avg {

class TouchEvent: public CursorEvent 
{
    public:
        TouchEvent(int id, Type EventType, BlobInfoPtr info, BlobPtr blob, 
                DPoint& Offset, DPoint& Scale);
        virtual Event* cloneAs(Type EventType);

        double getOrientation(){return m_Info->m_Orientation;};
        double getArea(){return m_Info->m_Area;};
        IntRect getBoundingBox(){return m_Info->m_BoundingBox;};
        double getInertia(){return m_Info->m_Inertia;};
        DPoint getCenter(){return m_Info->m_Center;};
        double getEccentricity(){return m_Info->m_Eccentricity;};
        DPoint getEigenValues(){return m_Info->m_EigenValues;};
        //BitmapPtr getBitmap();
        //DPoint[2] getScaledBasis(){return m_Info.m_ScaledBasis;};
        virtual void trace();
    
    protected:
        BlobInfoPtr m_Info;
        BlobPtr m_Blob;

    private:
        IntPoint transformPoint(DPoint& pt, DPoint& Offset, DPoint& Scale);
};

}
#endif
