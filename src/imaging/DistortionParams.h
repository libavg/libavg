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

#ifndef _DistortionParams_H_
#define _DistortionParams_H_

#include "../graphics/Point.h"

#include <libxml/parser.h>
#include <libxml/xmlwriter.h>

#include <vector>

namespace avg {

struct DPoint3 {
    DPoint3()
    {}

    DPoint3(double ix, double iy, double iz)
        : x(ix),
          y(iy),
          z(iz)
    {}

    double x;
    double y;
    double z;
};

//    DPoint FilmDisplacement = -DPoint(320,240); 
//    DPoint FilmScale = DPoint(w/2.,h/2.);
//    std::vector<double> DistortionParams;
//    DistortionParams.push_back(0.4);
//    DistortionParams.push_back(0.0);
//    DPoint3& P = DPoint(0,0,0); 
//    DPoint3& N = DPoint(0,0,1); 
//    double Angle = 0;
//    const DPoint& DisplayDisplacement=DPoint(-640,-360);
//    const DPoint& DisplayScale = DPoint(640,360);

struct DistortionParams {
    DistortionParams();
    DistortionParams(const DPoint& FilmDisplacement, const DPoint& FilmScale, 
            const std::vector<double>& DistortionParams, const DPoint3& P, const DPoint3& N, 
            double Angle, const DPoint& DisplayDisplacement, const DPoint& DisplayScale);
    void load(xmlNodePtr pParentNode);
    void save(xmlTextWriterPtr writer);

    DPoint m_FilmDisplacement;
    DPoint m_FilmScale;
    std::vector<double> m_DistortionParams;
    DPoint3 m_P;
    DPoint3 m_N;
    double m_Angle;
    DPoint m_DisplayDisplacement;
    DPoint m_DisplayScale;
};

}

#endif
