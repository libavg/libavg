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

#include "Blob.h"

#include "../graphics/Filterfill.h"
#include "../graphics/Pixel8.h"

#include <stdlib.h>
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <iostream>
#include <algorithm>

using namespace std;

namespace avg {

Blob::Blob(const Run & run)
{
    ObjectCounter::get()->incRef(&typeid(*this));
    m_Runs.reserve(50);
    m_Runs.push_back(run);
    m_pParent = BlobPtr();

    m_bStatsAvailable = false;
}

Blob::~Blob() 
{
    ObjectCounter::get()->decRef(&typeid(*this));
}

RunArray *Blob::getRuns()
{
    return &m_Runs;
}

void Blob::addRun(const Run & run)
{
    m_Runs.push_back(run);
}

void Blob::merge(const BlobPtr& other)
{
    assert(other);
    RunArray * other_runs=other->getRuns();
    m_Runs.insert(m_Runs.end(), other_runs->begin(), other_runs->end());
    other_runs->clear();
}

void Blob::render(BitmapPtr pSrcBmp, BitmapPtr pDestBmp, Pixel32 Color, 
        int Min, int Max, bool bFinger, bool bMarkCenter, Pixel32 CenterColor)
{
    assert (pSrcBmp->getBytesPerPixel() == 1);
    assert (pDestBmp->getBytesPerPixel() == 4);
    unsigned char *pSrc;
    unsigned char *pDest;
    unsigned char *pColor = (unsigned char *)(&Color);
    int IntensityScale = 2*256/(max(Max-Min, 1));
    for(RunArray::iterator it=m_Runs.begin();it!=m_Runs.end();++it) {
        assert (it->m_Row < pSrcBmp->getSize().y);
        assert (it->m_StartCol >= 0);
        assert (it->m_EndCol <= pSrcBmp->getSize().x);
        pSrc = pSrcBmp->getPixels()+it->m_Row*pSrcBmp->getStride();
        pDest = pDestBmp->getPixels()+it->m_Row*pDestBmp->getStride();
        int x_pos = it->m_StartCol;
        pSrc += x_pos;
        pDest+= x_pos*4;
        while(x_pos<it->m_EndCol) {
            int Factor = (*pSrc-Min)*IntensityScale;
            if (Factor < 0) {
                Factor = 0;
            }
            if (Factor > 255) {
                Factor = 255;
            }
            *(pDest++) = ((*pColor)*Factor) >> 8;
            *(pDest++) = ((*(pColor+1))*Factor) >> 8;
            *(pDest++) = ((*(pColor+2))*Factor) >> 8;
            *(pDest++) = ((*(pColor+3))*Factor) >> 8;
            pSrc++;
            x_pos++;
        }
    }

    assert(m_bStatsAvailable);
    if(bMarkCenter) {
        IntPoint Center = IntPoint(int(m_Center.x+0.5), int(m_Center.y+0.5));
        
        IntPoint End0 = IntPoint(m_ScaledBasis[0])+Center;
        pDestBmp->drawLine(Center, End0, CenterColor);
        IntPoint End1 = IntPoint(m_ScaledBasis[1])+Center;
        pDestBmp->drawLine(Center, End1, CenterColor);

        
        if (bFinger && m_RelatedBlobs.size() > 0) {
            // Draw finger direction
            BlobPtr pHandBlob = (m_RelatedBlobs)[0].lock();
            if (pHandBlob) {
                pDestBmp->drawLine(Center, IntPoint(pHandBlob->getCenter()),
                        Pixel32(0xD7, 0xC9, 0x56, 0xFF));
            }
        }
        if (!m_Contour.empty()) {
            for (vector<IntPoint>::iterator it=m_Contour.begin()+1; it!=m_Contour.end(); ++it) {
                IntPoint Pt1 = *(it-1);
                IntPoint Pt2 = *it;
                pDestBmp->drawLine(Pt1, Pt2, CenterColor);
            }
            pDestBmp->drawLine(*(m_Contour.end()-1), *m_Contour.begin(), CenterColor);
        }
    }
}
        
bool Blob::contains(IntPoint pt)
{
    for(RunArray::iterator it=m_Runs.begin(); it!=m_Runs.end(); ++it) {
        if (it->m_Row == pt.y && it->m_StartCol <= pt.x && it->m_EndCol > pt.x) {
            return true;
        } 
    }
    return false;
}

void Blob::calcStats()
{
    m_Center = calcCenter();
    m_Area = calcArea();
    m_BoundingBox = calcBBox();
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
    for(RunArray::iterator r=m_Runs.begin();r!=m_Runs.end();++r) {
        //This is the evaluated expression for the variance when using runs...
        ll = r->length();
        c_yy += ll* (r->m_Row- m_Center.y)*(r->m_Row- m_Center.y);
        c_xx += ( (r->m_EndCol-1) * r->m_EndCol * (2*r->m_EndCol-1) 
                - (r->m_StartCol-1) * r->m_StartCol * (2*r->m_StartCol -1))/6. 
            - m_Center.x * ( (r->m_EndCol-1)*r->m_EndCol - (r->m_StartCol-1)*r->m_StartCol  )
            + ll* m_Center.x*m_Center.x;
        c_xy += (r->m_Row-m_Center.y)*0.5*( (r->m_EndCol-1)*r->m_EndCol
                - (r->m_StartCol-1)*r->m_StartCol) 
                + ll *(m_Center.x*m_Center.y - m_Center.x*r->m_Row);
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

    m_bStatsAvailable = true;
}

const DPoint & Blob::getCenter() const
{
    return m_Center;
}

double Blob::getArea() const
{
    return m_Area;
}

const IntRect & Blob::getBoundingBox() const
{
    return m_BoundingBox;
}

double Blob::getEccentricity() const
{
    return m_Eccentricity;
}

double Blob::getInertia() const
{
    return m_Inertia;
}

double Blob::getOrientation() const
{
    return m_Orientation;
}

const DPoint & Blob::getScaledBasis(int i) const
{
    return m_ScaledBasis[i];
}

const DPoint & Blob::getEigenVector(int i) const
{
    return m_EigenVector[i];
}

const DPoint & Blob::getEigenValues() const
{
    return m_EigenValues;
}

void Blob::clearRelated()
{
    m_RelatedBlobs.clear();
}

void Blob::addRelated(BlobPtr pBlob)
{
    m_RelatedBlobs.push_back(pBlob);
}

const BlobPtr Blob::getFirstRelated()
{
    if (m_RelatedBlobs.empty()) {
        return BlobPtr();
    } else {
        return m_RelatedBlobs[0].lock();
    }
}

DPoint Blob::calcCenter()
{
    DPoint Center(0,0);
    double c = 0;
    for(RunArray::iterator r=m_Runs.begin();r!=m_Runs.end();++r) {
        Center += r->m_Center*r->length();
        c += r->length();
    }
    Center = Center/c;
    return Center;
}

IntRect Blob::calcBBox()
{
    int x1=INT_MAX;
    int y1=INT_MAX;
    int x2=0;
    int y2=0;
    for(RunArray::iterator r=m_Runs.begin();r!=m_Runs.end();++r) {
        x1 = std::min(x1, r->m_StartCol);
        y1 = std::min(y1, r->m_Row);
        x2 = std::max(x2, r->m_EndCol);
        y2 = std::max(y2, r->m_Row);
    }
    return IntRect(x1,y1,x2,y2);
}

int Blob::calcArea()
{
    int res = 0;
    for(RunArray::iterator r=m_Runs.begin();r!=m_Runs.end();++r) {
        res+= r->length();
    }
    return res;
}

void Blob::calcContour(int NumPoints)
{
    int w = m_BoundingBox.Width();
    int h = m_BoundingBox.Height();
    double StartDist = sqrt(double(w*w+h*h));
    for (double i=0; i<NumPoints; i++) {
        IntPoint OuterPt(int(StartDist*sin(i*2*M_PI/NumPoints)+m_Center.x), 
                int(StartDist*cos(i*2*M_PI/NumPoints)+m_Center.y));
        IntPoint InnerPt(m_Center);
        while (calcDist(OuterPt, InnerPt) > 2) {
            IntPoint MiddlePt = (OuterPt+InnerPt)/2;
            if (ptInBlob(MiddlePt)) {
                InnerPt = MiddlePt;
            } else {
                OuterPt = MiddlePt;
            }
        }
        m_Contour.push_back(InnerPt);
    }
}

bool Blob::ptInBlob(const IntPoint& Pt)
{
    if (m_BoundingBox.Contains(Pt)) {
        for (RunArray::iterator it = m_Runs.begin(); it!=m_Runs.end(); ++it) {
            if (Pt.y == it->m_Row && Pt.x >= it->m_StartCol && Pt.x < it->m_EndCol) {
                return true;
            }
        }
    }
    return false;
}

bool connected(const Run & run1, const Run & run2)
{
    if (run1.m_StartCol > run2.m_StartCol) {
        return run2.m_EndCol > run1.m_StartCol;
    } else {
        return run1.m_EndCol > run2.m_StartCol;
    }
}

void store_runs(BlobVectorPtr pComps, RunArray *runs1, RunArray *runs2)
{
    for (RunArray::iterator run1_it = runs1->begin(); run1_it!=runs1->end(); ++run1_it) {
        for (RunArray::iterator run2_it = runs2->begin(); run2_it!=runs2->end(); ++run2_it) {
            if (run2_it->m_StartCol > run1_it->m_EndCol) {
                break;
            }
            if (connected(*run1_it, *run2_it)) {
                BlobPtr p_blob = run1_it->m_pBlob.lock();
                while (p_blob->m_pParent) {
                    p_blob = p_blob->m_pParent;
                }
                if (!(run2_it->m_pBlob.expired())) {
                    BlobPtr c_blob = run2_it->m_pBlob.lock();
                    while (c_blob->m_pParent) {
                        c_blob = c_blob->m_pParent;
                    }
                    if (c_blob!=p_blob) {
                        p_blob->merge(c_blob); //destroys c_blobs runs_list
                        c_blob->m_pParent = p_blob;
                    }
                } else {
                    run2_it->m_pBlob = p_blob;
                    p_blob->addRun(*run2_it);
                }
            }
        }
    }
    for (RunArray::iterator run2_it = runs2->begin(); run2_it!=runs2->end(); ++run2_it) {
        if (run2_it->m_pBlob.expired()) {
            BlobPtr pBlob = BlobPtr(new Blob(*run2_it));
            pComps->push_back(pBlob);
            run2_it->m_pBlob = pBlob;
        }
    }
}

void findRunsInLine(BitmapPtr pBmp, int y, RunArray * pRuns, 
        unsigned char threshold)
{
    int run_start=0;
    int run_stop=0;
    const unsigned char * pPixel = pBmp->getPixels()+y*pBmp->getStride();
    bool cur=*pPixel>threshold;
    bool p;
    int Width = pBmp->getSize().x;
    for(int x=0; x<Width; x++) {
        p = *pPixel>threshold;
        if (cur!=p) {
            if (cur) {
                // Only if the run is longer than one pixel.
                if (x-run_start > 1) {
                    run_stop = x;
                    pRuns->push_back(Run(y, run_start, run_stop));
                    run_start = x;
                }
            } else {
                run_stop = x - 1;
                if (run_stop-run_start == 0 && !pRuns->empty()) {
                    // Single dark pixel: ignore the pixel, revive the last run.
                    run_start = pRuns->back().m_StartCol;
                    pRuns->pop_back();
                } else {
                    run_start = x;
                }
            }
            cur = p;
        }
        pPixel++;
    }
    if (cur){
        pRuns->push_back(Run(y, run_start, Width));
    }

}

BlobVectorPtr connected_components(BitmapPtr image, unsigned char threshold)
{
    if (threshold == 0) {
        return BlobVectorPtr();
    }
    assert(image->getPixelFormat() == I8);
    BlobVectorPtr pBlobs = BlobVectorPtr(new BlobVector);
    IntPoint size = image->getSize();
    RunArray *runs1=new RunArray();
    RunArray *runs2=new RunArray();
    RunArray *tmp;

    int y=0;
    findRunsInLine(image, 0, runs1, threshold);
    for (RunArray::iterator run1_it = runs1->begin(); run1_it!=runs1->end(); ++run1_it) {
        BlobPtr pBlob = BlobPtr(new Blob(*run1_it));
        pBlobs->push_back(pBlob);
        run1_it->m_pBlob = pBlob;
    }
    
    for(y=1; y<size.y; y++){
        findRunsInLine(image, y, runs2, threshold);
        store_runs(pBlobs, runs1, runs2);
        tmp = runs1;
        runs1 = runs2;
        runs2 = tmp;
        runs2->clear();
    }
    BlobVectorPtr pResultBlobs = BlobVectorPtr(new BlobVector);
    for (BlobVector::iterator it = pBlobs->begin(); it != pBlobs->end(); ++it) {
        if (!(*it)->m_pParent) {
            pResultBlobs->push_back(*it);
            (*it)->calcStats();
        }
    }
    delete runs1;
    delete runs2;
    return pResultBlobs;
}

}
