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
    fftw_plan srcPlan;
    
    double * pInData = (double*) fftw_malloc(sizeof(double) * size.x * size.y);
    unsigned char * pBmpPixels = pSrcBmp->getPixels();
    int stride = pSrcBmp->getStride();
    for (int y=0; y<size.y; ++y) {
        for (int x=0; x<size.x; ++x) {
            pInData[size.x*y + x] = pBmpPixels[stride*y + x];
        }
    }

    fftw_complex * pFreqData = (fftw_complex*) fftw_malloc(
            sizeof(fftw_complex) * size.x * (size.y/2+1));
    srcPlan = fftw_plan_dft_r2c_2d(size.x, size.y, pInData, pFreqData, FFTW_ESTIMATE);
    
    fftw_execute(srcPlan);
    fftw_destroy_plan(srcPlan);

    double * pOutData = (double*) fftw_malloc(sizeof(double) * size.x * size.y);
    fftw_plan destPlan = fftw_plan_dft_c2r_2d(size.x, size.y, pFreqData, pOutData, 
            FFTW_ESTIMATE);
    fftw_execute(destPlan);
    fftw_destroy_plan(destPlan);
    
    BitmapPtr pDestBmp(new Bitmap(size, I8));
    pBmpPixels = pDestBmp->getPixels();
    stride = pDestBmp->getStride();
    for (int y=0; y<size.y; ++y) {
        for (int x=0; x<size.x; ++x) {
            pBmpPixels[stride*y + x] = pOutData[size.x*y + x]/(size.x*size.y);
        }
    }

    fftw_free(pInData);
    fftw_free(pFreqData);
    fftw_free(pOutData);

    return pDestBmp;
}

}
