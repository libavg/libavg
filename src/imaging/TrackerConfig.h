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
//  Original author of this file is igor@c-base.org.
//

#ifndef _TrackerConfig_H_
#define _TrackerConfig_H_

#include "../graphics/Rect.h"
#include "CoordTransformer.h"

#include <string>

namespace avg {

struct TrackerConfig
{
    TrackerConfig();
    virtual ~TrackerConfig();
    
    void load(std::string sFilename);
    void save(std::string sFilename);

    // Coordinate transforms.
    IntRect m_ROI;//after applying de-distortion, take this as the table surface
    IntRect m_DestRect; // Transform all blobs to be in this rect at the end    

    CoordTransformerPtr m_pTrafo; 
    // Camera params
    int m_Brightness;
    int m_Exposure;
    int m_Gamma;
    int m_Gain;
    int m_Shutter;
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

    // Imaging params
    int m_Threshold; //min pixel val for detection
    int m_HistoryUpdateInterval;
    double m_Similarity; //max distance for tracking blobs
    double m_AreaBounds[2]; //min, max for area in percents of screen size
    double m_EccentricityBounds[2]; //min, max for Eccentricity

    bool m_bCreateDebugImages;
    bool m_bCreateFingerImage;
};

}
#endif
