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

#include "DistortionParams.h"

//#include <libxml/xmlwriter.h>
//#include <libxml/xmlstring.h>

namespace avg {

    DistortionParams::DistortionParams() 
      : m_FilmDisplacement(0,0),
        m_FilmScale(1,1),
        m_P(0,0,0),
        m_N(0,0,1),
        m_Angle(0.0),
        m_DisplayDisplacement(0,0),
        m_DisplayScale(1,1)
    {
        m_DistortionParams.push_back(0);
        m_DistortionParams.push_back(0);
    }

    DistortionParams::DistortionParams(const DPoint& FilmDisplacement, const DPoint& FilmScale, 
            const std::vector<double>& DistortionParams, const DPoint3& P, const DPoint3& N, 
            double Angle, const DPoint& DisplayDisplacement, const DPoint& DisplayScale)
      : m_FilmDisplacement(FilmDisplacement),
        m_FilmScale(FilmScale),
        m_DistortionParams(DistortionParams),
        m_P(P),
        m_N(N),
        m_Angle(Angle),
        m_DisplayDisplacement(DisplayDisplacement),
        m_DisplayScale(DisplayScale)
    {
    }

    void DistortionParams::load(xmlNodePtr pParentNode)
    {
    }

    void DistortionParams::save()
    {
    }

}
