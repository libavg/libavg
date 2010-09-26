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

#include "Blob.h"

#include "../base/ObjectCounter.h"
#include "../base/Exception.h"

#include "../graphics/Filterfill.h"
#include "../graphics/Pixel8.h"

#include <stdlib.h>
#include <math.h>

#include <climits>
#include <iostream>
#include <algorithm>

using namespace std;

namespace avg {

Blob::Blob(const Run& run)
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

void Blob::addRun(const Run& run)
{
    AVG_ASSERT((m_Runs.end()-1)->m_Row <= run.m_Row);
    m_Runs.push_back(run);
}


void Blob::merge(const BlobPtr& pOtherBlob)
{
    AVG_ASSERT(pOtherBlob);
    RunArray * pOtherRuns = pOtherBlob->getRuns();
    m_Runs.insert(m_Runs.end(), pOtherRuns->begin(), pOtherRuns->end());
    pOtherRuns->clear();
}

void Blob::render(BitmapPtr pSrcBmp, BitmapPtr pDestBmp, Pixel32 color, 
        int min, int max, bool bFinger, bool bMarkCenter, Pixel32 centerColor)
{
    AVG_ASSERT(pSrcBmp);
    AVG_ASSERT(pDestBmp);
    AVG_ASSERT(pSrcBmp->getBytesPerPixel() == 1);
    AVG_ASSERT(pDestBmp->getBytesPerPixel() == 4);
    AVG_ASSERT(pSrcBmp->getSize() == pDestBmp->getSize());
    unsigned char *pSrc;
    unsigned char *pDest;
    unsigned char *pColor = (unsigned char *)(&color);
    int intensityScale = 2*256/(std::max(max-min, 1));
    for (RunArray::iterator it = m_Runs.begin(); it != m_Runs.end(); ++it) {
        AVG_ASSERT(it->m_Row < pSrcBmp->getSize().y);
        AVG_ASSERT(it->m_StartCol >= 0);
        AVG_ASSERT(it->m_EndCol <= pSrcBmp->getSize().x);
        pSrc = pSrcBmp->getPixels()+it->m_Row*pSrcBmp->getStride();
        pDest = pDestBmp->getPixels()+it->m_Row*pDestBmp->getStride();
        int x = it->m_StartCol;
        pSrc += x;
        pDest+= x*4;
        while (x < it->m_EndCol) {
            int factor = (*pSrc-min)*intensityScale;
            if (factor < 0) {
                factor = 0;
            }
            if (factor > 255) {
                factor = 255;
            }
            *(pDest++) = ((*pColor)*factor) >> 8;
            *(pDest++) = ((*(pColor+1))*factor) >> 8;
            *(pDest++) = ((*(pColor+2))*factor) >> 8;
            *(pDest++) = ((*(pColor+3))*factor) >> 8;
            pSrc++;
            x++;
        }
    }
    AVG_ASSERT(m_bStatsAvailable);
    if (bMarkCenter) {
        IntPoint center = IntPoint(int(m_Center.x+0.5), int(m_Center.y+0.5));
        
        IntPoint end0 = IntPoint(m_ScaledBasis[0])+center;
        pDestBmp->drawLine(center, end0, centerColor);
        IntPoint end1 = IntPoint(m_ScaledBasis[1])+center;
        pDestBmp->drawLine(center, end1, centerColor);

        if (bFinger && m_RelatedBlobs.size() > 0) {
            // Draw finger direction
            BlobPtr pHandBlob = (m_RelatedBlobs)[0].lock();
            if (pHandBlob) {
                pDestBmp->drawLine(center, IntPoint(pHandBlob->getCenter()),
                        Pixel32(0xD7, 0xC9, 0x56, 0xFF));
            }
        }
        if (!m_Contour.empty()) {
            for (vector<IntPoint>::iterator it = m_Contour.begin()+1; 
                    it != m_Contour.end(); ++it)
            {
                IntPoint pt1 = *(it-1);
                IntPoint pt2 = *it;
                pDestBmp->drawLine(pt1, pt2, centerColor);
            }
            pDestBmp->drawLine(*(m_Contour.end()-1), *m_Contour.begin(), centerColor);
        }
    }
}
        
bool Blob::contains(IntPoint pt)
{
    for (RunArray::iterator it = m_Runs.begin(); it != m_Runs.end(); ++it) {
        if (it->m_Row == pt.y && it->m_StartCol <= pt.x && it->m_EndCol > pt.x) {
            return true;
        } 
    }
    return false;
}

void Blob::calcStats()
{
    m_Center = calcCenter();
    m_EstimatedNextCenter = m_Center;
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
    for (RunArray::iterator r = m_Runs.begin(); r != m_Runs.end();++r) {
        //This is the evaluated expression for the variance when using runs...
        ll = r->length();
        c_yy += ll* (r->m_Row- m_Center.y)*(r->m_Row- m_Center.y);
        c_xx += ( (r->m_EndCol-1) * r->m_EndCol * (2*r->m_EndCol-1) 
                - (r->m_StartCol-1) * r->m_StartCol * (2*r->m_StartCol -1))/6. 
            - m_Center.x * ((r->m_EndCol-1)*r->m_EndCol-(r->m_StartCol-1)*r->m_StartCol)
            + ll* m_Center.x*m_Center.x;
        c_xy += (r->m_Row-m_Center.y)*0.5*( (r->m_EndCol-1)*r->m_EndCol
                - (r->m_StartCol-1)*r->m_StartCol) 
                + ll *(m_Center.x*m_Center.y - m_Center.x*r->m_Row);
    }

    c_xx /= m_Area;
    c_yy /= m_Area;
    c_xy /= m_Area;

    m_Inertia = c_xx + c_yy;

    double T = sqrt( (c_xx - c_yy) * (c_xx - c_yy) + 4*c_xy*c_xy);
    m_Eccentricity = ((c_xx + c_yy) + T)/((c_xx+c_yy) - T);
    m_Orientation = 0.5*atan2(2*c_xy,c_xx-c_yy);
    // The l_i are variances (unit L^2) so to arrive at numbers that 
    // correspond to lengths in the picture we use sqrt
    // Ensure that eigenvectors always have standard orientation, i.e. the determinant
    // of the matrix with the eigenvectors as columns is >0
    //             E_1.x E_2.y - E_1.y E_2.x > 0
    if (fabs(c_xy) > 1e-30) {
        //FIXME. check l1!=0 l2!=0. li=0 happens for line-like components
        l1 = 0.5 * ((c_xx+c_yy) + sqrt((c_xx+c_yy)*(c_xx+c_yy)-4*(c_xx*c_yy-c_xy*c_xy)));
        l2 = 0.5 * ((c_xx+c_yy) - sqrt((c_xx+c_yy)*(c_xx+c_yy)-4*(c_xx*c_yy-c_xy*c_xy)));
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
        if (m_EigenVector[0].x*m_EigenVector[1].y - m_EigenVector[0].y*m_EigenVector[1].x
                < 0)
        {
            m_EigenVector[0] *= -1;
        }
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
            m_EigenVector[0].y = -1;
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

const DPoint& Blob::getCenter() const
{
    return m_Center;
}

const DPoint& Blob::getEstimatedNextCenter() const
{
    return m_EstimatedNextCenter;
}

double Blob::getArea() const
{
    return m_Area;
}

const IntRect& Blob::getBoundingBox() const
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

void Blob::calcNextCenter(DPoint oldCenter)
{
    m_EstimatedNextCenter = m_Center + (m_Center - oldCenter);
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
    DPoint center(0,0);
    double c = 0;
    for (RunArray::iterator r = m_Runs.begin(); r != m_Runs.end(); ++r) {
        center += r->m_Center*r->length();
        c += r->length();
    }
    center = center/c;
    return center;
}

IntRect Blob::calcBBox()
{
    int x1 = INT_MAX;
    int y1 = INT_MAX;
    int x2 = 0;
    int y2 = 0;
    for (RunArray::iterator r = m_Runs.begin(); r!=m_Runs.end(); ++r) {
        x1 = std::min(x1, r->m_StartCol);
        y1 = std::min(y1, r->m_Row);
        x2 = std::max(x2, r->m_EndCol);
        y2 = std::max(y2, r->m_Row);
    }
    return IntRect(x1, y1, x2, y2);
}

int Blob::calcArea()
{
    int res = 0;
    for (RunArray::iterator r = m_Runs.begin(); r != m_Runs.end(); ++r) {
        res+= r->length();
    }
    return res;
}

void Blob::initRowPositions()
{
    int offset = m_BoundingBox.tl.y;
    RunArray::iterator it = m_Runs.begin();
    for (int i = 0; i < m_BoundingBox.height(); i++) {
        while (it->m_Row-offset < i) {
            it++;
        }
        m_RowPositions.push_back(it);
    }
}

IntPoint getNeighbor(const IntPoint& pt, int dir)
{
    IntPoint neighborPt(pt);
    // dir encoding:
    //  3  2  1
    //  4 pt  0
    //  5  6  7
    switch(dir) {
        case 0:
        case 1:
        case 7:
            neighborPt.x++;
            break;
        case 3:
        case 4:
        case 5:
            neighborPt.x--;
            break;
        default:
            break;
    };
    switch(dir) {
        case 1:
        case 2:
        case 3:
            neighborPt.y--;
            break;
        case 5:
        case 6:
        case 7:
            neighborPt.y++;
            break;
        default:
            break;
    };
    return neighborPt;
}

IntPoint Blob::findNeighborInside(const IntPoint& pt, int& dir)
{
    if (dir & 1) {
        dir += 2;
    } else {
        dir++;
    }
    if (dir > 7) {
        dir -= 8;
    }

    for (int i = 0; i < 8; i++) {
        IntPoint curPt = getNeighbor(pt, dir);
        if (ptIsInBlob(curPt)) {
            return curPt;
        } else {
            dir--;
            if (dir < 0) {
                dir += 8;
            }
        }
    }
    AVG_ASSERT(false);
    return pt;
}

bool runIsLess(const Run& r1, const Run& r2)
{
    return r1.m_Row < r2.m_Row;
}

void Blob::calcContour(int precision)
{
    sort(m_Runs.begin(), m_Runs.end(), runIsLess);
    initRowPositions();
    
    // Moore Neighbor Tracing.
    IntPoint boundaryPt(m_Runs[0].m_StartCol, m_Runs[0].m_Row);
    IntPoint firstPt(boundaryPt);
    int i = precision;
    int dir = 1;
    do {
        i++;
        if (i >= precision) {
            m_Contour.push_back(boundaryPt);
            i = 0;
        }
        boundaryPt = findNeighborInside(boundaryPt, dir);
    } while (firstPt != boundaryPt);
}

ContourSeq Blob::getContour()
{
    return m_Contour;
}

bool Blob::ptIsInBlob(const IntPoint& pt)
{
    if (m_BoundingBox.contains(pt)) {
        RunArray::iterator it = m_RowPositions[pt.y-m_BoundingBox.tl.y];
        while (it->m_Row == pt.y) {
            if (pt.x >= it->m_StartCol && pt.x < it->m_EndCol) {
                return true;
            }
            it++;
        }
    }
    return false;
}

bool areConnected(const Run & run1, const Run & run2)
{
    if (run1.m_StartCol > run2.m_StartCol) {
        return run2.m_EndCol > run1.m_StartCol;
    } else {
        return run1.m_EndCol > run2.m_StartCol;
    }
}

void storeRuns(BlobVectorPtr pBlobs, RunArray* pUpperRuns, RunArray* pLowerRuns)
{
    for (RunArray::iterator run1_it = pUpperRuns->begin(); run1_it!=pUpperRuns->end();
            ++run1_it)
    {
        for (RunArray::iterator run2_it = pLowerRuns->begin(); run2_it!=pLowerRuns->end();
                ++run2_it)
        {
            if (run2_it->m_StartCol > run1_it->m_EndCol) {
                break;
            }
            if (areConnected(*run1_it, *run2_it)) {
                BlobPtr pBlob = run1_it->m_pBlob.lock();
                while (pBlob->m_pParent) {
                    pBlob = pBlob->m_pParent;
                }
                if (!(run2_it->m_pBlob.expired())) {
                    BlobPtr c_blob = run2_it->m_pBlob.lock();
                    while (c_blob->m_pParent) {
                        c_blob = c_blob->m_pParent;
                    }
                    if (c_blob != pBlob) {
                        // When we merge, make sure the smaller blob is merged
                        // into the bigger one to avoid a speed hit.
                        if (pBlob->getRuns()->size() > c_blob->getRuns()->size()) {
                            pBlob->merge(c_blob); //destroys c_blobs runs_list
                            c_blob->m_pParent = pBlob;
                        } else {
                            c_blob->merge(pBlob); 
                            pBlob->m_pParent = c_blob;
                        }
                    }
                } else {
                    run2_it->m_pBlob = pBlob;
                    pBlob->addRun(*run2_it);
                }
            }
        }
    }
    for (RunArray::iterator run2_it = pLowerRuns->begin(); run2_it!=pLowerRuns->end();
            ++run2_it)
    {
        if (run2_it->m_pBlob.expired()) {
            BlobPtr pBlob = BlobPtr(new Blob(*run2_it));
            pBlobs->push_back(pBlob);
            run2_it->m_pBlob = pBlob;
        }
    }
}

void findRunsInLine(BitmapPtr pBmp, int y, RunArray* pRuns, unsigned char threshold)
{
    int runStart=0;
    int runStop=0;
    const unsigned char * pPixel = pBmp->getPixels()+y*pBmp->getStride();
    bool bIsInRun = *pPixel > threshold;

    int width = pBmp->getSize().x;
    for (int x = 0; x < width; x++) {
        bool bPixelInRun = *pPixel > threshold;
        if (bIsInRun != bPixelInRun) {
            if (bIsInRun) {
                // Only if the run is longer than one pixel.
                if (x-runStart > 1) {
                    runStop = x;
                    pRuns->push_back(Run(y, runStart, runStop));
                    runStart = x;
                }
            } else {
                runStop = x - 1;
                if (runStop-runStart == 0 && !pRuns->empty()) {
                    // Single dark pixel: ignore the pixel, revive the last run.
                    runStart = pRuns->back().m_StartCol;
                    pRuns->pop_back();
                } else {
                    runStart = x;
                }
            }
            bIsInRun = bPixelInRun;
        }
        pPixel++;
    }
    if (bIsInRun) {
        pRuns->push_back(Run(y, runStart, width));
    }

}

BlobVectorPtr findConnectedComponents(BitmapPtr pBmp, unsigned char threshold)
{
    AVG_ASSERT(pBmp->getPixelFormat() == I8);
    BlobVectorPtr pBlobs = BlobVectorPtr(new BlobVector);
    IntPoint size = pBmp->getSize();
    RunArray* pUpperRuns = new RunArray();
    RunArray* pLowerRuns = new RunArray();

    int y = 0;
    findRunsInLine(pBmp, 0, pUpperRuns, threshold);
    for (RunArray::iterator it = pUpperRuns->begin(); it!=pUpperRuns->end(); ++it) 
    {
        BlobPtr pBlob = BlobPtr(new Blob(*it));
        pBlobs->push_back(pBlob);
        it->m_pBlob = pBlob;
    }
    
    for (y = 1; y < size.y; y++) {
        findRunsInLine(pBmp, y, pLowerRuns, threshold);
        storeRuns(pBlobs, pUpperRuns, pLowerRuns);
        RunArray* pTmpRuns = pUpperRuns;
        pUpperRuns = pLowerRuns;
        pLowerRuns = pTmpRuns;
        pLowerRuns->clear();
    }
    BlobVectorPtr pResultBlobs = BlobVectorPtr(new BlobVector);
    for (BlobVector::iterator it = pBlobs->begin(); it != pBlobs->end(); ++it) {
        if (!(*it)->m_pParent) {
            pResultBlobs->push_back(*it);
            (*it)->calcStats();
        }
    }
    delete pUpperRuns;
    delete pLowerRuns;
    return pResultBlobs;
}


}
