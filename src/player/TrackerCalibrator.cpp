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

#include "../imaging/DeDistort.h"

extern "C" {
#include "../lmfit/lmmin.h"
#include "../lmfit/lm_eval.h"
}

using namespace std;

#define NUM_POINTS 3 
#define MIN_DIST_FROM_BORDER 30

namespace avg {


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
void lm_evaluate_tracker( double* p, int m_dat, double* fvec,
                                  void *data, int *info ) {
    TrackerCalibrator *mydata = static_cast<TrackerCalibrator*>(data);
    mydata->evaluate_tracker(p, m_dat, fvec, info);

}

void TrackerCalibrator::print_tracker(int n_par, double *p, int m_dat, 
        double *fvec, int iflag, int iter, int nfev){
#ifdef DEBUG_FIT
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
    assert(n_par == 8);
#ifdef DEBUG_FIT
    cerr<<" DisplayDisplacement = "<<m_DisplayDisplacement;
    cerr<<" DisplayScale = "<<m_DisplayScale;
    cerr<<" FilmDisplacement = "<<m_FilmDisplacement;
    cerr<<" FilmScale = "<<m_FilmScale;
    cerr<<" unDistortionParams = "<<DPoint(m_DistortionParams[0], m_DistortionParams[1]);
    cerr<<" Trapezoid = "<<m_TrapezoidFactor;
    cerr<<" angle = "<<m_Angle;
    cerr<<" => norm: "<< lm_enorm( m_dat, fvec )<<endl;
#endif
}
    TrackerCalibrator::TrackerCalibrator(const IntPoint& CamExtents, 
            const IntRect& ROI, const IntPoint& DisplayExtents)
        : m_CurPoint(0),
          m_CamExtents(CamExtents),
          m_ROI(ROI),
          m_DisplayExtents(DisplayExtents),
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
    void TrackerCalibrator::initThisFromDouble(double *p){
        m_DisplayOffset.x = p[0]; 
        m_DisplayOffset.y = p[1];
        m_DisplayScale.x = p[2];
        m_DisplayScale.y = p[3];
        m_DistortParams.clear();
        m_DistortParams.push_back( p[4] );
        m_DistortParams.push_back( p[5]);
        m_Angle = p[6];
        m_TrapezoidFactor = p[7];
    }

    void TrackerCalibrator::evaluate_tracker(double *p, int m_dat, double* fvec, int* info){
        initThisFromDouble(p);
        m_CurrentTrafo = DeDistortPtr( 
                new DeDistort(m_FilmOffset,
                    m_FilmScale,
                    m_DistortParams,
                    m_Angle,
                    m_TrapezoidFactor,
                    m_DisplayOffset,
                    m_DisplayScale
                    )
                );

#ifdef DEBUG_FIT
        for(int i=0;i<=15;i+=5) {
            cerr<<"sample value of trafo of "
                <<mydata->CamPoints[i]<<" : "
                <<mydata->CurrentTrafo->transform_point(mydata->CamPoints[i])
                <<" dist="
                <<calcDist(DPoint(mydata->DisplayPoints[i]), mydata->CurrentTrafo->transform_point(mydata->CamPoints[i]))
            <<"=="<<
            <<DPoint(mydata->DisplayPoints[i])
            <<endl;
        }
#endif 
        for (int i=0; i<m_dat; i++){
            fvec[i] = calcDist(m_CurrentTrafo->transform_point(m_CamPoints[i]), DPoint(m_DisplayPoints[i])); 
        }
        *info = *info; /* to prevent a 'unused variable' warning */
        /* if <parameters drifted away> { *info = -1; } */
    }

    void TrackerCalibrator::makeTransformer(DeDistortPtr &new_trafo, 
            DPoint &display_scale, DPoint &display_offset)
    {
        lm_control_type control;
        
        m_CurrentTrafo = DeDistortPtr();

        lm_initialize_control( &control );
        control.maxcall=1000;
        control.epsilon=1e-6;
        control.ftol = 1e-4;
        control.xtol = 1e-4;
        control.gtol = 1e-4;
        unsigned int dat = m_DisplayPoints.size();
        assert(dat == m_CamPoints.size());
      
        //fill in reasonable defaults
        DPoint center = DPoint(m_ROI.Center());
        double w = m_ROI.Width();
        double h = m_ROI.Height();
        m_FilmOffset = -DPoint(m_CamExtents)/2+DPoint(m_ROI.tl); 
        m_FilmScale = DPoint(2./w,2./h);
        m_DistortParams.push_back(0);
        m_DistortParams.push_back(0);
        m_Angle = 0;
        m_TrapezoidFactor = 0.2;
        m_DisplayOffset= DPoint(m_DisplayExtents)/2; 
        m_DisplayScale = DPoint(m_DisplayExtents)/2;

        int n_p = 8;
        double p[] = {
            m_DisplayOffset.x, 
            m_DisplayOffset.y, 
            m_DisplayScale.x, 
            m_DisplayScale.y,
            m_DistortParams[0],
            m_DistortParams[1],
            m_Angle,
            m_TrapezoidFactor
        };
        initThisFromDouble(p);
        lm_minimize(dat, n_p, p, lm_evaluate_tracker, lm_print_tracker,
                     this, &control );
        //feed the out variables
        //FIXME displacement and scaling splitting between the CoordTrafo (used in TrackerThread)
        //and display_offset, display_scale (used in EventStream::pollevent)
        initThisFromDouble(p);
        new_trafo = DeDistortPtr( 
                new DeDistort(m_FilmOffset,
                    m_FilmScale,
                    m_DistortParams,
                    m_Angle,
                    m_TrapezoidFactor,
                    -m_FilmOffset,
                    DPoint(1./m_FilmScale.x,1./m_FilmScale.y)
                    )
                );
        display_scale = DPoint((m_DisplayScale.x*m_FilmScale.x), 
                (m_DisplayScale.y*m_FilmScale.y));
        display_offset = m_DisplayOffset+ DPoint( display_scale.x*m_FilmOffset.x, display_scale.y*m_FilmOffset.y);
        cerr << "display_offset= " << display_offset << 
                ", display_scale = " << display_scale << endl;
        cerr << "DisplayOffset = " << m_DisplayOffset << 
                ", data.FilmOffset = " << m_FilmOffset << endl;
    }
}
