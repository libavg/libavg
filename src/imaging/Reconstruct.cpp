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

BitmapPtr bandpass(BitmapPtr pSrcBmp)
{
    IntPoint size = pSrcBmp->getSize();
    cerr << size << endl;
    fftwf_plan srcPlan;
    
    float * pInData = (float*) fftwf_malloc(sizeof(float) * size.x * size.y);
    unsigned char * pBmpPixels = pSrcBmp->getPixels();
    int stride = pSrcBmp->getStride();
    for (int y=0; y<size.y; ++y) {
        for (int x=0; x<size.x; ++x) {
            pInData[size.x*y + x] = pBmpPixels[stride*y + x];
        }
    }

    int freqStride = size.x/2+1;
    fftwf_complex * pFreqData = (fftwf_complex*) fftwf_malloc(
            sizeof(fftwf_complex) * size.y * freqStride);
    srcPlan = fftwf_plan_dft_r2c_2d(size.x, size.y, pInData, pFreqData, FFTW_ESTIMATE);
    
    fftwf_execute(srcPlan);
    fftwf_destroy_plan(srcPlan);
/*
    int radius = (10*10);
    for (int y=0; y<size.y; ++y) {
        for (int x=0; x<freqStride; ++x) {
            if (y*y + x*x > radius) {
                fftwf_complex * pCurData = &(pFreqData[y*freqStride + x]);
                (*pCurData)[0] = 0.0;
                (*pCurData)[1] = 0.0;
            }
        }
    }
*/
    float * pOutData = (float*) fftwf_malloc(sizeof(float) * size.x * size.y);
    fftwf_plan destPlan = fftwf_plan_dft_c2r_2d(size.x, size.y, pFreqData, pOutData, 
            FFTW_ESTIMATE);
    fftwf_execute(destPlan);
    fftwf_destroy_plan(destPlan);
    
    BitmapPtr pDestBmp(new Bitmap(size, I8));
    pBmpPixels = pDestBmp->getPixels();
    stride = pDestBmp->getStride();
    for (int y=0; y<size.y; ++y) {
        for (int x=0; x<size.x; ++x) {
            pBmpPixels[stride*y + x] = pOutData[size.x*y + x]/(size.x*size.y);
        }
    }

    fftwf_free(pInData);
    fftwf_free(pFreqData);
    fftwf_free(pOutData);

    return pDestBmp;
}

}
