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

#include "FreqFilter.h"

#include "../base/Logger.h"
#include "../base/ProfilingZone.h"
#include "../base/ScopeTimer.h"
#include "../base/Exception.h"
#include "../base/StringHelper.h"

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
    m_pBPData = (float*) fftwf_malloc(sizeof(float) * size.x * size.y);
    m_ifftPlan = fftwf_plan_dft_c2r_2d(size.y, size.x, m_pFreqData, m_pBPData,
            FFTW_MEASURE);
    for (unsigned i=0; i<m_Frequencies.size(); ++i) {
        m_pBPFreqData.push_back((fftwf_complex*) fftwf_malloc(
                sizeof(fftwf_complex) * size.y * getFreqStride()));
        if (m_Frequencies[i] > m_Size.x/2 || m_Frequencies[i] > m_Size.y/2) {
            throw Exception(AVG_ERR_OUT_OF_RANGE, 
                    "Frequency " + toString(m_Frequencies[i]) + " > half of image size.");        
        }
        BitmapPtr pBmp(new Bitmap(m_Size, I8));
        m_pBPBmps.push_back(pBmp);
        m_pBPFreqBmps.push_back(BitmapPtr());
    }
}

FreqFilter::~FreqFilter()
{
    fftwf_destroy_plan(m_fftPlan);
    fftwf_destroy_plan(m_ifftPlan);
    fftwf_free(m_pInData);
    fftwf_free(m_pFreqData);
    for (unsigned i=0; i<m_pBPFreqData.size(); ++i) {
        fftwf_free(m_pBPFreqData[i]);
    }
    m_pBPFreqData.clear();
}

static ProfilingZoneID ProfilingZoneTotal("Total");

static ProfilingZoneID ProfilingZoneInputCopy("Copy input data");
static ProfilingZoneID ProfilingZoneFFT("Forward FFT");
static ProfilingZoneID ProfilingZoneBandpass("Bandpass");
static ProfilingZoneID ProfilingZoneFreqCalc("Frequency calculations");
static ProfilingZoneID ProfilingZoneIFFT("Inverse FFT");
static ProfilingZoneID ProfilingZoneSubtractBmps("Subtract bitmaps");
static ProfilingZoneID ProfilingZoneCopyOutput("Copy output data");

void FreqFilter::filterImage(BitmapPtr pSrcBmp)
{
    ScopeTimer timer(ProfilingZoneTotal);
    {
        ScopeTimer timer(ProfilingZoneInputCopy);
        copyBmpToFloatBuffer(pSrcBmp, m_pInData);
    }

    {
        ScopeTimer timer(ProfilingZoneFFT);
        fftwf_execute(m_fftPlan);
    }

    for (unsigned i=0; i<m_Frequencies.size(); ++i) {
        ScopeTimer timer(ProfilingZoneBandpass);
        {
            ScopeTimer timer(ProfilingZoneFreqCalc);
            doFreqDomainLowpass(m_pFreqData, m_pBPFreqData[i], m_Frequencies[i]);
            m_pBPFreqBmps[i] = cvtFreqDataToBmp(m_pBPFreqData[i]);
        }
        {
            ScopeTimer timer(ProfilingZoneIFFT);
            fftwf_execute_dft_c2r(m_ifftPlan, m_pBPFreqData[i], m_pBPData);
        }

        {
            ScopeTimer timer(ProfilingZoneCopyOutput);
            float sizeScale = 1.f/(m_Size.x*m_Size.y);
            float bandpassScale = 1.f; // (m_Size.x*m_Size.x) / (4*maxRadius);
            float scale = sizeScale * bandpassScale;
            BitmapPtr pBandpassBmp = m_pBPBmps[i];
            unsigned char * pBmpPixels = pBandpassBmp->getPixels();
            int stride = pBandpassBmp->getStride();
            for (int y=0; y<m_Size.y; ++y) {
                for (int x=0; x<m_Size.x; ++x) {
                    float curPixel = m_pBPData[m_Size.x*y + x]*scale;
                    pBmpPixels[stride*y + x] = fabs(curPixel);
                }
            }
        }
    }
}

BitmapPtr FreqFilter::getFreqImage() const
{
    return cvtFreqDataToBmp(m_pFreqData);
}

BitmapPtr FreqFilter::getFreqBPImage(int i) const
{
    return m_pBPFreqBmps[i];
}

BitmapPtr FreqFilter::getBandpassImage(int i) const
{
    return m_pBPBmps[i];
}
    
void FreqFilter::copyBmpToFloatBuffer(BitmapPtr pBmp, float* pBuffer)
{
    unsigned char * pBmpPixels = pBmp->getPixels();
    int stride = pBmp->getStride();
    IntPoint size = pBmp->getSize();
    for (int y=0; y<size.y; ++y) {
        for (int x=0; x<size.x; ++x) {
            pBuffer[size.x*y + x] = pBmpPixels[stride*y + x];
        }
    }
}

void FreqFilter::doFreqDomainBandpass(const fftwf_complex * pInBuffer,
        fftwf_complex * pOutBuffer, float minFreq, float maxFreq)
{
    float minRadius = minFreq*minFreq;
    float maxRadius = maxFreq*maxFreq;
    float ymult = (m_Size.x*m_Size.x)/(m_Size.y*m_Size.y);
    int stride = getFreqStride();
    for (int y=0; y<m_Size.y; ++y) {
        int fy = y;
        if (fy > m_Size.y/2) {
            fy -= m_Size.y;
        }
        for (int x=0; x<stride; ++x) {
            int offset = y*stride + x;
            float radius = fy*fy*ymult + x*x;
            if (radius >= minRadius && radius < maxRadius) {
                pOutBuffer[offset][0] = pInBuffer[offset][0];
                pOutBuffer[offset][1] = pInBuffer[offset][1];
            } else {
                pOutBuffer[offset][0] = 0.f;
                pOutBuffer[offset][1] = 0.f;
            }
        }
    }
}

void FreqFilter::doFreqDomainLowpass(const fftwf_complex * pInBuffer,
        fftwf_complex * pOutBuffer, float freq)
{
    float radius2 = freq*freq;
    float ymult = float(m_Size.x*m_Size.x)/(m_Size.y*m_Size.y);
    int stride = getFreqStride();
    for (int y=0; y<m_Size.y; ++y) {
        int fy = y;
        if (fy > m_Size.y/2) {
            fy -= m_Size.y;
        }
        for (int x=0; x<stride; ++x) {
            int offset = y*stride + x;
            float curRadius2 = fy*fy*ymult + x*x;
            if (curRadius2 < radius2) {
                pOutBuffer[offset][0] = pInBuffer[offset][0];
                pOutBuffer[offset][1] = pInBuffer[offset][1];
            } else {
                pOutBuffer[offset][0] = 0.f;
                pOutBuffer[offset][1] = 0.f;
            }
        }
    }
}

BitmapPtr FreqFilter::cvtFreqDataToBmp(fftwf_complex * pFreqData) const
{
    BitmapPtr pFreqBmp(new Bitmap(m_Size+IntPoint(2,0), I8));
    unsigned char * pBmpPixels = pFreqBmp->getPixels();
    int stride = pFreqBmp->getStride();
    for (int y=0; y<m_Size.y; ++y) {
        for (int x=0; x<getFreqStride(); ++x) {
            float curPixel0 = fabs(pFreqData[getFreqStride()*y + x][0]);
            pBmpPixels[stride*y + x*2] = curPixel0;
            pBmpPixels[stride*y + x*2 + 1] = curPixel0;
        }
    }
    return pFreqBmp;

}

int FreqFilter::getFreqStride() const
{
    return m_Size.x/2 + 1;
}

}
