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

#include "CoordTransformer.h"

#include "../graphics/Rect.h"
#include "../graphics/Point.h"

#include <iostream>
#include <math.h>

#define sqrt3 sqrt(3.)

namespace avg{

CoordTransformer::CoordTransformer(IntRect srcRect, double K1, double T, double RescaleFactor)
    : m_TrapezoidFactor(T),
      m_K1(K1),
      m_RescaleFactor(RescaleFactor)
{
    m_Center = DPoint((srcRect.tl.x+srcRect.br.x-1)/2., (srcRect.tl.y+srcRect.br.y-1)/2.);
    //normalize to center-edge distance
    m_Scale = sqrt( pow(m_Center.x - srcRect.Width()+1,2) +  pow(m_Center.y - srcRect.Height()+1,2) );
    m_TrapezoidScale = srcRect.Height()/2;

}

CoordTransformer::~CoordTransformer()
{
}

DPoint CoordTransformer::inv_distortion(const DPoint &pt)
{
    if (fabs(m_K1)<1e-10){
        return DPoint(pt);
    }
    DPoint pt_norm = (pt - m_Center)/m_Scale;
    double r_d = sqrt(pt_norm.x*pt_norm.x + pt_norm.y*pt_norm.y)*m_RescaleFactor;
    double sub1;
    if (m_K1>0){
        sub1 =pow(sqrt((27*r_d*r_d*m_K1 + 4)/m_K1)/(6*sqrt(3)*m_K1) +
                r_d/(2*m_K1), 1./3.);  
    }else{
        sub1 =pow(sqrt((27*r_d*r_d*m_K1 - 4)/m_K1)/(6*sqrt(3)*m_K1) -
                r_d/(2*m_K1), 1./3.);  
    }
    double oldr = (sub1 - 1./(3*m_K1*sub1));
    double inv_S = oldr/r_d*m_RescaleFactor;
    return (pt_norm*inv_S)*m_Scale + m_Center;
}

DPoint CoordTransformer::distortion(const DPoint &pt)
{
    DPoint pt_norm = (pt - m_Center)/m_Scale;
    double r_d_squared = pt_norm.x*pt_norm.x + pt_norm.y*pt_norm.y;
    double S = (1 + m_K1*r_d_squared)/m_RescaleFactor;
    return pt_norm*S*m_Scale + m_Center;
}

DPoint CoordTransformer::inv_trapezoid(const DPoint &pt)
{
    //stretch x coord
    double yn = (pt.y - m_Center.y)/m_TrapezoidScale;
    return DPoint( 
            (pt.x + m_Center.x * yn*m_TrapezoidFactor)/(1+yn*m_TrapezoidFactor),
            pt.y);
}

DPoint CoordTransformer::trapezoid(const DPoint &pt)
{
    //stretch x coord
    double yn = (pt.y - m_Center.y)/m_TrapezoidScale;
    return DPoint( 
            //m_Center.x + ( pt.x - m_Center.x) * (1 + m_TrapezoidFactor * yn), 
            pt.x + yn*m_TrapezoidFactor * (pt.x - m_Center.x),
            pt.y);
}

DPoint CoordTransformer::inverse_transform_point(const DPoint &pt)
{
    return inv_trapezoid(inv_distortion(pt));
}

DPoint CoordTransformer::transform_point(const DPoint &pt)
{
    return distortion(trapezoid(pt));
}

double CoordTransformer::getPixelSize(const DPoint &pt)
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
