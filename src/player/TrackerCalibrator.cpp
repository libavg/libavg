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

#include "../base/ObjectCounter.h"

#include "../imaging/DeDistort.h"

extern "C" {
#include "../lmfit/lmmin.h"
#include "../lmfit/lm_eval.h"
}

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace std;

#define NUM_POINTS 4 
#define MIN_DIST_FROM_BORDER 20
//#define DEBUG_FIT  1

namespace avg {

enum Params { DISPSCALE_X, DISPSCALE_Y, DISPOFFSET_X, DISPOFFSET_Y, DIST_2, DIST_3, 
            ANGLE, TRAPEZ, NUM_PARAMS};

void lm_print_tracker( int n_par, double* p, int m_dat, double* fvec, 
                       void *data, int iflag, int iter, int nfev )
/*
 *       data  : for soft control of printout behaviour, add control
 *                 variables to the data struct
 *       iflag : 0 (init) 1 (outer loop) 2(inner loop) -1(terminated)
 *       iter  : outer loop counter
 *       nfev  : number of calls to *evaluate
 */
{
    TrackerCalibrator *mydata;
    mydata = static_cast<TrackerCalibrator*>(data);
    mydata->print_tracker(n_par, p, m_dat, fvec, iflag, iter, nfev);

}

void lm_evaluate_tracker(double* p, int m_dat, double* fvec,
        void *data, int *info)
{
    TrackerCalibrator *mydata = static_cast<TrackerCalibrator*>(data);
    mydata->evaluate_tracker(p, m_dat, fvec, info);

}

TrackerCalibrator::TrackerCalibrator(const IntPoint& CamExtents, 
        const IntPoint& DisplayExtents)
    : m_CurPoint(0),
      m_CamExtents(CamExtents),
      m_DisplayExtents(DisplayExtents),
      m_bCurPointSet(false)
{
    ObjectCounter::get()->incRef(&typeid(*this));
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
}

TrackerCalibrator::~TrackerCalibrator()
{
    /*
       cerr << "Calibration done. Number of points: " << m_DisplayPoints.size() << endl;
       for (unsigned int i=0; i<m_DisplayPoints.size(); ++i) {
       cerr << "  " << m_DisplayPoints[i] << "-->" << m_CamPoints[i] << endl;
       }
     */
    ObjectCounter::get()->decRef(&typeid(*this));
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
        return false;
    }
}

IntPoint TrackerCalibrator::getDisplayPoint()
{
    return m_DisplayPoints[m_CurPoint];
}

void TrackerCalibrator::setCamPoint(const DPoint& pt)
{
    m_CamPoints[m_CurPoint] = pt;
    m_bCurPointSet = true;
}

DeDistortPtr TrackerCalibrator::makeTransformer()
{
    lm_control_type control;
    lm_initialize_control( &control );
    control.maxcall=1000;
    //        control.epsilon=1e-8;
    //        control.ftol = 1e-4;
    //        control.xtol = 1e-4;
    //        control.gtol = 1e-4;

    int dat = int(m_DisplayPoints.size());
    assert(dat == int(m_CamPoints.size()));

    //fill in reasonable defaults
    m_DistortParams.clear();
    m_DistortParams.push_back(0);
    m_DistortParams.push_back(0);
    m_Angle = 0;
    m_TrapezoidFactor = 0.0;
    m_DisplayOffset= DPoint(0,0);
    m_DisplayScale = DPoint(2,2);

    int n_p = NUM_PARAMS;
    //should really match the Params enum!!!!
    double p[] = {
        m_DisplayScale.x, 
        m_DisplayScale.y,
        m_DisplayOffset.x, 
        m_DisplayOffset.y, 
        m_DistortParams[0],
        m_DistortParams[1],
        m_Angle,
        m_TrapezoidFactor
    };
    initThisFromDouble(p);
#ifdef DEBUG_FIT
    for(unsigned j=0;j<m_DisplayPoints.size();j++) {
        cerr<<"(cam) "<<m_CamPoints[j]<<" -> (display) "<<m_DisplayPoints[j]<<"\n";
    }
#endif
    lm_minimize(dat, n_p, p, lm_evaluate_tracker, lm_print_tracker,
            this, &control );
    initThisFromDouble(p);
    return m_CurrentTrafo;
}

void TrackerCalibrator::initThisFromDouble(double *p)
{
    m_DisplayOffset.x = p[DISPOFFSET_X]; 
    m_DisplayOffset.y = p[DISPOFFSET_Y];
    m_DisplayScale.x = p[DISPSCALE_X];
    m_DisplayScale.y = p[DISPSCALE_Y];
    m_DistortParams.clear();
    m_DistortParams.push_back( p[DIST_2 ] );
    m_DistortParams.push_back( p[DIST_3]);
    m_Angle = p[ANGLE];
    m_TrapezoidFactor = p[TRAPEZ];
    m_CurrentTrafo = DeDistortPtr( 
            new DeDistort(DPoint(m_CamExtents),
                m_DistortParams,
                m_Angle,
                m_TrapezoidFactor,
                m_DisplayOffset,
                m_DisplayScale
                )
            );
}

void TrackerCalibrator::evaluate_tracker(double *p, int m_dat, double* fvec, int* info)
{
    initThisFromDouble(p);

#ifdef DEBUG_FIT
    for(int i=0;i<=12;i+=3) {
        DPoint screenPoint = m_CurrentTrafo->transformBlobToScreen(
                m_CurrentTrafo->transform_point(m_CamPoints[i]));
        cerr << "sample value of trafo of (cam) "
             << m_CamPoints[i]<<" : (transformed) "
             << screenPoint
             << "== (display)"
             << DPoint(m_DisplayPoints[i])
             << " dist="
             << calcDist(DPoint(m_DisplayPoints[i]), screenPoint) 
             << endl;
    }
#endif 
    for (int i=0; i<m_dat; i++){
        fvec[i] = calcDist(
                m_CurrentTrafo->transformBlobToScreen(
                    m_CurrentTrafo->transform_point(m_CamPoints[i])
                    ), 
                DPoint(m_DisplayPoints[i])
                ); 
    }
    *info = *info; /* to prevent a 'unused variable' warning */
    /* if <parameters drifted away> { *info = -1; } */
}

void TrackerCalibrator::print_tracker(int n_par, double *p, int m_dat, 
        double *fvec, int iflag, int iter, int nfev)
{
#ifdef DEBUG_FIT
    initThisFromDouble(p);
    if (iflag==2) {
        printf ("trying step in gradient direction\n");
    } else if (iflag==1) {
        printf ("determining gradient (iteration %d)\n", iter);
    } else if (iflag==0) {
        printf ("starting minimization\n");
    } else if (iflag==-1) {
        printf ("terminated after %d evaluations\n", nfev);
    }
#endif
    assert(n_par == NUM_PARAMS);
#ifdef DEBUG_FIT
    cerr<<" DisplayScale = "<<m_DisplayScale;
    cerr<<" DisplayOffset= "<<m_DisplayOffset;
    cerr<<" unDistortionParams = "<<DPoint(m_DistortParams[0], m_DistortParams[1]);
    cerr<<" Trapezoid = "<<m_TrapezoidFactor;
    cerr<<" angle = "<<m_Angle;
    cerr<<" => norm: "<< lm_enorm( m_dat, fvec )<<endl;
#endif
}

}
