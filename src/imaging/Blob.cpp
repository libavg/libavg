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
#include <iostream>
#include <algorithm>

using namespace std;

namespace avg {

Blob::Blob(const RunPtr& pRun)
{
    m_pRuns = new RunArray();
    m_pRuns->reserve(50);
    m_pRuns->push_back(pRun);
    m_pParent = BlobPtr();
}

Blob::~Blob() 
{
    delete m_pRuns;
}

RunArray *Blob::getRuns()
{
    return m_pRuns;
}

void Blob::addRun(const RunPtr& pRun)
{
    m_pRuns->push_back(pRun);
}

void Blob::merge(const BlobPtr& other)
{
    assert(other);
    RunArray * other_runs=other->getRuns();
    m_pRuns->insert(m_pRuns->end(), other_runs->begin(), other_runs->end());
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
    for(RunArray::iterator r=m_pRuns->begin();r!=m_pRuns->end();++r) {
        pSrc = pSrcBmp->getPixels()+(*r)->m_Row*pSrcBmp->getStride();
        pDest = pDestBmp->getPixels()+(*r)->m_Row*pDestBmp->getStride();
        int x_pos = (*r)->m_StartCol;
        pSrc += x_pos;
        pDest+= x_pos*4;
        while(x_pos<(*r)->m_EndCol) {
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
    if(bMarkCenter) {
        BlobInfoPtr pInfo = getInfo();
        DPoint DCenter = pInfo->getCenter();
        IntPoint Center = IntPoint(int(DCenter.x+0.5), int(DCenter.y+0.5));
        
        IntPoint End0 = IntPoint(pInfo->getScaledBasis(0))+Center;
        pDestBmp->drawLine(Center, End0, CenterColor);
        IntPoint End1 = IntPoint(pInfo->getScaledBasis(1))+Center;
        pDestBmp->drawLine(Center, End1, CenterColor);

        
        if (bFinger && m_pBlobInfo->m_RelatedBlobs.size() > 0) {
            // Draw finger direction
            BlobInfoPtr pHandBlob = (m_pBlobInfo->m_RelatedBlobs)[0].lock();
            if (pHandBlob) {
                pDestBmp->drawLine(Center, IntPoint(pHandBlob->getCenter()),
                        Pixel32(0xD7, 0xC9, 0x56, 0xFF));
            }
        }
    }
}
        
bool Blob::contains(IntPoint pt)
{
    for(RunArray::iterator it=m_pRuns->begin(); it!=m_pRuns->end(); ++it) {
        if ((*it)->m_Row == pt.y && (*it)->m_StartCol <= pt.x && (*it)->m_EndCol > pt.x) {
            return true;
        } 
    }
    return false;
}

BlobInfoPtr Blob::getInfo()
{
    if (!m_pBlobInfo) {
        m_pBlobInfo = BlobInfoPtr(new BlobInfo(m_pRuns));
    }
    return m_pBlobInfo;
}

bool connected(RunPtr r1, RunPtr r2)
{
    if (r1->m_StartCol > r2->m_StartCol) {
        return r2->m_EndCol > r1->m_StartCol;
    } else {
        return r1->m_EndCol > r2->m_StartCol;
    }
}

void store_runs(BlobVectorPtr pComps, RunArray *runs1, RunArray *runs2)
{
    for (RunArray::iterator run1_it = runs1->begin(); run1_it!=runs1->end(); ++run1_it) {
        for (RunArray::iterator run2_it = runs2->begin(); run2_it!=runs2->end(); ++run2_it) {
            if ((*run2_it)->m_StartCol > (*run1_it)->m_EndCol) {
                break;
            }
            if (connected(*run1_it, *run2_it)) {
                BlobPtr p_blob = (*run1_it)->m_pBlob.lock();
                while (p_blob->m_pParent) {
                    p_blob = p_blob->m_pParent;
                }
                if (!((*run2_it)->m_pBlob.expired())) {
                    BlobPtr c_blob = (*run2_it)->m_pBlob.lock();
                    while (c_blob->m_pParent) {
                        c_blob = c_blob->m_pParent;
                    }
                    if (c_blob!=p_blob) {
                        p_blob->merge(c_blob); //destroys c_blobs runs_list
                        c_blob->m_pParent = p_blob;
                    }
                } else {
                    (*run2_it)->m_pBlob = p_blob;
                    p_blob->addRun(*run2_it);
                }
            }
        }
    }
    for (RunArray::iterator run2_it = runs2->begin(); run2_it!=runs2->end(); ++run2_it) {
        if ((*run2_it)->m_pBlob.expired()) {
            BlobPtr pBlob = BlobPtr(new Blob(*run2_it));
            pComps->push_back(pBlob);
            (*run2_it)->m_pBlob = pBlob;
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
                    pRuns->push_back(RunPtr(new Run(y, run_start, run_stop)));
                    run_start = x;
                }
            } else {
                run_stop = x - 1;
                if (run_stop-run_start == 0 && !pRuns->empty()) {
                    // Single dark pixel: ignore the pixel, revive the last run.
                    RunPtr pLastRun = pRuns->back();
                    run_start = pLastRun->m_StartCol;
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
        pRuns->push_back(RunPtr(new Run(y, run_start, Width)));
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
        (*run1_it)->m_pBlob = pBlob;
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
        }
    }
    delete runs1;
    delete runs2;
    return pResultBlobs;
}

}
