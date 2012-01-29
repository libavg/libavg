//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2011 Ulrich von Zadow
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

#include "Reconstruct.h"

#include "../base/Logger.h"
#include "../base/ProfilingZone.h"
#include "../base/ScopeTimer.h"
#include "../base/Exception.h"

#include <fftw3.h>

#include <iostream>

using namespace std;

namespace avg {

BitmapPtr lowpass(BitmapPtr pSrcBmp, BitmapPtr& pFreqBmp, float cutoffFreq)
{
    IntPoint size = pSrcBmp->getSize();
    fftwf_plan srcPlan;
    float * pInData = (float*) fftwf_malloc(sizeof(float) * size.x * size.y);
    
    int freqStride = size.x/2+1;
    cerr << "freqStride: " << freqStride << endl;
    fftwf_complex * pFreqData = (fftwf_complex*) fftwf_malloc(
            sizeof(fftwf_complex) * size.y * freqStride);
    srcPlan = fftwf_plan_dft_r2c_2d(size.y, size.x, pInData, pFreqData, FFTW_ESTIMATE);
    
    unsigned char * pBmpPixels = pSrcBmp->getPixels();
    int stride = pSrcBmp->getStride();
    for (int y=0; y<size.y; ++y) {
        for (int x=0; x<size.x; ++x) {
            pInData[size.x*y + x] = pBmpPixels[stride*y + x];
        }
    }

    fftwf_execute(srcPlan);
    fftwf_destroy_plan(srcPlan);

    pFreqBmp = BitmapPtr(new Bitmap(size+IntPoint(2,0), I8));
    pBmpPixels = pFreqBmp->getPixels();
    stride = pFreqBmp->getStride();
    for (int y=0; y<size.y; ++y) {
        for (int x=0; x<freqStride; ++x) {
            float curPixel0 = fabs(pFreqData[freqStride*y + x][0]);
            float curPixel1 = fabs(pFreqData[freqStride*y + x][1]);
            pBmpPixels[stride*y + x*2] = curPixel0;
            pBmpPixels[stride*y + x*2 + 1] = curPixel0;
//            cerr << "(" << x << ", " << y << "): " << curPixel0 << endl;
        }
    }

    float radius = cutoffFreq*cutoffFreq;
    float xmult = 1.f/(size.x*size.x);
    float ymult = 1.f/(size.y*size.y);
    for (int y=0; y<size.y; ++y) {
        for (int x=0; x<freqStride; ++x) {
            
            int fy = y;
            if (fy > size.y/2) {
                fy -= size.y;
            }
/*            
            int fx = x;
            if (fx > freqStride/2) {
                fx -= freqStride;
            }
            */
            if (fy*fy*ymult + x*x*xmult > radius) {
                fftwf_complex * pCurData = &(pFreqData[y*freqStride + x]);
                (*pCurData)[0] = 0.0;
                (*pCurData)[1] = 0.0;
            }
        }
    }

    float * pOutData = (float*) fftwf_malloc(sizeof(float) * size.x * size.y);
    fftwf_plan destPlan = fftwf_plan_dft_c2r_2d(size.y, size.x, pFreqData, pOutData, 
            FFTW_ESTIMATE);
    fftwf_execute(destPlan);
    fftwf_destroy_plan(destPlan);
    
    BitmapPtr pDestBmp(new Bitmap(size, I8));
    pBmpPixels = pDestBmp->getPixels();
    stride = pDestBmp->getStride();
    for (int y=0; y<size.y; ++y) {
        for (int x=0; x<size.x; ++x) {
            float curPixel = pOutData[size.x*y + x];
            if (curPixel >= 0) {
                pBmpPixels[stride*y + x] = curPixel/float(size.x*size.y);
            } else {
                pBmpPixels[stride*y + x] = 0;
            }
        }
    }

    fftwf_free(pInData);
    fftwf_free(pFreqData);
    fftwf_free(pOutData);

    return pDestBmp;
}

}
