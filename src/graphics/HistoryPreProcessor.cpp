#include "HistoryPreProcessor.h"
#include "Bitmap.h"
#include "Filterfill.h"

#include <iostream>
#include <sstream>

using namespace std;

#define FAST_HISTORY_SPEED 16

namespace avg {
    
HistoryPreProcessor::HistoryPreProcessor(IntPoint dimensions, unsigned int UpdateInterval)
    : m_FrameCounter(0),
      m_UpdateInterval(UpdateInterval)
{
    m_pHistoryBmp = BitmapPtr(new Bitmap(dimensions, I16));
    reset();
}

HistoryPreProcessor::~HistoryPreProcessor()
{
}
void HistoryPreProcessor::setInterval(unsigned int UpdateInterval)
{
    m_FrameCounter = 0;
    m_UpdateInterval = UpdateInterval;
}

unsigned int HistoryPreProcessor::getInterval()
{
    return m_UpdateInterval;
}

void HistoryPreProcessor::reset()
{
    m_HistoryInitialized = 256/FAST_HISTORY_SPEED;
}

void HistoryPreProcessor::updateHistory(BitmapPtr new_img)
{
    assert(new_img->getSize() == m_pHistoryBmp->getSize());
    if (m_HistoryInitialized > 0) {
        m_pHistoryBmp->copyPixels(*new_img);
        m_HistoryInitialized *= -1;
    } else { 
        if (m_HistoryInitialized == 0 && (m_FrameCounter < m_UpdateInterval-1)){
            m_FrameCounter++;
            return;
        }
        m_FrameCounter = 0;
        const unsigned char * pSrc = new_img->getPixels();
        unsigned short * pDest = (unsigned short*)(m_pHistoryBmp->getPixels());
        int DestStride = m_pHistoryBmp->getStride()/m_pHistoryBmp->getBytesPerPixel();
        IntPoint Size = m_pHistoryBmp->getSize();
        if (m_HistoryInitialized == 0) {
            for (int y=0; y<Size.y; y++) {
                const unsigned char * pSrcPixel = pSrc;
                unsigned short * pDestPixel = pDest;
                for (int x=0; x<Size.x; x++) {
                    int t = 255*int(*pDestPixel);
                    *pDestPixel = (t)/256 + *pSrcPixel;
                    pDestPixel++;
                    pSrcPixel++;
                }
                pDest += DestStride;
                pSrc += new_img->getStride();
            }
        } else {
            for (int y=0; y<Size.y; y++) {
                const unsigned char * pSrcPixel = pSrc;
                unsigned short * pDestPixel = pDest;
                for (int x=0; x<Size.x; x++) {
                    int t = (FAST_HISTORY_SPEED-1)*int(*pDestPixel);
                    *pDestPixel = (t)/FAST_HISTORY_SPEED + int(*pSrcPixel)*
                            (256/FAST_HISTORY_SPEED);
                    pDestPixel++;
                    pSrcPixel++;
                }
                pDest += DestStride;
                pSrc += new_img->getStride();
            }
            m_HistoryInitialized++;
        }
    }
}

void HistoryPreProcessor::applyInPlace(BitmapPtr img)
{
    updateHistory(img);
    unsigned short * pSrc = (unsigned short*)m_pHistoryBmp->getPixels();
    int SrcStride = m_pHistoryBmp->getStride()/m_pHistoryBmp->getBytesPerPixel();
    int DestStride = img->getStride();
    unsigned char * pDest = img->getPixels();
    IntPoint Size = img->getSize();
    unsigned char Max = 0;
    for (int y=0; y<Size.y; y++) {
        const unsigned short * pSrcPixel = pSrc;
        unsigned char * pDestPixel = pDest;
        for (int x=0; x<Size.x; x++) {
            //*pDestPixel = (unsigned char)abs((int)*pDestPixel - (int)(*pSrcPixel)/256);
            unsigned char Src = *pSrcPixel/256;
            if ((*pDestPixel)>Src) {
                *pDestPixel = *pDestPixel-Src;
                if (Max < *pDestPixel) {
                    Max = *pDestPixel;
                }
            } else {
                *pDestPixel = 0;
            }
            pDestPixel++;
            pSrcPixel++;
        }
        pDest += DestStride;
        pSrc += SrcStride;
    }
//    normalizeHistogram(img, Max);
}

// Fast pseudo-normalization with an integer factor.
void HistoryPreProcessor::normalizeHistogram(BitmapPtr pBmp, unsigned char Max)
{
    if (Max < 128) {
        Max = 128;
    }
    int Factor = int(256.0/Max);
    unsigned char * pLine = pBmp->getPixels();
    IntPoint Size = pBmp->getSize();
    int Stride = pBmp->getStride();
    for (int y=0; y<Size.y; y++) {
        unsigned char * pPixel = pLine;
        for (int x=0; x<Size.x; x++) {
            *pPixel *= Factor;
            pPixel++;
        }
        pLine += Stride;
    }
}

}

