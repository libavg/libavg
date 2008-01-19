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

#ifndef _ConnectedComps_H_
#define _ConnectedComps_H_

#include "Run.h"

#include "../graphics/Bitmap.h"
#include "../graphics/Pixel32.h"

#include "../base/Point.h"

#include <assert.h>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace avg {

class Blob;
typedef boost::shared_ptr<class Blob> BlobPtr;
typedef boost::weak_ptr<class Blob> BlobWeakPtr;
typedef std::vector<BlobPtr> BlobVector;
typedef std::vector<BlobWeakPtr> BlobWeakPtrVector;
typedef boost::shared_ptr<BlobVector> BlobVectorPtr;
typedef std::vector<IntPoint> ContourSeq;

class Blob
{
    public:
        Blob(const Run & run);
        ~Blob();

        void addRun(const Run & run);
        void merge(const BlobPtr& other);
        RunArray* getRuns();
        void render(BitmapPtr pSrcBmp, BitmapPtr pDestBmp, Pixel32 Color, 
                int Min, int Max, bool bFinger, bool bMarkCenter, 
                Pixel32 CenterColor= Pixel32(0x00, 0x00, 0xFF, 0xFF));
        bool contains(IntPoint pt);

        void calcStats();
        void calcContour(int NumPoints);
        ContourSeq getContour();

        const DPoint& getCenter() const;
        double getArea() const;
        const IntRect & getBoundingBox() const;
        double getEccentricity() const;
        double getInertia() const;
        double getOrientation() const;
        const DPoint& getScaledBasis(int i) const;
        const DPoint& getEigenVector(int i) const;
        const DPoint& getEigenValues() const;

        void clearRelated();
        void addRelated(BlobPtr pBlob);
        const BlobPtr getFirstRelated(); 

        BlobPtr m_pParent;

    private:
        Blob(const Blob &);
        DPoint calcCenter();
        IntRect calcBBox();
        int calcArea();
        IntPoint findNeighborInside(const IntPoint& Pt, int& Dir);
        bool ptInBlob(const IntPoint& Pt);

        RunArray m_Runs;
        BlobWeakPtrVector m_RelatedBlobs; // For fingers, this contains the hand.
                                          // For hands, this contains the fingers.

        bool m_bStatsAvailable;
        DPoint m_Center;
        double m_Area;
        IntRect m_BoundingBox;
        double m_Eccentricity;
        double m_Inertia;
        double m_Orientation;
        DPoint m_ScaledBasis[2];
        DPoint m_EigenVector[2];
        DPoint m_EigenValues;

        ContourSeq m_Contour;
};

BlobVectorPtr connected_components(BitmapPtr image, unsigned char object_threshold);

}

#endif
