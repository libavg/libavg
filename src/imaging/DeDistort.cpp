//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2008 Ulrich von Zadow
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
//  Original author of this file is igor@c-base.org 
//

#include "DeDistort.h"
#include "TrackerConfig.h"

#include "../base/StringHelper.h"

#include <cstring>
#include <iostream>
#include <math.h>

const double sqrt3 = sqrt(3.);

using namespace std;

namespace avg {

// This transformation is based on the undistort code found at 
// http://www.math.rutgers.edu/~ojanen/undistort/index.html. 
//   a lot of parameters enter here, some of which can be calculated/set manually, 
//   some of which need to be determined via an optimization procedure
//
//   * m_CamExtents is the size of the camera image. This is used to transform the 
//     coordinates of the image so they fall in the range (-1,-1)-(1,1)
//   * m_pDistortionParams OPT see paper
//   * m_Angle corrects rotation of camera OPT
//   * m_DisplayScale convert back from standard coords to display
//   * m_DisplayOffset correct the offset of the display from the center of the table
//     

DeDistort::DeDistort()
    : m_CamExtents(1,1),
      m_Angle(0.0),
      m_TrapezoidFactor(0),
      m_DisplayOffset(0,0),
      m_DisplayScale(1,1)
{
    m_DistortionParams.push_back(0);
    m_DistortionParams.push_back(0);
    m_RescaleFactor = calc_rescale();
}

DeDistort::DeDistort(const DPoint& camExtents, const DPoint& displayExtents)
    : m_Angle(0.0),
      m_TrapezoidFactor(0),
      m_DisplayOffset(0,0)
{
    m_CamExtents = camExtents; 
    m_DistortionParams.push_back(0);
    m_DistortionParams.push_back(0);
    m_DisplayScale.x = displayExtents.x/camExtents.x;
    m_DisplayScale.y = displayExtents.y/camExtents.y;
    m_RescaleFactor = calc_rescale();
}

DeDistort::DeDistort(const DPoint& camExtents, const vector<double>& distortionParams,
        double angle, double trapezoidFactor, const DPoint& displayOffset, 
        const DPoint& displayScale)
    : m_CamExtents(camExtents),
      m_DistortionParams(distortionParams),
      m_Angle(angle),
      m_TrapezoidFactor(trapezoidFactor),
      m_DisplayOffset(displayOffset),
      m_DisplayScale(displayScale)
{
    m_RescaleFactor = calc_rescale();
}

DeDistort::~DeDistort()
{
}

DRect DeDistort::getDisplayArea(const DPoint& displayExtents)
{
    return getActiveBlobArea(DRect(DPoint(0,0), displayExtents));
}

DRect DeDistort::getActiveBlobArea(const DRect& displayROI)
{
    DRect activeRect;
    activeRect.tl = transformScreenToBlob(DPoint(displayROI.tl));
    activeRect.br = transformScreenToBlob(DPoint(displayROI.br));
    if (activeRect.height() < 1) {
        double temp = activeRect.tl.y;
        activeRect.tl.y = activeRect.br.y;
        activeRect.br.y = temp;
    } 
    if (activeRect.width() < 1) {
        double temp = activeRect.tl.x;
        activeRect.tl.x = activeRect.br.x;
        activeRect.br.x = temp;
    } 
    return activeRect;
}

void DeDistort::load(const DPoint& camExtents, const TrackerConfig& config)
{
    m_CamExtents = camExtents;
    m_DistortionParams.clear();
    m_DistortionParams.push_back(config.getDoubleParam
            ("/transform/distortionparams/@p2"));
    m_DistortionParams.push_back(config.getDoubleParam
            ("/transform/distortionparams/@p3"));
    m_TrapezoidFactor = config.getDoubleParam("/transform/trapezoid/@value");
    m_Angle = config.getDoubleParam("/transform/angle/@value");
    m_DisplayOffset = config.getPointParam("/transform/displaydisplacement/");
    m_DisplayScale = config.getPointParam("/transform/displayscale/");

    m_RescaleFactor = calc_rescale();
}

void DeDistort::save(TrackerConfig& config)
{
    config.setParam("/transform/distortionparams/@p2", 
            toString(m_DistortionParams[0]));
    config.setParam("/transform/distortionparams/@p3", 
            toString(m_DistortionParams[1]));
    config.setParam("/transform/trapezoid/@value", 
            toString(m_TrapezoidFactor));
    config.setParam("/transform/angle/@value", 
            toString(m_Angle));
    config.setParam("/transform/displaydisplacement/@x", 
            toString(m_DisplayOffset.x));
    config.setParam("/transform/displaydisplacement/@y", 
            toString(m_DisplayOffset.y));
    config.setParam("/transform/displayscale/@x", 
            toString(m_DisplayScale.x));
    config.setParam("/transform/displayscale/@y", 
            toString(m_DisplayScale.y));
}

bool DeDistort::operator ==(const DeDistort& other) const
{
    return (m_CamExtents == other.m_CamExtents &&
            m_DistortionParams == other.m_DistortionParams &&
            m_Angle == other.m_Angle &&
            m_TrapezoidFactor == other.m_TrapezoidFactor &&
            m_DisplayOffset == other.m_DisplayOffset &&
            m_DisplayScale == other.m_DisplayScale &&
            m_RescaleFactor == other.m_RescaleFactor);
}
        
void DeDistort::dump() const
{
    cerr << "  Transform:" << endl;
    cerr << "    CamExtents: " << m_CamExtents << endl;
    cerr << "    DistortionParams: " << m_DistortionParams[0] << ", " 
            << m_DistortionParams[1] << m_DistortionParams[2] << endl;
    cerr << "    Trapezoid: " << m_TrapezoidFactor << endl;
    cerr << "    Angle: " << m_Angle << endl;
    cerr << "    DisplayOffset: " << m_DisplayOffset << endl;
    cerr << "    DisplayScale: " << m_DisplayScale << endl;
}

DPoint DeDistort::transformScreenToBlob(const DPoint& pt)
{
    // scale to blob image resolution and translate 0,0 to upper left corner.
    DPoint DestPt = pt-m_DisplayOffset;
    DestPt = DPoint(DestPt.x/m_DisplayScale.x, DestPt.y/m_DisplayScale.y);
    return DestPt;
}

DPoint DeDistort::inverse_transform_point(const DPoint& pt)
{
    DPoint destPt = pt-m_CamExtents/2;
    destPt = DPoint(2*destPt.x/m_CamExtents.x, 2*destPt.y/m_CamExtents.y);
    destPt = inv_trapezoid(m_TrapezoidFactor, destPt);
    destPt = destPt.getRotated(-m_Angle);
    destPt *= m_RescaleFactor;
    destPt = inverse_undistort(m_DistortionParams, destPt);
    destPt = DPoint(destPt.x*m_CamExtents.x/2, destPt.y*m_CamExtents.y/2);
    destPt += m_CamExtents/2;
    return destPt;
}

DPoint DeDistort::transformBlobToScreen(const DPoint& pt)
{
    DPoint destPt = DPoint(m_DisplayScale.x*pt.x, m_DisplayScale.y*pt.y);
    destPt += m_DisplayOffset;
    return destPt;
}

DPoint DeDistort::transform_point(const DPoint& pt)
{
    DPoint destPt = pt-m_CamExtents/2;
    destPt = DPoint(2*destPt.x/m_CamExtents.x, 2*destPt.y/m_CamExtents.y);
    destPt = undistort(m_DistortionParams, destPt);
    destPt /= m_RescaleFactor;
    destPt = destPt.getRotated(m_Angle);
    destPt = trapezoid(m_TrapezoidFactor, destPt);
    destPt = DPoint(destPt.x*m_CamExtents.x/2, destPt.y*m_CamExtents.y/2);
    destPt += m_CamExtents/2;
    return destPt;
}

DPoint DeDistort::inv_trapezoid(const double trapezoid_factor, const DPoint& pt)
{
    // stretch x coord
    double yn = pt.y;
    return DPoint(pt.x/(1+yn*trapezoid_factor), pt.y);
}

DPoint DeDistort::trapezoid(const double trapezoid_factor, const DPoint& pt)
{
    // stretch x coord
    double yn = pt.y;
    return DPoint(pt.x*(1+yn*trapezoid_factor), pt.y);
}

double distort_map(const vector<double>& params, double r) 
{
    double S = 0;
    int counter = 2;
    std::vector<double>::const_iterator v;
    for(v=params.begin(); v!=params.end(); ++v){
        S += (*v) * pow(r, counter);
        ++counter;
    }
    return r+S;
}

double DeDistort::calc_rescale()
{
    //make sure that the undistort transformation stays within the normalized box
    double scale = distort_map(m_DistortionParams, sqrt(2.0));
    return scale/sqrt(2.0);
}

double inv_distort_map(const vector<double>& params, double r) 
{
    double r1,r2,r3,f1,f2;
    r1 = r;
    r2 = r+.001;
    f1 = distort_map(params, r1)-r;
    f2 = distort_map(params, r2)-r;
    while (fabs(f2) > 0.0001) {
        r3 = (r1*f2-r2*f1)/(f2-f1);
        r1 = r2;
        r2 = r3;
        f1 = f2;
        f2 = distort_map(params, r2)-r;
    }
    return r2;
}

#define EPSILON 0.00001
DPoint DeDistort::inverse_undistort(const vector<double> &params, const DPoint &pt)
{
    if (params.empty()) {
        return pt;
    }
    DPoint pt_norm = pt;
    double r_d = sqrt(pt_norm.x*pt_norm.x + pt_norm.y*pt_norm.y);
    double S;
    if (r_d < EPSILON) {
        S=0;
    } else {
        S = inv_distort_map(params, r_d)/r_d;
    }
    DPoint result = pt_norm*(S);
    return result;
}

DPoint DeDistort::undistort(const vector<double>& params, const DPoint &pt) 
{
    std::vector<double>::const_iterator v = params.begin();
    if (v == params.end()) {
        return pt;
    }
    DPoint pt_norm = pt;
    double r_d = sqrt(pt_norm.x*pt_norm.x + pt_norm.y*pt_norm.y);
    double S;
    if (r_d < EPSILON) {
        S=0;
    } else {
        S = distort_map(params, r_d)/r_d;
    }
    
    DPoint result = pt_norm*(S);
    return result;
}
    
}
