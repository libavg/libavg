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
#include "CalibratorDataType"
#include "TrackerEventSource.h"

using namespace std;

#define NUM_POINTS 5
#define MIN_DIST_FROM_BORDER 10

namespace avg {
//void lm_print_tracker( int n_par, double* par, int m_dat, double* fvec,
//                               void *data, int iflag, int iter, int nfev );
  struct {
              std::vector<IntPoint> DisplayPoints;
              std::vector<DPoint> CamPoints;
              CoordTransformerPtr CurrentTrafo;
              //double (*user_func)( std::vector<IntPoint> &DisplayPoints, std::vector<DPoint> &CamPoints);
  } CalibratorDataType;


void lm_evaluate_tracker( double* par, int m_dat, double* fvec,
                                  void *data, int *info ) {
    int i;
    CalibratorDataType *mydata;
    mydata = static_cast<CalibratorDataType*> data;
    
    mydata->CurrentTransform = DeDistort();//FIXME
    for (i=0; i<m_dat; i++){
        fvec[i] = calcDist(mydata->DisplayPoint[i], mydata->CurrentCoord->transform_point(mydata->CamPoint)); 
    }
    *info = *info; /* to prevent a 'unused variable' warning */
    /* if <parameters drifted away> { *info = -1; } */

}


    TrackerCalibrator::TrackerCalibrator(TrackerEventSource* pTracker, IntPoint DisplayExtents,
            CoordTransformerPtr pOrigTrafo)
        : m_pTracker(pTracker),
          m_CurPoint(0),
          m_pOrigTrafo(pOrigTrafo),
          m_bCurPointSet(false)
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
/*
        cerr << "Calibration done. Number of points: " << m_DisplayPoints.size() << endl;
        for (unsigned int i=0; i<m_DisplayPoints.size(); ++i) {
            cerr << "  " << m_DisplayPoints[i] << "-->" << m_CamPoints[i] << endl;
        }
*/
    }


    bool TrackerCalibrator::nextPoint()
    {
        if (!m_bCurPointSet) {
            // There is no data for the previous point, so delete it.
            m_DisplayPoints.erase(m_DisplayPoints.begin()+m_CurPoint);
            m_CamPoints.erase(m_CamPoints.begin()+m_CurPoint);
        } else {
            m_CurPoint++;
        }
        m_bCurPointSet = false;
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
        m_bCurPointSet = true;
    }

    void TrackerCalibrator::abort()
    {
        // TODO: Reset tracker calibration data.
        // m_pTracker->setCoordTransformer(pOrigTrafo);
    }

    void TrackerEventSource::calibrate()
    {
        lm_control_type control;
        CalibratorDataType data;
        
        data->DisplayPoints = m_DisplayPoints;
        data->CamPoints = m_CamPoints;
        data->CurrentTrafo = CoordTransformerPtr();
        
        lm_initialize_control( &control );
        int m_dat = m_DisplayPoints.length();
        assert(m_dat == m_CamPoints.length());
        
        DPoint center = m_pTracker->mTrackerConfig.m_ROI.center();
        double w = m_pTracker->mTrackerConfig.m_ROI.width();
        double h = m_pTracker->mTrackerConfig.m_ROI.height();
        DPoint FilmDisplacement= -DPoint(320,240); 
        DPoint FilmScale = DPoint(w/2.,h/2.);
        std::vector<double> DistortionParams;
        DistortionParams.push_back(0.4);
        DistortionParams.push_back(0.0);
        DPoint3& P = DPoint(0,0,0); 
        DPoint3& N = DPoint(0,0,1); 
        double Angle = 0;
        const DPoint& DisplayDisplacement=DPoint(-640,-360);
        const DPoint& DisplayScale = DPoint(640,360);

        int n_p = 11;
        double[] p = {
            DisplayDisplacement.x, 
            DisplayDisplacement.y, 
            DisplayScale.x, 
            DisplayScale.y,
            DistortionParams[0],
            DistortionParams[1],
            N.z,
            P.x,
            P.y,
            P.z,
            Angle
        };
        lm_minimize( m_dat, n_p, p, lm_evaluate_tracker, lm_print_default,
                     &data, &control );
//        double[] p = {
//            DisplayDisplacement.x, 
//            DisplayDisplacement.y, 
//            DisplayScale.x, 
//            DisplayScale.y,
//            DistortionParams[0],
//            DistortionParams[1],
//            N.z,
//            P.x,
//            P.y,
//            P.z,
//            Angle
//        };
        m_pTracker->m_TrackerConfig.m_Trafo = data->CurrentTrafo;
        m_pTracker->setConfig();
        // TODO
    }
}
