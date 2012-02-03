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

#include <iostream>
#include <string.h>

using namespace std;

namespace avg {

FreqFilter::FreqFilter(const IntPoint& size, const std::vector<float>& frequencies)
    : m_Size(size),
      m_Frequencies(frequencies)
{

    m_pInData = (float*) fftwf_malloc(sizeof(float) * size.x * size.y);
    m_pFreqData = (fftwf_complex*) fftwf_malloc(
            sizeof(fftwf_complex) * size.y * getFreqStride());
    m_fftPlan = fftwf_plan_dft_r2c_2d(size.y, size.x, m_pInData, m_pFreqData,
            FFTW_ESTIMATE);
    m_pLowpassData = (fftwf_complex*) fftwf_malloc(
            sizeof(fftwf_complex) * size.y * getFreqStride());
    m_pOutData = (float*) fftwf_malloc(sizeof(float) * size.x * size.y);
    m_ifftPlan = fftwf_plan_dft_c2r_2d(size.y, size.x, m_pLowpassData, m_pOutData,
            FFTW_ESTIMATE);
}

FreqFilter::~FreqFilter()
{
    fftwf_destroy_plan(m_fftPlan);
    fftwf_destroy_plan(m_ifftPlan);
    fftwf_free(m_pInData);
    fftwf_free(m_pFreqData);
    fftwf_free(m_pLowpassData);
    fftwf_free(m_pOutData);
}

void FreqFilter::filterImage(BitmapPtr pSrcBmp)
{
    unsigned char * pBmpPixels = pSrcBmp->getPixels();
    int stride = pSrcBmp->getStride();
    for (int y=0; y<m_Size.y; ++y) {
        for (int x=0; x<m_Size.x; ++x) {
            m_pInData[m_Size.x*y + x] = pBmpPixels[stride*y + x];
        }
    }

    fftwf_execute(m_fftPlan);

    m_LowpassBmps.clear();

    for (unsigned i=0; i<m_Frequencies.size(); ++i) {
        memcpy(m_pLowpassData, m_pFreqData,
                sizeof(fftwf_complex) * m_Size.y * getFreqStride());
        float cutoffFreq = m_Frequencies[i];
        float radius = cutoffFreq*cutoffFreq;
        float xmult = 1.f/(m_Size.x*m_Size.x);
        float ymult = 1.f/(m_Size.y*m_Size.y);
        for (int y=0; y<m_Size.y; ++y) {
            for (int x=0; x<getFreqStride(); ++x) {
                int fy = y;
                if (fy > m_Size.y/2) {
                    fy -= m_Size.y;
                }
                if (fy*fy*ymult + x*x*xmult > radius) {
                    fftwf_complex * pCurData = &(m_pLowpassData[y*getFreqStride() + x]);
                    (*pCurData)[0] = 0.0;
                    (*pCurData)[1] = 0.0;
                }
            }
        }
        fftwf_execute(m_ifftPlan);
        BitmapPtr pLowpassBmp(new Bitmap(m_Size, I8));
        pBmpPixels = pLowpassBmp->getPixels();
        stride = pLowpassBmp->getStride();
        for (int y=0; y<m_Size.y; ++y) {
            for (int x=0; x<m_Size.x; ++x) {
                float curPixel = m_pOutData[m_Size.x*y + x];
                if (curPixel >= 0) {
                    pBmpPixels[stride*y + x] = curPixel/float(m_Size.x*m_Size.y);
                } else {
                    pBmpPixels[stride*y + x] = 0;
                }
            }
        }
        m_LowpassBmps.push_back(pLowpassBmp);
    }
}

BitmapPtr FreqFilter::getFreqImage() const
{
    BitmapPtr pFreqBmp(new Bitmap(m_Size+IntPoint(2,0), I8));
    unsigned char * pBmpPixels = pFreqBmp->getPixels();
    int stride = pFreqBmp->getStride();
    for (int y=0; y<m_Size.y; ++y) {
        for (int x=0; x<getFreqStride(); ++x) {
            float curPixel0 = fabs(m_pFreqData[getFreqStride()*y + x][0]);
            float curPixel1 = fabs(m_pFreqData[getFreqStride()*y + x][1]);
            pBmpPixels[stride*y + x*2] = curPixel0;
            pBmpPixels[stride*y + x*2 + 1] = curPixel0;
//            cerr << "(" << x << ", " << y << "): " << curPixel0 << endl;
        }
    }
    return pFreqBmp;
}

BitmapPtr FreqFilter::getBandpassImage(int i) const
{
    return m_LowpassBmps[i];
}

int FreqFilter::getFreqStride() const
{
    return m_Size.x/2 + 1;
}

}
