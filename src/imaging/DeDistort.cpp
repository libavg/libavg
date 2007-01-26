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

DeDistort::DeDistort(IntRect srcRect, double K1, double T, double RescaleFactor)
    : m_TrapezoidFactor(T),
      m_K1(K1),
      m_RescaleFactor(RescaleFactor)
{
    m_Center = DPoint((srcRect.tl.x+srcRect.br.x-1)/2., (srcRect.tl.y+srcRect.br.y-1)/2.);
    //normalize to center-edge distance
    m_Scale = sqrt( pow(m_Center.x - srcRect.Width()+1,2) +  pow(m_Center.y - srcRect.Height()+1,2) );
    m_TrapezoidScale = srcRect.Height()/2;

}

DeDistort::~DeDistort()
{
}

DPoint DeDistort::inverse_transform_point(const DPoint &pt)
{
    return inv_trapezoid(inv_distortion(pt));
}
// This transformation is based on the undistort code found at 
// http://www.math.rutgers.edu/~ojanen/undistort/index.html. 
//   a lot of parameters enter here, some of which can be calculated/set manually, 
//   some of which need to be determined via an optimization procedure
//
//   * m_FilmDisplacement moves the optical axis back to the center of the image:
//     calculate from ROI
//   * m_?FilmScale scales the ROI to standard coords
//     calculate from size of ROI 
//   ** m_pDistortionParams OPT see paper
//   ** m_P m_N OPT see paper
//   ** m_Angle corrects rotation of camera OPT
//   * m_?DisplayScale convert back from standard coords to display
//     calculate from useable area of display
//   * m_DisplayDisplacement correct the offset of the display from the center of the table
//     calculate from useable area of display
//     


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
DPoint DeDistort::undistort(std::vector<double> &params, DPoint &pt) {

    std::vector<double>::iterator v = params.begin();
    if ( v == params.end() ) {
        return pt;
    }
    double S = *(v++);
    DPoint pt_norm = pt; //no need to scale anymore?
    double r_d = sqrt(pt_norm.x*pt_norm.x + pt_norm.y*pt_norm.y);
    for(;v!=params.end();++v){
        S = S*r_d + *v;
    }
    S*=r_d*r_d;
    return pt_norm*S;
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
