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
            FFTW_MEASURE | FFTW_PRESERVE_INPUT);
    m_pLPFreqData = (fftwf_complex*) fftwf_malloc(
            sizeof(fftwf_complex) * size.y * getFreqStride());
    m_pLPData = (float*) fftwf_malloc(sizeof(float) * size.x * size.y);
    m_pPrevLPData = (float*) fftwf_malloc(sizeof(float) * size.x * size.y);
    m_pBPData = (float*) fftwf_malloc(sizeof(float) * size.x * size.y);
    m_ifftPlan = fftwf_plan_dft_c2r_2d(size.y, size.x, m_pLPFreqData, m_pLPData,
            FFTW_MEASURE);
}

FreqFilter::~FreqFilter()
{
    fftwf_destroy_plan(m_fftPlan);
    fftwf_destroy_plan(m_ifftPlan);
    fftwf_free(m_pInData);
    fftwf_free(m_pFreqData);
    fftwf_free(m_pLPFreqData);
    fftwf_free(m_pLPData);
    fftwf_free(m_pPrevLPData);
}

static ProfilingZoneID ProfilingZoneTotal("Total");

static ProfilingZoneID ProfilingZoneInputCopy("Copy input data");
static ProfilingZoneID ProfilingZoneFFT("Forward FFT");
static ProfilingZoneID ProfilingZoneBandpass("Bandpass");
static ProfilingZoneID ProfilingZoneLowpass("Lowpass");
static ProfilingZoneID ProfilingZoneIFFT("Inverse FFT");
static ProfilingZoneID ProfilingZoneSubtractBmps("Subtract bitmaps");
static ProfilingZoneID ProfilingZoneCopyOutput("Copy output data");

void FreqFilter::filterImage(BitmapPtr pSrcBmp)
{
    ScopeTimer timer(ProfilingZoneTotal);
    {
        ScopeTimer timer(ProfilingZoneInputCopy);
        unsigned char * pBmpPixels = pSrcBmp->getPixels();
        int stride = pSrcBmp->getStride();
        for (int y=0; y<m_Size.y; ++y) {
            for (int x=0; x<m_Size.x; ++x) {
                m_pInData[m_Size.x*y + x] = pBmpPixels[stride*y + x];
            }
        }
    }

    {
        ScopeTimer timer(ProfilingZoneFFT);
        fftwf_execute(m_fftPlan);
    }

    m_pBPBmps.clear();

    for (unsigned i=0; i<m_Frequencies.size(); ++i) {
        ScopeTimer timer(ProfilingZoneBandpass);
        {
            ScopeTimer timer(ProfilingZoneLowpass);
            memcpy(m_pLPFreqData, m_pFreqData,
                    sizeof(fftwf_complex) * m_Size.y * getFreqStride());
            float cutoffFreq = m_Frequencies[i];
            float radius = cutoffFreq*cutoffFreq;
            float xmult = 1.f/(m_Size.x*m_Size.x);
            float ymult = 1.f/(m_Size.y*m_Size.y);
            for (int y=0; y<m_Size.y; ++y) {
                int fy = y;
                if (fy > m_Size.y/2) {
                    fy -= m_Size.y;
                }
                for (int x=0; x<getFreqStride(); ++x) {
                    if (fy*fy*ymult + x*x*xmult > radius) {
                        fftwf_complex * pCurData = &(m_pLPFreqData[y*getFreqStride() + x]);
                        (*pCurData)[0] = 0.0;
                        (*pCurData)[1] = 0.0;
                    }
                }
            }
        }
        {
            ScopeTimer timer(ProfilingZoneIFFT);
            fftwf_execute_dft_c2r(m_ifftPlan, m_pLPFreqData, m_pLPData);
        }

        // Execute bandpass by subtracting lowpass bitmaps from each other.
        {
            ScopeTimer timer(ProfilingZoneSubtractBmps);
            float * pOldLPData;
            if (i==0) {
                pOldLPData = m_pInData;
            } else {
                pOldLPData = m_pPrevLPData;
            }
           
            float scale = 1.f/(m_Size.x*m_Size.y);
            for (int y=0; y<m_Size.y; ++y) {
                for (int x=0; x<m_Size.x; ++x) {
                    int offset = m_Size.x*y + x;
                    m_pLPData[offset] *= scale;
                    m_pBPData[offset] = m_pLPData[offset] - pOldLPData[offset];
                }
            }
        }

        // Exchange buffers
        float* pTemp = m_pPrevLPData;
        m_pPrevLPData = m_pLPData;
        m_pLPData = pTemp;

        {
            ScopeTimer timer(ProfilingZoneCopyOutput);
            BitmapPtr pBandpassBmp(new Bitmap(m_Size, I8));
            unsigned char * pBmpPixels = pBandpassBmp->getPixels();
            int stride = pBandpassBmp->getStride();
            for (int y=0; y<m_Size.y; ++y) {
                for (int x=0; x<m_Size.x; ++x) {
                    float curPixel = m_pBPData[m_Size.x*y + x];
                    pBmpPixels[stride*y + x] = fabs(curPixel);
                }
            }
            m_pBPBmps.push_back(pBandpassBmp);
        }
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
//            float curPixel1 = fabs(m_pFreqData[getFreqStride()*y + x][1]);
            pBmpPixels[stride*y + x*2] = curPixel0;
            pBmpPixels[stride*y + x*2 + 1] = curPixel0;
        }
    }
    return pFreqBmp;
}

BitmapPtr FreqFilter::getBandpassImage(int i) const
{
    return m_pBPBmps[i];
}

int FreqFilter::getFreqStride() const
{
    return m_Size.x/2 + 1;
}

}
