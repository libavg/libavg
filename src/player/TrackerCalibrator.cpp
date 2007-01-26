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

#include "TrackerCalibrator.h"

#include "TrackerEventSource.h"

using namespace std;

#define NUM_POINTS 5
#define MIN_DIST_FROM_BORDER 10

namespace avg {

    TrackerCalibrator::TrackerCalibrator(TrackerEventSource* pTracker, IntPoint DisplayExtents,
            CoordTransformerPtr pOrigTrafo)
        : m_pTracker(pTracker),
          m_CurPoint(0),
          m_pOrigTrafo(pOrigTrafo)
    {
        IntPoint OffsetPerPoint((DisplayExtents.x-MIN_DIST_FROM_BORDER*2)/(NUM_POINTS-1),
                (DisplayExtents.y-MIN_DIST_FROM_BORDER*2)/(NUM_POINTS-1));
        for (int y=0; y<NUM_POINTS; y++) {
            for (int x=0; x<NUM_POINTS; x++) {
                m_DisplayPoints.push_back(
                    IntPoint(OffsetPerPoint.x*x+MIN_DIST_FROM_BORDER, 
                            OffsetPerPoint.y*y+MIN_DIST_FROM_BORDER));
                m_CamPoints.push_back(DPoint(0,0));
            }
        }
        // TODO
        // CoordTransformerPtr pIdentTrafo(new IdentityTransformer);
        // m_pTracker->setCoordTransformer(pIdentTrafo);
    }

    TrackerCalibrator::~TrackerCalibrator()
    {
    }


    bool TrackerCalibrator::nextPoint()
    {
        m_CurPoint++;
        if (m_CurPoint < m_DisplayPoints.size()) {
            return true;
        } else {
            m_pTracker->calibrate(m_DisplayPoints, m_CamPoints);
            return false;
        }
    }

    int TrackerCalibrator::getDisplayPointX()
    {
        return m_DisplayPoints[m_CurPoint].x;
    }

    int TrackerCalibrator::getDisplayPointY()
    {
        return m_DisplayPoints[m_CurPoint].y;
    }

    void TrackerCalibrator::setCamPoint(double x, double y)
    {
        m_CamPoints[m_CurPoint] = DPoint(x, y);
    }

    void TrackerCalibrator::abort()
    {
        // TODO: Reset tracker calibration data.
        // m_pTracker->setCoordTransformer(pOrigTrafo);
    }

}
