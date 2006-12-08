#include "HistoryPreProcessor.h"
#include "Bitmap.h"
#include "Filterfill.h"
#include <iostream>
namespace avg {
    
HistoryPreProcessor::HistoryPreProcessor(IntPoint dimensions, unsigned short N, unsigned int frame_skip)
    :m_N(N),
    m_FrameCounter(0),
    m_FrameSkip(frame_skip),
    m_bHistoryInitialized(false)
{
    m_pHistoryBmp = BitmapPtr(new Bitmap(dimensions, I8));
}

HistoryPreProcessor::~HistoryPreProcessor()
{
}
void HistoryPreProcessor::reconfigure(unsigned short N, unsigned int frame_skip, bool reset)
{
    m_FrameCounter = 0;
    m_N = N;
    m_FrameSkip = frame_skip;
    if(reset){
        //FilterFill<Pixel16> filt(0x0).applyInPlace(m_pHistoryBmp);
        //FIXME
    }
}

void HistoryPreProcessor::updateHistory(BitmapPtr new_img)
{
    assert(new_img->getSize() == m_pHistoryBmp->getSize());
    //else
    if (m_bHistoryInitialized) {
        if (m_FrameSkip && (m_FrameCounter <= m_FrameSkip)){
            m_FrameCounter++;
            return;
        }
        m_FrameCounter = 0;
        const unsigned char * pSrc = new_img->getPixels();
        unsigned char * pDest = (m_pHistoryBmp->getPixels());
        int DestStride = m_pHistoryBmp->getStride()/m_pHistoryBmp->getBytesPerPixel();
        IntPoint Size = m_pHistoryBmp->getSize();
        for (int y=0; y<Size.y; y++) {
            const unsigned char * pSrcPixel = pSrc;
            unsigned char * pDestPixel = pDest;
            for (int x=0; x<Size.x; x++) {
                int t = (m_N-1)*(*pDestPixel);
                *pDestPixel = (t)/m_N + *pSrcPixel;
                pDestPixel++;
                pSrcPixel++;
            }
            pDest += DestStride;
            pSrc += new_img->getStride();
        }
    } else {
        m_pHistoryBmp->copyPixels(*new_img);
        m_bHistoryInitialized = true;
    }
    m_pHistoryBmp->dump();
    for(int i=0;i<m_pHistoryBmp->getSize().x;++i){
        std::cerr<<(int)*(m_pHistoryBmp->getPixels()+i);
    }
    std::cerr<<std::endl;
}

void HistoryPreProcessor::applyInPlace(BitmapPtr img){
    updateHistory(img);
    unsigned char * pSrc = m_pHistoryBmp->getPixels();
    int SrcStride = m_pHistoryBmp->getStride()/m_pHistoryBmp->getBytesPerPixel();
    unsigned char * pDest = img->getPixels();
    IntPoint Size = img->getSize();
    for (int y=0; y<Size.y; y++) {
        const unsigned char * pSrcPixel = pSrc;
        unsigned char * pDestPixel = pDest;
        for (int x=0; x<Size.x; x++) {
            *pDestPixel = (unsigned char)abs((int)*pDestPixel - (int)*pSrcPixel);
            pDestPixel++;
            pSrcPixel++;
        }
        pDest += img->getStride();
        pSrc += SrcStride;
    }
}
}
