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

#ifndef _TrackerCalibrator_H_
#define _TrackerCalibrator_H_

#include "../api.h"
#include "../imaging/DeDistort.h"
#include "../base/GLMHelper.h"

#include <vector>

namespace avg {

class AVG_API TrackerCalibrator {

public:
    friend void lm_evaluate_tracker(double* p, int m_dat, double* fvec,
            void *data, int *info);
    friend void lm_print_tracker(int n_par, double* p, int m_dat, double* fvec, 
            void *data, int iflag, int iter, int nfev);

    TrackerCalibrator(const IntPoint& CamExtents, const IntPoint& DisplayExtents);
    virtual ~TrackerCalibrator();

    bool nextPoint();
    IntPoint getDisplayPoint();
    void setCamPoint(const glm::vec2& pt);

    DeDistortPtr makeTransformer();

private:
    void initThisFromDouble(double *p);
    
    void evaluate_tracker(double *p, int m_dat, double *fvec, int* info);
    void print_tracker(int n_par, double *p, int m_dat, 
            double *fvec, int iflag, int iter, int nfev);

    std::vector<double> m_DistortParams;
    double m_Angle;
    glm::dvec2 m_DisplayScale;
    glm::dvec2 m_DisplayOffset;
    double m_TrapezoidFactor;
    DeDistortPtr m_CurrentTrafo;
    unsigned int m_CurPoint;
    std::vector<IntPoint> m_DisplayPoints;
    std::vector<glm::dvec2> m_CamPoints;

    IntPoint m_CamExtents;
    IntPoint m_DisplayExtents;

    bool m_bCurPointSet;
};

}

#endif
