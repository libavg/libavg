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

#include "BlobInfo.h"

#include <math.h>

namespace avg {

BlobInfo::BlobInfo(RunList *pRuns)
{
    m_Center = calcCenter(pRuns);
    m_Area = calcArea(pRuns);
    m_BoundingBox = calcBBox(pRuns);
    /*
       more useful numbers that can be calculated from c
       see e.g. 
       <http://www.cs.cf.ac.uk/Dave/Vision_lecture/node36.html#SECTION00173000000000000000>

       Orientation = tan−1(2(c_xy)/(c_xx − c_yy)) /2
       Inertia = c_xx + c_yy
       Eccentricity = ...
       */
    double c_xx = 0;
    double c_yy =0;
    double c_xy = 0;
    double ll=0;
    double l1;
    double l2;
    double tmp_x;
    double tmp_y;
    double mag;
    for(RunList::iterator r=pRuns->begin();r!=pRuns->end();++r) {
        //This is the evaluated expression for the variance when using runs...
        ll = r->length();
        c_yy += ll* (r->m_Row- m_Center.y)*(r->m_Row- m_Center.y);
        c_xx += ( (r->m_EndCol-1) * r->m_EndCol * (2*r->m_EndCol-1) 
                - (r->m_StartCol-1) * r->m_StartCol * (2*r->m_StartCol -1))/6. 
            - m_Center.x * ( (r->m_EndCol-1)*r->m_EndCol - (r->m_StartCol-1)*r->m_StartCol  )
            + ll* m_Center.x*m_Center.x;
        c_xy += (r->m_Row-m_Center.y)*0.5*( (r->m_EndCol-1)*r->m_EndCol
                - (r->m_StartCol-1)*r->m_StartCol) + ll *(m_Center.x*m_Center.y - m_Center.x*r->m_Row);
    }

    c_xx/=m_Area;
    c_yy/=m_Area;
    c_xy/=m_Area;

    m_Inertia = c_xx + c_yy;

    double T = sqrt( (c_xx - c_yy) * (c_xx - c_yy) + 4*c_xy*c_xy);
    m_Eccentricity = ((c_xx + c_yy) + T)/((c_xx+c_yy) - T);
    m_Orientation = 0.5*atan2(2*c_xy,c_xx-c_yy);
    //the l_i are variances (unit L^2) so to arrive at numbers that 
    //correspond to lengths in the picture we use sqrt
    if (fabs(c_xy) > 1e-30) {
        //FIXME. check l1!=0 l2!=0. li=0 happens for line-like components
        l1 = 0.5 * ( (c_xx+c_yy) + sqrt( (c_xx+c_yy)*(c_xx+c_yy) - 4 * (c_xx*c_yy-c_xy*c_xy) ) );
        l2 = 0.5 * ( (c_xx+c_yy) - sqrt( (c_xx+c_yy)*(c_xx+c_yy) - 4 * (c_xx*c_yy-c_xy*c_xy) ) );
        tmp_x = c_xy/l1 - c_xx*c_yy/(c_xy*l1)+ (c_xx/c_xy);
        tmp_y = 1.;
        mag = sqrt(tmp_x*tmp_x + tmp_y*tmp_y);
        m_EigenVector[0].x = tmp_x/mag;
        m_EigenVector[0].y = tmp_y/mag;
        m_EigenValues.x = l1;
        tmp_x = c_xy/l2 - c_xx*c_yy/(c_xy*l2)+ (c_xx/c_xy);
        tmp_y = 1.;
        mag = sqrt(tmp_x*tmp_x + tmp_y*tmp_y);
        m_EigenVector[1].x = tmp_x/mag;
        m_EigenVector[1].y = tmp_y/mag;
        m_EigenValues.y = l2;
    } else {
        //matrix already diagonal
        if (c_xx > c_yy) {
            m_EigenVector[0].x = 1;
            m_EigenVector[0].y = 0;
            m_EigenVector[1].x = 0;
            m_EigenVector[1].y = 1;
            m_EigenValues.x = c_xx;
            m_EigenValues.y = c_yy;
        } else {
            m_EigenVector[0].x = 0;
            m_EigenVector[0].y = 1;
            m_EigenVector[1].x = 1;
            m_EigenVector[1].y = 0;
            m_EigenValues.x = c_yy;
            m_EigenValues.y = c_xx;
        }
    }
    m_ScaledBasis[0].x = m_EigenVector[0].x*sqrt(m_EigenValues.x);
    m_ScaledBasis[0].y = m_EigenVector[0].y*sqrt(m_EigenValues.x);
    m_ScaledBasis[1].x = m_EigenVector[1].x*sqrt(m_EigenValues.y);
    m_ScaledBasis[1].y = m_EigenVector[1].y*sqrt(m_EigenValues.y);
}

const DPoint & BlobInfo::getCenter() const
{
    return m_Center;
}

double BlobInfo::getArea() const
{
    return m_Area;
}

const IntRect & BlobInfo::getBoundingBox() const
{
    return m_BoundingBox;
}

double BlobInfo::getEccentricity() const
{
    return m_Eccentricity;
}

double BlobInfo::getInertia() const
{
    return m_Inertia;
}

double BlobInfo::getOrientation() const
{
    return m_Orientation;
}

const DPoint & BlobInfo::getScaledBasis(int i) const
{
    return m_ScaledBasis[i];
}

const DPoint & BlobInfo::getEigenVector(int i) const
{
    return m_EigenVector[i];
}

const DPoint & BlobInfo::getEigenValues() const
{
    return m_EigenValues;
}

DPoint BlobInfo::calcCenter(RunList *pRuns)
{
    DPoint Center;
    double c = 0;
    for(RunList::iterator r=pRuns->begin();r!=pRuns->end();++r){
        Center += r->center()*r->length();
        c += r->length();
    }
    Center = Center/c;
    return Center;
}

IntRect BlobInfo::calcBBox(RunList *pRuns)
{
    int x1=__INT_MAX__;
    int y1=__INT_MAX__;
    int x2=0;
    int y2=0;
    for(RunList::iterator r=pRuns->begin();r!=pRuns->end();++r){
        x1 = std::min(x1, r->m_StartCol);
        y1 = std::min(y1, r->m_Row);
        x2 = std::max(x2, r->m_EndCol);
        y2 = std::max(y2, r->m_Row);
    }
    return IntRect(x1,y1,x2,y2);
}

int BlobInfo::calcArea(RunList *pRuns)
{
    int res = 0;
    for(RunList::iterator r=pRuns->begin();r!=pRuns->end();++r){
        res+= r->length();
    }
    return res;
}

}
