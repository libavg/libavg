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
//  Original author of this file is igor@c-base.org 
//

#include "DeDistort.h"

#include "../base/XMLHelper.h"

#include <iostream>
#include <math.h>

const double sqrt3 = sqrt(3.);

using namespace std;

namespace avg{

// This transformation is based on the undistort code found at 
// http://www.math.rutgers.edu/~ojanen/undistort/index.html. 
//   a lot of parameters enter here, some of which can be calculated/set manually, 
//   some of which need to be determined via an optimization procedure
//
//   * m_FilmDisplacement moves the optical axis back to the center of the image:
//   * m_FilmScale scales the ROI to standard coords
//   * m_pDistortionParams OPT see paper
//   * m_P m_N OPT see paper
//   * m_Angle corrects rotation of camera OPT
//   * m_DisplayScale convert back from standard coords to display
//   * m_DisplayDisplacement correct the offset of the display from the center of the table
//     

void writePoint(xmlTextWriterPtr writer, string sName, DPoint& Val)
{
    int rc;
    rc = xmlTextWriterStartElement(writer, BAD_CAST sName.c_str());
    writeAttribute(writer, "x", Val.x);
    writeAttribute(writer, "y", Val.y);
    rc = xmlTextWriterEndElement(writer);

}

void writePoint3(xmlTextWriterPtr writer, string sName, DPoint3& Val)
{
    int rc;
    rc = xmlTextWriterStartElement(writer, BAD_CAST sName.c_str());
    writeAttribute(writer, "x", Val.x);
    writeAttribute(writer, "y", Val.y);
    writeAttribute(writer, "z", Val.z);
    rc = xmlTextWriterEndElement(writer);

}

DeDistort::DeDistort()
    : m_FilmDisplacement(0,0),
      m_FilmScale(1,1),
      m_P(0,0,0),
      m_N(0,0,1),
      m_Angle(0.0),
      m_TrapezoidFactor(0),
      m_DisplayDisplacement(0,0),
      m_DisplayScale(1,1)
{
    m_DistortionParams.push_back(0);
    m_DistortionParams.push_back(0);
    m_RescaleFactor = calc_rescale();
}

DeDistort::DeDistort(const DPoint& FilmDisplacement, const DPoint& FilmScale, 
            const std::vector<double>& DistortionParams, const DPoint3& P, 
            const DPoint3& N, double Angle, double TrapezoidFactor,
            const DPoint& DisplayDisplacement, const DPoint& DisplayScale)
    : m_FilmDisplacement(FilmDisplacement),
      m_FilmScale(FilmScale),
      m_DistortionParams(DistortionParams),
      m_P(P),
      m_N(N),
      m_Angle(Angle),
      m_TrapezoidFactor(TrapezoidFactor),
      m_DisplayDisplacement(DisplayDisplacement),
      m_DisplayScale(DisplayScale)
{
    m_RescaleFactor = calc_rescale();
}

DeDistort::~DeDistort()
{
}

void DeDistort::load(xmlNodePtr pParentNode)
{
    xmlNodePtr curXmlChild = pParentNode->xmlChildrenNode;
    while (curXmlChild) {
        const char * pNodeName = (const char *)curXmlChild->name;
        if (!strcmp(pNodeName, "cameradisplacement")) {
            m_FilmDisplacement.x = getRequiredDoubleAttr(curXmlChild, "x");
            m_FilmDisplacement.y = getRequiredDoubleAttr(curXmlChild, "y");
        } else if (!strcmp(pNodeName, "camerascale")) {
            m_FilmScale.x = getRequiredDoubleAttr(curXmlChild, "x");
            m_FilmScale.y = getRequiredDoubleAttr(curXmlChild, "y");
        } else if (!strcmp(pNodeName, "distortionparams")) {
            m_DistortionParams.push_back(getRequiredDoubleAttr(curXmlChild, "p2"));
            m_DistortionParams.push_back(getRequiredDoubleAttr(curXmlChild, "p3"));
        } else if (!strcmp(pNodeName, "p")) {
            m_P.x = getRequiredDoubleAttr(curXmlChild, "x");
            m_P.y = getRequiredDoubleAttr(curXmlChild, "y");
            m_P.z = getRequiredDoubleAttr(curXmlChild, "z");
        } else if (!strcmp(pNodeName, "n")) {
            m_N.x = getRequiredDoubleAttr(curXmlChild, "x");
            m_N.y = getRequiredDoubleAttr(curXmlChild, "y");
            m_N.z = getRequiredDoubleAttr(curXmlChild, "z");
        } else if (!strcmp(pNodeName, "trapezoid")) {
            m_TrapezoidFactor = getRequiredDoubleAttr(curXmlChild, "value");
        } else if (!strcmp(pNodeName, "angle")) {
            m_Angle = getRequiredDoubleAttr(curXmlChild, "value");
        } else if (!strcmp(pNodeName, "displaydisplacement")) {
            m_DisplayDisplacement.x = getRequiredDoubleAttr(curXmlChild, "x");
            m_DisplayDisplacement.y = getRequiredDoubleAttr(curXmlChild, "y");
        } else if (!strcmp(pNodeName, "displayscale")) {
            m_DisplayScale.x = getRequiredDoubleAttr(curXmlChild, "x");
            m_DisplayScale.y = getRequiredDoubleAttr(curXmlChild, "y");
        }
        curXmlChild = curXmlChild->next;
    }
    m_RescaleFactor = calc_rescale();
}

void DeDistort::save(xmlTextWriterPtr writer)
{
    int rc;
    rc = xmlTextWriterStartElement(writer, BAD_CAST "transform");
    writePoint(writer, "cameradisplacement", m_FilmDisplacement);
    writePoint(writer, "camerascale", m_FilmScale);
    rc = xmlTextWriterStartElement(writer, BAD_CAST "distortionparams");
    writeAttribute(writer, "p2", m_DistortionParams[0]);
    writeAttribute(writer, "p3", m_DistortionParams[1]);
    rc = xmlTextWriterEndElement(writer);
    writePoint3(writer, "p", m_P);
    writePoint3(writer, "n", m_N);
    writeSimpleXMLNode(writer, "trapezoid", m_TrapezoidFactor);
    writeSimpleXMLNode(writer, "angle", m_Angle);
    writePoint(writer, "displaydisplacement", m_DisplayDisplacement);
    writePoint(writer, "displayscale", m_DisplayScale);
    rc = xmlTextWriterEndElement(writer);
}

DPoint DeDistort::inverse_transform_point(const DPoint &pt)
{
//    return inverse_undistort(m_DistortionParams, pt);
//    return inverse_pinhole(m_P, m_N, pt);
    return translate(-m_FilmDisplacement,
            scale(DPoint(1./m_FilmScale.x, 1./m_FilmScale.y),
                inverse_undistort(m_DistortionParams,
                    scale(m_RescaleFactor,
//                        inverse_pinhole(m_P, m_N,
                        rotate(-m_Angle,
                            inv_trapezoid(m_TrapezoidFactor,
                                scale(DPoint(1./m_DisplayScale.x, 1./m_DisplayScale.y),
                                    translate(-m_DisplayDisplacement,
                                        pt
                                        )
                                    )
                                )
                            )
                        )
                    )
                )
            );
}

DPoint DeDistort::transform_point(const DPoint &pt)
{
    return translate(m_DisplayDisplacement, //translate 0,0 to center of display
            scale(m_DisplayScale,  //scale back to real display resolution
                trapezoid(m_TrapezoidFactor,
                    rotate(m_Angle, //rotate
                    //pinhole(m_P, m_N, //apply pinhole
                        scale(1./m_RescaleFactor,
                            undistort(m_DistortionParams, //undistort;
                                scale(m_FilmScale,  // scale to -1,-1,1,1
                                    translate(m_FilmDisplacement, // move optical axis to (0,0) 
                                        pt 
                                        )
                                    )
                                )
                            )
                        )
                    )
                )
            );
}

DPoint DeDistort::inv_trapezoid(const double trapezoid_factor, const DPoint &pt)
{
    //stretch x coord
    double yn = pt.y;
    return DPoint( 
            pt.x/(1+yn*trapezoid_factor),
            pt.y);
}

DPoint DeDistort::trapezoid(const double trapezoid_factor, const DPoint &pt)
{
    //stretch x coord
    double yn = pt.y;
    return DPoint( 
            //m_Center.x + ( pt.x - m_Center.x) * (1 + m_TrapezoidFactor * yn), 
            pt.x * (1 + yn*trapezoid_factor),
            pt.y);
}

//scale a point around the origin
DPoint DeDistort::scale(const double scale, const DPoint &pt){
    return DPoint(pt.x, pt.y)*scale;
}

DPoint DeDistort::scale(const DPoint &scales, const DPoint &pt){
    return DPoint(pt.x*scales.x, pt.y*scales.y);
}

//translate a point pt by the distance displacement
DPoint DeDistort::translate(const DPoint &displacement, const DPoint &pt){
    return pt + displacement;
}

//rotate a point counter-clockwise around the origin
DPoint DeDistort::rotate(double angle, const DPoint &pt){
    return DPoint( 
            cos(angle) * pt.x - sin(angle) * pt.y, 
            sin(angle) * pt.x + cos(angle) * pt.y
            );
}

double distort_map(const std::vector<double> &params, double r) {
    double S = 0;
    int counter = 3;
    std::vector<double>::const_iterator v;
    for(v=params.begin(); v!=params.end(); ++v){
        S += (*v) * pow(r, counter);
        ++counter;
    }
    return r+S;
}

double DeDistort::calc_rescale(){
    //make sure that the undistort transformation stays within the normalized box
    double scale = distort_map(m_DistortionParams, sqrt(2.0));
    return scale/sqrt(2.0);
}

//bool everythingZero(const std::vector<double> &params)
//{
//    for (int i=0; i<params.size(); ++i) {
//        if (fabs(params[i]) > 0.00001) {
//            return false;
//        }
//    }
//    return true;
//}
//

double inv_distort_map(const std::vector<double> &params, double r) {
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
DPoint DeDistort::inverse_undistort(const std::vector<double> &params, const DPoint &pt) {
    if ( params.empty() ) {
        return pt;
    }
    DPoint pt_norm = pt; //no need to scale anymore?
    double r_d = sqrt(pt_norm.x*pt_norm.x + pt_norm.y*pt_norm.y);
    double S;
    if (r_d < EPSILON){
        S=0;
    } else {
        S = inv_distort_map(params, r_d)/r_d;
    }
    DPoint result = pt_norm*(S);
    //cerr<<"inv distort: "<< result <<endl;
    return result;
}

DPoint DeDistort::undistort(const std::vector<double> &params, const DPoint &pt) {
    std::vector<double>::const_iterator v = params.begin();
    if ( v == params.end() ) {
        return pt;
    }
    DPoint pt_norm = pt; //no need to scale anymore?
    double r_d = sqrt(pt_norm.x*pt_norm.x + pt_norm.y*pt_norm.y);
    double S;
    if (r_d < EPSILON){
        S=0;
    } else {
        S = distort_map(params, r_d)/r_d;
    }
    
    DPoint result = pt_norm*(S);
    //cerr<<"distort: "<< result <<endl;
    return result;
}
//apply a pinhole transformation to the point pt.

DPoint DeDistort::inverse_pinhole(const DPoint3 &P, const DPoint3 &N, const DPoint &pt){
    double n1=N.x;
    double n2=N.y;
    double n3=N.z;
    double P1=P.x;
    double P2=P.y;
    double P3=P.z;
    double x=pt.x;
    double y=pt.y;
    double a = ((n1*n3*P2-n2*n3*P1-n1*n3)*P3+n1*n2*P2*P2
            +((n1*n1-n2*n2)*P1+n1*n2*(-y-1)
                +(n3*n3+n2*n2)*x)
            *P2-n1*n2*P1*P1
            +(-n1*n1*y+n3*n3*(1-y)+n1*n2*x+n2*n2)*P1
            +n1*n2*y+(-n3*n3-n2*n2)*x)
        /((n3*P2+n3*P1-n3)*P3+n2*P2*P2
                +((n2+n1)*P1+n2*(-y-1)-n1*x+n3*n3+n2*n2+n1*n1)*P2
                +n1*P1*P1+(-n2*y-n1*x+n3*n3+n2*n2+n1*n1-n1)*P1+n2*y
                +n1*x-n3*n3-n2*n2-n1*n1);
    double b = -((n1*n3*P2-n2*n3*P1+n2*n3)*P3+n1*n2*P2*P2
            +((n1*n1-n2*n2)*P1-n1*n2*y
                +(n3*n3+n2*n2)*x-n3*n3
                -n1*n1)
            *P2-n1*n2*P1*P1
            +(-n3*n3*y-n1*n1*y+n1*n2*x+n1*n2)*P1
            +n3*n3*y+n1*n1*y-n1*n2*x)
        /((n3*P2+n3*P1-n3)*P3+n2*P2*P2
                +((n2+n1)*P1+n2*(-y-1)-n1*x+n3*n3+n2*n2+n1*n1)*P2
                +n1*P1*P1+(-n2*y-n1*x+n3*n3+n2*n2+n1*n1-n1)*P1+n2*y
                +n1*x-n3*n3-n2*n2-n1*n1);
    return DPoint(a,b);
}
DPoint DeDistort::pinhole(const DPoint3& P, const DPoint3& N, const DPoint &pt)
{
    double n1=N.x;
    double n2=N.y;
    double n3=N.z;
    double P1=P.x;
    double P2=P.y;
    double P3=P.z;
    double a=pt.x;
    double b=pt.y;

    double x=1./(-a*n1 + -b*n2 + -n3*n3 + a*n1*P1 + b*n1*P1 + a*n2*P2 + b*n2*P2) * 
        (
         n3*n3*(-a + ((-1) + a + b)*P1) + 
         (n1+P1) * (
             n1*(-a + a*P1 + b*P1) + 
             n2*( -b + a*P2 + b*P2)
         ) +
         n3*(-a + n1 + a*P1 + b*P1)*P3
         );
    double y=1./(-a*n1 + -b*n2 + -1*n3*n3 + a*n1*P1 + b*n1*P1 + a*n2*P2 + b*n2*P2) * 
         (
          n3*n3*(-b + ((-1)+a+b)*P2) + 
          (n2+P2)* (
              n1*(-a+ a*P1 + b*P1) + 
              n2*(-b + a*P2 + b*P2)
          ) + 
          n3*(-b + n2 + a*P2 + b*P2)*P3
          );

    return DPoint(x,y);
}
    
}
