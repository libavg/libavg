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
    //void lm_print_tracker( int n_par, double* par, int m_dat, double* fvec,
    //                               void *data, int iflag, int iter, int nfev );
    std::ostream& operator<<( std::ostream& os, const DPoint3 &p)
    {
        os << "(" << p.x << "," << p.y << ","<<p.z<<")";
        return os;
    };

    typedef struct {
        std::vector<IntPoint> DisplayPoints;
        std::vector<DPoint> CamPoints;
        DPoint FilmDisplacement;
        DPoint FilmScale;
        CoordTransformerPtr CurrentTrafo;
        //double (*user_func)( std::vector<IntPoint> &DisplayPoints, std::vector<DPoint> &CamPoints);
    } CalibratorDataType;


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
    CalibratorDataType *mydata;
    mydata = static_cast<CalibratorDataType*>(data);

    if (iflag==2) {
        printf ("trying step in gradient direction\n");
    } else if (iflag==1) {
        printf ("determining gradient (iteration %d)\n", iter);
    } else if (iflag==0) {
        printf ("starting minimization\n");
    } else if (iflag==-1) {
        printf ("terminated after %d evaluations\n", nfev);
    }

//    DPoint DisplayDisplacement;
//    DPoint DisplayScale;
    vector<double> unDistortionParams;
    DPoint3 N;
    DPoint3 P;
    double Angle;
    double Trapezoid;
    assert(n_par == 8);
    DPoint DisplayDisplacement;
    DPoint DisplayScale;
    DisplayDisplacement.x = p[0]; 
    DisplayDisplacement.y = p[1];
    DisplayScale.x = p[2];
    DisplayScale.y = p[3];
    unDistortionParams.push_back(fabs(p[4]));
    unDistortionParams.push_back(fabs(p[5]));
    N.x = 0;
    N.y = 0;
    N.z = 1;
    P.x = 0;
    P.y = 0;
    P.z = 0;
    Angle = p[6];
    Trapezoid = p[7];
    cerr<<" DisplayDisplacement = "<<DisplayDisplacement;
    cerr<<" DisplayScale = "<<DisplayScale;
    cerr<<" FilmDisplacement = "<<mydata->FilmDisplacement;
    cerr<<" FilmScale = "<<mydata->FilmScale;
    cerr<<" unDistortionParams = "<<DPoint(unDistortionParams[0], unDistortionParams[1]);
    cerr<<" Trapezoid = "<<Trapezoid;
    cerr<<" angle = "<<Angle;
    cerr<<" => norm: "<< lm_enorm( m_dat, fvec )<<endl;
#if 0
    double f, y, t;
    int i;
    printf( "  par: " );
    for( i=0; i<n_par; ++i )
        printf( " %12g", par[i] );
    printf ( " => norm: %12g\n", lm_enorm( m_dat, fvec ) );
    if ( iflag == -1 ) {
        printf( "  fitting data as follows:\n" );
        for( i=0; i<m_dat; ++i ) {
            t = (mydata->user_t)[i];
            y = (mydata->user_y)[i];
            f = mydata->user_func( t, par );
            printf( "    t[%2d]=%12g y=%12g fit=%12g residue=%12g\n",
                    i, t, y, f, y-f );
        }
    }
#endif
}
void lm_evaluate_tracker( double* p, int m_dat, double* fvec,
                                  void *data, int *info ) {
    int i;
    CalibratorDataType *mydata;
    mydata = static_cast<CalibratorDataType*>(data);
    std::vector<double> distort_params;
    distort_params.push_back(0);
    distort_params.push_back(0.1);
    DPoint3  P = DPoint3(0,0,0); 
    DPoint3 N = DPoint3(0,0,1); 
    double Angle = 0;
    double TrapezoidFactor = 0;
    DPoint DisplayDisplacement;
    DPoint DisplayScale;
    DisplayDisplacement.x = p[0]; 
    DisplayDisplacement.y = p[1];
    DisplayScale.x = p[2];
    DisplayScale.y = p[3];
    distort_params[0] = fabs(p[4]);
    distort_params[1] = fabs(p[5]);
    Angle = p[6];
    TrapezoidFactor = p[7];
    mydata->CurrentTrafo = DeDistortPtr( 
            new DeDistort(mydata->FilmDisplacement,
                mydata->FilmScale,
                distort_params,
                P, N,
                Angle,
                TrapezoidFactor,
                DisplayDisplacement,
                DisplayScale
                )
            );

//            DisplayDisplacement.x, 
//            DisplayDisplacement.y, 
//            DisplayScale.x, 
//            DisplayScale.y,
//            DistortionParams[0],
//            DistortionParams[1],
//            Angle,
//            Trapezoid
//        };
        
//    mydata->CurrentTrafo = DeDistort();//FIXME
    for(int i=0;i<=15;i+=5) {
        cerr<<"sample value of trafo of "<<mydata->CamPoints[i]<<" : "<<mydata->CurrentTrafo->transform_point(mydata->CamPoints[i])<<" dist="<<calcDist(DPoint(mydata->DisplayPoints[i]), mydata->CurrentTrafo->transform_point(mydata->CamPoints[i]));
        cerr<<"==?"<<DPoint(mydata->DisplayPoints[i])<<endl;
    }
    for (i=0; i<m_dat; i++){
        fvec[i] = calcDist(mydata->CurrentTrafo->transform_point(mydata->CamPoints[i]), DPoint(mydata->DisplayPoints[i])); 
    }
    *info = *info; /* to prevent a 'unused variable' warning */
    /* if <parameters drifted away> { *info = -1; } */
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

    void TrackerCalibrator::makeTransformer(DeDistortPtr &new_trafo, DPoint &display_scale, DPoint &display_offset)
    {
        lm_control_type control;
        CalibratorDataType data;
        
        data.DisplayPoints = m_DisplayPoints;
        data.CamPoints = m_CamPoints;
        data.CurrentTrafo = DeDistortPtr();

        lm_initialize_control( &control );
        control.maxcall=1000;
        control.epsilon=1e-6;
        control.ftol = 1e-3;
        control.xtol = 1e-3;
        control.gtol = 1e-2;
        unsigned int dat = m_DisplayPoints.size();
        assert(dat == m_CamPoints.size());
       
        DPoint center = DPoint(m_ROI.Center());
        double w = m_ROI.Width();
        double h = m_ROI.Height();
        data.FilmDisplacement= -DPoint(m_CamExtents)/2+DPoint(m_ROI.tl); 
        data.FilmScale = DPoint(2./w,2./h);
        cerr<<"film displacement "<<data.FilmDisplacement;
        cerr<<"film scale "<<data.FilmScale<<endl;
        std::vector<double> unDistortionParams;
        unDistortionParams.push_back(0);
        unDistortionParams.push_back(0);
        DPoint3 P = DPoint3(0,0,0); 
        DPoint3 N = DPoint3(0,0,1); 
        double Angle = 0;
        double TrapezoidFactor = 0.2;
        DPoint DisplayDisplacement= DPoint(m_DisplayExtents)/2; 
        DPoint DisplayScale = DPoint(m_DisplayExtents)/2;

        int n_p = 8;
        double p[] = {
            DisplayDisplacement.x, 
            DisplayDisplacement.y, 
            DisplayScale.x, 
            DisplayScale.y,
            unDistortionParams[0],
            unDistortionParams[1],
            Angle,
            TrapezoidFactor
        };
        lm_minimize(dat, n_p, p, lm_evaluate_tracker, lm_print_tracker,
                     &data, &control );
        DisplayDisplacement.x = p[0];
        DisplayDisplacement.y = p[1];
        DisplayScale.x = p[2];
        DisplayScale.y = p[3];
        unDistortionParams[0] = p[4];
        unDistortionParams[1] = p[5];
        Angle = p[6];
        TrapezoidFactor = p[7];
        //feed the out variables
        //FIXME displacement and scaling splitting between the CoordTrafo (used in TrackerThread)
        //and display_offset, display_scale (used in EventStream::pollevent)
        new_trafo = DeDistortPtr( 
                new DeDistort(data.FilmDisplacement,
                    data.FilmScale,
                    unDistortionParams,
                    P, N,
                    Angle,
                    TrapezoidFactor,
                    -data.FilmDisplacement,
                    DPoint(1./data.FilmScale.x,1./data.FilmScale.y)
                    )
                );
        display_offset = DisplayDisplacement-data.FilmDisplacement;
        display_scale = DPoint(DisplayScale.x*data.FilmScale.x, DisplayScale.x*data.FilmScale.y);
    }
}
