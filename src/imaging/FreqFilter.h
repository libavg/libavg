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

#ifndef _FreqFilter_H_
#define _FreqFilter_H_

#include "../graphics/Bitmap.h"

#include <fftw3.h>
#include <vector>

namespace avg {

class FreqFilter {
public:
    FreqFilter(const IntPoint& size, const std::vector<float>& frequencies);
    virtual ~FreqFilter();
    void filterImage(BitmapPtr pSrcBmp);
    BitmapPtr getFreqImage() const;
    BitmapPtr getBandpassImage(int i) const;

private:
    void copyBmpToFloatBuffer(BitmapPtr pSrcBmp, float* pBuffer);
    void doFreqDomainBandpass(const fftwf_complex * pInBuffer,
            fftwf_complex * pOutBuffer, float minFreq, float maxFreq);

    int getFreqStride() const;

    IntPoint m_Size;
    std::vector<float> m_Frequencies;
    std::vector<BitmapPtr> m_pBPBmps;

    fftwf_plan m_fftPlan;
    fftwf_plan m_ifftPlan;
    BitmapPtr m_FreqImage;
    float* m_pInData;
    fftwf_complex * m_pFreqData;
    fftwf_complex * m_pBPFreqData;
    float* m_pBPData;     // Bandpass data
};

}
#endif
