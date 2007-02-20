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

#ifndef _TrackerCalibrator_H_
#define _TrackerCalibrator_H_

#include "../graphics/Point.h"

#include "../imaging/DeDistort.h"

#include <vector>

namespace avg {

typedef class ITransformerTarget;

class TrackerCalibrator {

public:
    TrackerCalibrator(const IntPoint& CamExtents, 
            const IntRect& ROI, const IntPoint& DisplayExtents);
    virtual ~TrackerCalibrator();

    bool nextPoint();
    int getDisplayPointX();
    int getDisplayPointY();
    void setCamPoint(double x, double y);

    void makeTransformer(DeDistortPtr &new_trafo, DPoint &display_scale, DPoint &display_offset);

    //actually these two should not really be public. They are not part of the public interface 
    //of TrackerCalibrator.
    //private:
    //FIXME make lm_print_tracker and lm_evaluate_tracker friends of
    //class TrackerCalibrator
    void evaluate_tracker(double *p, int m_dat, double *fvec, int* info);
    void print_tracker(int n_par, double *p, int m_dat, 
            double *fvec, int iflag, int iter, int nfev);

private:
    void initThisFromDouble(double *p);
    std::vector<double> m_DistortParams;
    double m_Angle;
    DPoint m_FilmScale;
    DPoint m_FilmOffset;
    DPoint m_DisplayScale;
    DPoint m_DisplayOffset;
    double m_TrapezoidFactor;
    DeDistortPtr m_CurrentTrafo;
    unsigned int m_CurPoint;
    std::vector<IntPoint> m_DisplayPoints;
    std::vector<DPoint> m_CamPoints;

    IntPoint m_CamExtents;
    IntRect m_ROI;
    IntPoint m_DisplayExtents;

    bool m_bCurPointSet;
};

}

#endif
