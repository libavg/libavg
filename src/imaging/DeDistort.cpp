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
#include <vector>
#include "../graphics/Rect.h"
#include "../graphics/Point.h"

#include <iostream>
#include <math.h>

#define sqrt3 sqrt(3.)

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
DPoint inv_distort_map(std::vector<double> &params, double r);
double distort_map(std::vector<double> &params, double r);

DeDistort::DeDistort(DPoint &FilmDisplacement, DPoint &FilmScale, 
   std::vector<double> DistortionParams, 
   double P[3], double N[3], double Angle, 
   DPoint DisplayScale, DPoint DisplayDisplacement ):
    m_FilmDisplacement(FilmDisplacement),
    m_DisplayScale(FilmScale),
    m_Angle(Angle),
    m_P(P),
    m_N(N),
    m_DistortionParams(DistortionParams),
    m_FilmDisplacement(FilmDisplacement)
    m_FilmScale(FilmScale),
{


}

DeDistort::~DeDistort()
{
}

DPoint DeDistort::inverse_transform_point(const DPoint &pt)
{
//FIXME
}


DPoint DeDistort::transform_point(const DPoint &pt)
{
    return translate(m_DisplayDisplacement, //translate 0,0 to center of display
        scale(m_DisplayScale,  //scale back to real display resolution
            rotate(m_Angle, //rotate
                pinhole(m_P[0], m_P[1], m_P[2], m_N[0], m_N[1], m_N[2], //apply pinhole
                    undistort(m_DistortionParams, //undistort;
                        scale(m_FilmScale,  //scale to -1,-1,1,1
                            translate(m_FilmDisplacement, //move optical axis to center of image
                                pt 
                            )
                        )
                    )
                )
            )
        )
    );
}

//scale a point around the origin
DPoint DeDistort::scale(DPoint &scales, DPoint &pt){
    return DPoint(pt.x*scales.x, pt.y*scales.y);
}

//translate a point pt by the distance displacement
DPoint DeDistort::translate(DPoint &displacement, DPoint &pt){
    return pt + displacement;
}
//rotate a point counter-clockwise around the origin
DPoint DeDistort::rotate(double angle, Dpoint &pt){
    return DPoint( 
            sin(angle) * pt.x + cos(angle) * pt.y, 
            cos(angle) * pt.x + sin(angle) * pt.y
            );
}

//FIXME
DPoint inv_distort_map(std::vector<double> &params, double r){
  double r1,r2,r3,f1,f2;
  r1 = r;
  r2 = r+.001;
  f1 = distort_map(params, pt)-r;
  f2 = distort_map(params, pt)-r;
  while (fabs(f2) > 0.0001) {
    r3 = (r1*f2-r2*f1)/(f2-f1);
    r1 = r2;
    r2 = r3;
    f1 = f2;
    f2 = distort_map(r2, c, N)-r;
  }
  return r2;


}

double distort_map(std::vector<double> &params, double r) {
    double S = 1;
    for(int counter=2;v!=params.end();++v, ++counter){
        S += (*v) * power(r, counter)
    }
    return S;
}

DPoint DeDistort::undistort(std::vector<double> &params, DPoint &pt) {

    std::vector<double>::iterator v = params.begin();
    if ( v == params.end() ) {
        return pt;
    }
    double r_d = sqrt(pt_norm.x*pt_norm.x + pt_norm.y*pt_norm.y);
    double S = inv_distort_map(params, r_d);
    DPoint pt_norm = pt; //no need to scale anymore?
    
    return S*pt_norm;
}
DPoint DeDistort::undistort(std::vector<double> &params, DPoint &pt) {

    std::vector<double>::iterator v = params.begin();
    if ( v == params.end() ) {
        return pt;
    }
    double r_d = sqrt(pt_norm.x*pt_norm.x + pt_norm.y*pt_norm.y);
    double S = distort_map(params, r_d);
    DPoint pt_norm = pt; //no need to scale anymore?
    
    return S*pt_norm;
}
//apply a pinhole transformation to the point pt.

DPoint DeDistort::pinhole(double normal_vec_1, double normal_vec_2, double normal_vec_3, 
        double pinhole_position_1, double pinhole_position_2, double pinhole_position_3,
        DPoint &pt) {

    double n1=normal_vec_1;
    double n2=normal_vec_2;
    double n3=normal_vec_3;
    double P1=pinhole_position_1;
    double P2=pinhole_position_2;
    double P3=pinhole_position_3;
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
    double y=1./(-a*n1 + -b*n2 + -1*n3*n3 + a*n1*P1 + b*n1*P1 + a*n2*P2 + b*n2.*P2) * 
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

double DeDistort::getPixelSize(const DPoint &pt)
{
    //volume dxdy transforms as |D|dx'dy' where |D| is the functional determinant
    //det { { dx'/dx, dx'/dy}, {dy'/dx,dy'/dy}}
    //|D| = dx'/dx * dy'/dy - dx'/dy * dy'/dx
    //
    //with x'=x'(x,y) are the new coordinates in terms of the old ones
    //in our case.
    //trapezoid:
    //x' = m0 + (x-m0)*(1+m_TrapezoidFactor*yn)
    //y' = y
    //|D| = (1+m_TrapezoidFactor*yn)|(x,y)
    //
    //distortion:
    //x' = S*(x-x0) + x0
    //y' = S*(y-y0) + y0
    //oBdA x0=y0=0 (volume invariant under translation)
    //S = (1+m_K1*|(x,y)|^2/m_Scale)
    //dx'/dx = S+x*dS/dx
    //dx'/dy = x*dS/dy
    //dy'/dx = y*dS/dx
    //dy'/dy = S+y*dS/dx
    //maxima:
    //S(x,y):= (1+K1 *(x**2+y**2));
    //dS/dx = 2xK1
    //dS/dy = 2yK1
    //xd(x,y):=S(x,y)*x;
    //yd(x,y):=S(x,y)*y;
    //
    //diff(xd(x,y),x)*diff(yd(x,y),y)-diff(xd(x,y),y)*diff(yd(x,y),x);
    //|D| = S^2 + 2*x*y*dS/dx*dS/dy = S^2 + 16 x^2*y^2 K1^2
    //|D| = (...)|(xt,yt) where xt,yt are the coords after trapezoid trafo 
    double yn = (pt.y - m_Center.y)/m_Scale;
    DPoint pt2 = trapezoid(pt);
    return (1+m_TrapezoidFactor*yn); // FIXME
}
    
}
