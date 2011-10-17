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
//

#include "FilterGauss.h"
#include "Filterfill.h"
#include "Pixel8.h"
#include "Bitmap.h"

#include "../base/MathHelper.h"
#include "../base/Exception.h"

#include <iostream>
#include <math.h>

using namespace std;

namespace avg {
    
FilterGauss::FilterGauss(double radius)
    : m_Radius(radius)
{
    calcKernel();
}

FilterGauss::~FilterGauss()
{
}

BitmapPtr FilterGauss::apply(BitmapPtr pBmpSrc)
{
    AVG_ASSERT(pBmpSrc->getPixelFormat() == I8);
    int intRadius = int(ceil(m_Radius));
    
    // Convolve in x-direction
    IntPoint tempSize(pBmpSrc->getSize().x-2*intRadius, pBmpSrc->getSize().y);
    BitmapPtr pTempBmp = BitmapPtr(new Bitmap(tempSize, I8, pBmpSrc->getName()));
    int srcStride = pBmpSrc->getStride();
    int tempStride = pTempBmp->getStride();
    unsigned char * pSrcLine = pBmpSrc->getPixels();
    unsigned char * pTempLine = pTempBmp->getPixels();
    for (int y = 0; y < tempSize.y; ++y) {
        unsigned char * pSrcPixel = pSrcLine+intRadius;
        unsigned char * pTempPixel = pTempLine;
        switch (intRadius) {
            case 3:
                for (int x = 0; x < tempSize.x; ++x) {
                    *pTempPixel = (*(pSrcPixel-3)*m_Kernel[0] + 
                            *(pSrcPixel-2)*m_Kernel[1] +
                            *(pSrcPixel-1)*m_Kernel[2] + 
                            *(pSrcPixel)*m_Kernel[3] +
                            *(pSrcPixel+1)*m_Kernel[4] + 
                            *(pSrcPixel+2)*m_Kernel[5] +
                            *(pSrcPixel+3)*m_Kernel[6])/256;
                    ++pSrcPixel;
                    ++pTempPixel;
                }
                break;
            case 2:
                for (int x = 0; x < tempSize.x; ++x) {
                    *pTempPixel = (*(pSrcPixel-2)*m_Kernel[0] +
                            *(pSrcPixel-1)*m_Kernel[1] + 
                            *(pSrcPixel)*m_Kernel[2] +
                            *(pSrcPixel+1)*m_Kernel[3] + 
                            *(pSrcPixel+2)*m_Kernel[4])/256;
                    ++pSrcPixel;
                    ++pTempPixel;
                }
                break;
            case 1:
                for (int x = 0; x < tempSize.x; ++x) {
                    *pTempPixel = (*(pSrcPixel-1)*m_Kernel[0] + 
                            *(pSrcPixel)*m_Kernel[1] +
                            *(pSrcPixel+1)*m_Kernel[2])/256;
                    ++pSrcPixel;
                    ++pTempPixel;
                }
                break;
            default:
                // This is _really_ slow!
                for (int x = 0; x < tempSize.x; ++x) {
                    *pTempPixel = 0;
                    unsigned char * pKernelPixel = pSrcPixel-intRadius;
                    for (int w=0; w <= intRadius*2; ++w) {
                        *pTempPixel += ((*pKernelPixel)*m_Kernel[w])/256;
                        pKernelPixel++;
                    }
                    ++pSrcPixel;
                    ++pTempPixel;
                }
        }
        pSrcLine += srcStride;
        pTempLine += tempStride;
    }

    // Convolve in y-direction
    IntPoint destSize(tempSize.x, tempSize.y-2*intRadius);
    BitmapPtr pDestBmp = BitmapPtr(new Bitmap(destSize, I8, pBmpSrc->getName()));
    int destStride = pDestBmp->getStride();
    pTempLine = pTempBmp->getPixels()+intRadius*tempStride;
    unsigned char * pDestLine = pDestBmp->getPixels();
    for (int y = 0; y < destSize.y; ++y) {
        unsigned char * pTempPixel = pTempLine;
        unsigned char * pDestPixel = pDestLine;
        switch (intRadius) {
            case 3:
                for (int x = 0; x < destSize.x; ++x) {
                    *pDestPixel = (*(pTempPixel-3*tempStride)*m_Kernel[0] +
                            *(pTempPixel-2*tempStride)*m_Kernel[1] +
                            *(pTempPixel-1*tempStride)*m_Kernel[2] + 
                            *(pTempPixel)*m_Kernel[3] +
                            *(pTempPixel+1*tempStride)*m_Kernel[4] + 
                            *(pTempPixel+2*tempStride)*m_Kernel[5] +
                            *(pTempPixel+3*tempStride)*m_Kernel[6])/256;
                    ++pTempPixel;
                    ++pDestPixel;
                }
                break;
            case 2:
                for (int x = 0; x < destSize.x; ++x) {
                    *pDestPixel = (*(pTempPixel-2*tempStride)*m_Kernel[0] +
                            *(pTempPixel-1*tempStride)*m_Kernel[1] + 
                            *(pTempPixel)*m_Kernel[2] +
                            *(pTempPixel+1*tempStride)*m_Kernel[3] + 
                            *(pTempPixel+2*tempStride)*m_Kernel[4])/256;
                    ++pTempPixel;
                    ++pDestPixel;
                }
                break;
            case 1:
                for (int x = 0; x < destSize.x; ++x) {
                    *pDestPixel = (*(pTempPixel-1*tempStride)*m_Kernel[0] + 
                            *(pTempPixel)*m_Kernel[1] +
                            *(pTempPixel+1*tempStride)*m_Kernel[2])/256;
                    ++pTempPixel;
                    ++pDestPixel;
                }
                break;
            default:
                // This is _really_ slow!
                for (int x = 0; x < tempSize.x; ++x) {
                    *pDestPixel = 0;
                    unsigned char * pKernelPixel = pTempPixel-intRadius*tempStride;
                    for (int w = 0; w <= intRadius*2; ++w) {
                        *pDestPixel += (*pKernelPixel*m_Kernel[w])/256;
                        pKernelPixel += tempStride;
                    }
                    ++pTempPixel;
                    ++pDestPixel;
                }
        }
        pTempLine += tempStride;
        pDestLine += destStride;
    }
    return pDestBmp;
}

void FilterGauss::dumpKernel()
{
    cerr << "Gauss, radius " << m_Radius << endl;
    cerr << "  Kernel width: " << m_KernelWidth << endl;
    for (int i = 0; i < m_KernelWidth; ++i) {
        cerr << "  " << m_Kernel[i] << endl;
    }
}

void FilterGauss::calcKernel()
{
    double FloatKernel[15];
    double Sum = 0;
    int intRadius = int(ceil(m_Radius));
    m_KernelWidth = intRadius*2+1;
    for (int i = 0; i <= intRadius; ++i) {
        FloatKernel[intRadius+i] = exp(-i*i/m_Radius-1)/sqrt(2*M_PI);
        FloatKernel[intRadius-i] = FloatKernel[intRadius+i];
        Sum += FloatKernel[intRadius+i];
        if (i != 0) {
            Sum += FloatKernel[intRadius-i];
        }
    }
    for (int i = 0; i < m_KernelWidth; ++i) {
        m_Kernel[i] = int(FloatKernel[i]*256/Sum+0.5);
    }
}

}
