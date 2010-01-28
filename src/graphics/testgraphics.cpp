//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2008 Ulrich von Zadow
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

#include "GraphicsTest.h"
#include "Bitmap.h"
#include "Pixel32.h"
#include "Pixel24.h"
#include "Pixel16.h"
#include "Filtercolorize.h"
#include "Filtergrayscale.h"
#include "Filterfill.h"
#include "Filterflip.h"
#include "Filterfliprgb.h"
#include "Filterflipuv.h"
#include "Filter3x3.h"
#include "FilterConvol.h"
#include "HistoryPreProcessor.h"
#include "FilterHighpass.h"
#include "FilterFastBandpass.h"
#include "FilterGauss.h"
#include "FilterBlur.h"
#include "FilterBandpass.h"
#include "FilterFastDownscale.h"
#include "FilterMask.h"
#include "FilterFloodfill.h"
#include "FilterDilation.h"
#include "FilterErosion.h"
#include "FilterGetAlpha.h"
#include "FilterResizeBilinear.h"

#include "../base/TestSuite.h"
#include "../base/Exception.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable: 4251)
#endif
#include <Magick++.h>
#ifdef _WIN32
#pragma warning(pop)
#endif

#include <cstring>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

using namespace avg;
using namespace std;

BitmapPtr initBmp(PixelFormat PF)
{
    int height;
    if (PF == YCbCr422) {
        height = 10;
    } else {
        height = 7;

    }
    BitmapPtr pBmp(new Bitmap(IntPoint(4,height), PF));
    for(int y=0; y<height; ++y) {
        for (int x=0; x<4; ++x) {
            unsigned char * pPixel = 
                pBmp->getPixels()+y*pBmp->getStride()+x*pBmp->getBytesPerPixel();
            *(pPixel) = x;
            if (pBmp->getBytesPerPixel() > 1) {
                *(pPixel+1) = 0;
            }
            if (pBmp->getBytesPerPixel() > 2) {
                *(pPixel+2) = x;
                *(pPixel) = 16*y;
            }
            if (pBmp->getBytesPerPixel() > 3) {
                *(pPixel+3) = 0x80;
            }
        }
    }
    return pBmp;
}

class BitmapTest: public GraphicsTest {
public:
    BitmapTest()
        : GraphicsTest("BitmapTest", 2)
    {
    }

    void runTests() 
    {
        runPFTests(B8G8R8A8);
        runPFTests(R8G8B8A8);
        runPFTests(R8G8B8X8);
        runPFTests(R8G8B8);
        runPFTests(I8);
        runPFTests(I16);
        runPFTests(YCbCr422);
        runLineTest(B8G8R8A8, Pixel32(0,0,255,255));
        runLineTest(B8G8R8, Pixel24(0,0,255));
        runLineTest(I8, Pixel8(255));

        cerr << "    Testing OwnsBits." << endl;
        unsigned char pData[4*7*3];
        for (int i=0; i<4*7*3; ++i) {
            pData[i] = i;
        }
        Bitmap Bmp1 = Bitmap(IntPoint(4,7), R8G8B8, pData, 12, true, "");
        Bitmap Bmp2 = Bitmap(IntPoint(4,7), R8G8B8, pData, 12, false, "");
        testEqual(Bmp1, Bmp2, "BmpOwnsBits");
        {
            cerr << "    Testing copyPixels - R8G8B8X8->R8G8B8->R8G8B8X8." << endl;
            BitmapPtr pBmp = initBmp(R8G8B8X8);
            Bitmap Bmp1(IntPoint(4,7), R8G8B8);
            Bmp1.copyPixels(*pBmp);
            Bitmap Bmp2(IntPoint(4,7), R8G8B8X8);
            Bmp2.copyPixels(Bmp1);
            for(int y=0; y<7; ++y) {
                for (int x=0; x<4; ++x) {
                    *(Bmp2.getPixels()+y*Bmp2.getStride()+x
                            *Bmp2.getBytesPerPixel()+3) = 0x80;
                }
            }
            testEqual(Bmp2, *pBmp, "BmpCopyPixels_1");
        }
        {
            cerr << "    Testing copyPixels - R8G8B8A8->R8G8B8->R8G8B8A8." << endl;
            BitmapPtr pBmp = initBmp(R8G8B8A8);
            BitmapPtr pBaselineBmp = initBmp(R8G8B8A8);
            BitmapPtr pCopyBmp = BitmapPtr(new Bitmap(IntPoint(4,7), R8G8B8));
            pCopyBmp->copyPixels(*pBmp);
            pBmp->copyPixels(*pCopyBmp);
            for(int y=0; y<7; ++y) {
                for (int x=0; x<4; ++x) {
                    *(pBmp->getPixels()+y*pBmp->getStride()+x
                            *pBmp->getBytesPerPixel()+3) = 0x80;
                }
            }
            testEqual(*pBmp, *pBaselineBmp, "BmpCopyPixels_2");
        }
        {
            cerr << "    Testing copyPixels - I8->I16->I8." << endl;
            BitmapPtr pBmp = initBmp(I8);
            BitmapPtr pBaselineBmp = initBmp(I8);
            BitmapPtr pCopyBmp = BitmapPtr(new Bitmap(IntPoint(4,7), I16));
            pCopyBmp->copyPixels(*pBmp);
            pBmp->copyPixels(*pCopyBmp);
            testEqual(*pBmp, *pBaselineBmp, "BmpCopyPixels_3");
        }
        {
            cerr << "    Testing copyPixels - R8G8B8A8->R32G32B32A32F->R8G8B8A8." << endl;
            BitmapPtr pBmp = initBmp(R8G8B8A8);
            BitmapPtr pBaselineBmp = initBmp(R8G8B8A8);
            BitmapPtr pCopyBmp = BitmapPtr(new Bitmap(IntPoint(4,7), R32G32B32A32F));
            pCopyBmp->copyPixels(*pBmp);
            pBmp->copyPixels(*pCopyBmp);
            testEqual(*pBmp, *pBaselineBmp, "BmpCopyPixels_3");
        }
        {
            cerr << "    Testing statistics." << endl;
            cerr << "      I8" << endl;
            testStatistics(I8, Pixel8(0), Pixel8(0), Pixel8(2), Pixel8(2));
            cerr << "      R8G8B8A8" << endl;
            testStatistics(R8G8B8A8, Pixel32(0,0,0,0), Pixel32(0,0,0,0), 
                    Pixel32(2,2,2,2), Pixel32(2,2,2,2));
            cerr << "      R8G8B8X8" << endl;
            testStatistics(R8G8B8X8, Pixel32(0,0,0,255), Pixel32(0,0,0,255), 
                    Pixel32(2,2,2,255), Pixel32(2,2,2,255));
            cerr << "      R8G8B8" << endl;
            testStatistics(R8G8B8, Pixel24(0,0,0), Pixel24(0,0,0), 
                    Pixel24(2,2,2), Pixel24(2,2,2));

        }
        {
            cerr << "    Testing YUV->RGB conversion." << endl;
            testYUV2RGB();
        }
        runSaveTest(B8G8R8A8);
        runSaveTest(B8G8R8X8);
    }
    
private:
    template<class Pixel>
    void testStatistics(PixelFormat pf, const Pixel& p00, const Pixel& p01,
            const Pixel& p10, const Pixel& p11)
    {
        BitmapPtr pBmp = BitmapPtr(new Bitmap(IntPoint(2,2), pf));
        pBmp->setPixel(IntPoint(0,0), p00);
        pBmp->setPixel(IntPoint(0,1), p01);
        pBmp->setPixel(IntPoint(1,0), p10);
        pBmp->setPixel(IntPoint(1,1), p11);
        TEST(pBmp->getAvg() == 1);
        TEST(pBmp->getStdDev() == 1);
    }

    void runPFTests(PixelFormat PF)
    {
        cerr << "    Testing " << Bitmap::getPixelFormatString(PF) << endl;
        BitmapPtr pBmp = initBmp(PF);
        {
            cerr << "      Testing size." <<endl;
            if (PF == YCbCr422) {
                TEST(pBmp->getSize() == IntPoint(4,10));
            } else {
                TEST(pBmp->getSize() == IntPoint(4,7));
            }
        }
        {
            cerr << "      Testing copy constructor." << endl;
            Bitmap BmpCopy1(*pBmp);
            testEqual(BmpCopy1, *pBmp, "BmpCopyConstructor");
        }
        {
            cerr << "      Testing assignment operator." << endl;
            Bitmap BmpCopy2 = *pBmp;
            testEqual(BmpCopy2, *pBmp, "BmpAssignment");
        }
        {
            cerr << "      Testing sub-bitmap constructor." << endl;
            Bitmap BmpCopy3 (*pBmp, IntRect(IntPoint(0,0), pBmp->getSize()));
            testEqual(BmpCopy3, *pBmp, "BmpSubBmpCtor");
        }
        if (PF == I8) {
            cerr << "      Testing getHistogram." << endl;
            HistogramPtr pHist = pBmp->getHistogram();
            TEST((*pHist)[0] == 7);
            TEST((*pHist)[1] == 7);
            TEST((*pHist)[2] == 7);
            TEST((*pHist)[3] == 7);
            bool bOk = true;
            for (int i=4; i<256; ++i) {
                if (bOk) {
                    bOk = ((*pHist)[i] == 0);
                }
            }
            TEST(bOk);   
        }
    }

    void runSaveTest(PixelFormat PF)
    {
        cerr << "    Testing save for " << Bitmap::getPixelFormatString(PF) << endl;
        BitmapPtr pBmp = initBmp(PF);
        pBmp->save("test.tif");
        Bitmap LoadedBmp("test.tif");
        ::remove("test.tif");
        testEqual(LoadedBmp, *pBmp, "BmpSave");
    }

    template<class Pixel>
    void runLineTest(PixelFormat PF, Pixel Color)
    {
        cerr << "    Testing line drawing for " << Bitmap::getPixelFormatString(PF) << endl;
        Bitmap Bmp(IntPoint(15, 15), PF);
        memset(Bmp.getPixels(), 0, Bmp.getStride()*15);
        Bmp.drawLine(IntPoint(7,7), IntPoint( 0, 2), Color);
        Bmp.drawLine(IntPoint(7,7), IntPoint( 0,12), Color);
        Bmp.drawLine(IntPoint(7,7), IntPoint( 2, 0), Color);
        Bmp.drawLine(IntPoint(7,7), IntPoint( 2,14), Color);
        Bmp.drawLine(IntPoint(7,7), IntPoint(12, 0), Color);
        Bmp.drawLine(IntPoint(7,7), IntPoint(12,14), Color);
        Bmp.drawLine(IntPoint(7,7), IntPoint(14, 2), Color);
        Bmp.drawLine(IntPoint(7,7), IntPoint(14,12), Color);
        string sFName = getSrcDirName()+"baseline/LineResult"+Bitmap::getPixelFormatString(PF)+".png";
        Bitmap BaselineBmp(sFName);
        Bitmap BaselineBmp2(IntPoint(15,15), PF);
        BaselineBmp2.copyPixels(BaselineBmp);
        testEqual(Bmp, BaselineBmp2, "BmpLineDraw");
    }

    void testYUV2RGB()
    {
        BitmapPtr pYBmp = BitmapPtr(new Bitmap(IntPoint(16, 16), I8));
        for (int x=0; x<16; ++x) {
            FilterFillRect<Pixel8>(IntRect(x, 0, x+1, 16), 255-x*16).applyInPlace(pYBmp);
        }
        BitmapPtr pUBmp = BitmapPtr(new Bitmap(IntPoint(8, 8), I8));
        BitmapPtr pVBmp = BitmapPtr(new Bitmap(IntPoint(8, 8), I8));
        FilterFillRect<Pixel8>(IntRect(0, 0, 8, 4), 128).applyInPlace(pUBmp);
        FilterFillRect<Pixel8>(IntRect(0, 0, 8, 4), 128).applyInPlace(pVBmp);
        for (int y=4; y<8; ++y) {
            FilterFillRect<Pixel8>(IntRect(0, y, 8, y+1), y*64).applyInPlace(pUBmp);
            FilterFillRect<Pixel8>(IntRect(0, y, 8, y+1), 255-y*64).applyInPlace(pVBmp);
        }
        BitmapPtr pRGBBmp = BitmapPtr(new Bitmap(IntPoint(16, 16), B8G8R8X8));
        FilterFill<Pixel32>(Pixel32(255,0,0,255)).applyInPlace(pRGBBmp);
        pRGBBmp->copyYUVPixels(*pYBmp, *pUBmp, *pVBmp);
        testEqual(*pRGBBmp, "YUV2RGBResult1", B8G8R8X8, 0.5, 0.5);
    }

};

class FilterColorizeTest: public GraphicsTest {
public:
    FilterColorizeTest()
        : GraphicsTest("FilterColorizeTest", 2)
    {
    }

    void runTests() 
    {
        runPFTests(R8G8B8A8);
        runPFTests(R8G8B8);
    }

private:    
    void runPFTests(PixelFormat PF) {
        BitmapPtr pBmp = initBmp(PF);
        FilterColorize(15, 0).applyInPlace(pBmp);
        FilterColorize(100, 50).applyInPlace(pBmp);
        FilterColorize(50, 100).applyInPlace(pBmp);
//        pBmp->dump();
    }
};

class FilterGrayscaleTest: public GraphicsTest {
public:
    FilterGrayscaleTest()
        : GraphicsTest("FilterGrayscaleTest", 2)
    {
    }

    void runTests() 
    {
        runPFTests(R8G8B8A8);
        runPFTests(R8G8B8);
        runPFTests(I8);
    }

private:
    void runPFTests(PixelFormat PF) {
        BitmapPtr pBmp = initBmp(PF);
        FilterGrayscale().applyInPlace(pBmp);
//        pBmp->dump();
    }
};

class FilterFillTest: public GraphicsTest {
public:
    FilterFillTest()
        : GraphicsTest("FilterFillTest", 2)
    {
    }

    void runTests() 
    {
        runPFTests<Pixel32>(R8G8B8A8, Pixel32(255,80,0,255));
        runPFTests<Pixel24>(R8G8B8, Pixel24(255,80,0));
    }

private:
    template<class PixelC>
    void runPFTests(PixelFormat PF, PixelC Color) {
        BitmapPtr pBmp = initBmp(PF);
        FilterFill<PixelC>(Color).applyInPlace(pBmp);
//        pBmp->dump();
    }
};

class FilterFlipTest: public GraphicsTest {
public:
    FilterFlipTest()
        : GraphicsTest("FilterFlipTest", 2)
    {
    }

    void runTests() 
    {
        runPFTests(R8G8B8A8);
        runPFTests(B8G8R8X8);
        runPFTests(R8G8B8);
        runPFTests(I8);
    }

private:
    void runPFTests(PixelFormat PF) {
        BitmapPtr pBmp = initBmp(PF);
        {
            BitmapPtr pBmp1 = FilterFlip().apply(pBmp);
            BitmapPtr pBmp2 = FilterFlip().apply(pBmp1);
            TEST(*pBmp == *pBmp2);
        }
        pBmp = initBmp(PF);
        { 
            Bitmap BmpBaseline = *pBmp;
            FilterFlip().applyInPlace(pBmp);
            FilterFlip().applyInPlace(pBmp);
            TEST(*pBmp == BmpBaseline);
        }
//        pBmp->dump();
    }
};

class FilterFlipRGBTest: public GraphicsTest {
public:
    FilterFlipRGBTest()
        : GraphicsTest("FilterFlipRGBTest", 2)
    {
    }

    void runTests() 
    {
        runPFTests(B8G8R8A8);
        runPFTests(R8G8B8A8);
        runPFTests(R8G8B8);
    }

private:
    void runPFTests(PixelFormat PF) {
        BitmapPtr pBmp = initBmp(PF);
        {
            BitmapPtr pBmp1 = FilterFlipRGB().apply(pBmp);
            BitmapPtr pBmp2 = FilterFlipRGB().apply(pBmp1);
            TEST(*pBmp == *pBmp2);
        }
        { 
            Bitmap BmpBaseline = *pBmp;
            FilterFlipRGB().applyInPlace(pBmp);
            FilterFlipRGB().applyInPlace(pBmp);
            TEST(*pBmp == BmpBaseline);
        }
//        pBmp->dump();
    }
};

class FilterFlipUVTest: public GraphicsTest {
public:
    FilterFlipUVTest()
        : GraphicsTest("FilterFlipUVTest", 2)
    {
    }

    void runTests() 
    {
        BitmapPtr pBmp = initBmp(YCbCr422);
        {
            BitmapPtr pBmp1 = FilterFlipUV().apply(pBmp);
            BitmapPtr pBmp2 = FilterFlipUV().apply(pBmp1);
            TEST(*pBmp == *pBmp2);
        }
        { 
            Bitmap BmpBaseline = *pBmp;
            FilterFlipUV().applyInPlace(pBmp);
            FilterFlipUV().applyInPlace(pBmp);
            TEST(*pBmp == BmpBaseline);
        }
//        pBmp->dump();
    }
};

class FilterComboTest: public GraphicsTest {
public:
    FilterComboTest()
        : GraphicsTest("FilterComboTest", 2)
    {
    }

    void runTests() 
    {
        try {
            char * pSrcDir = getenv("srcdir");
            string sFilename;
            if (pSrcDir) {
                sFilename = (string)pSrcDir+"/";
            }
            sFilename += "../test/rgb24-64x64.png";
            Bitmap TempBmp(sFilename);
            PixelFormat pf = R8G8B8;    
            BitmapPtr pBmp;
            pBmp = createBmp(TempBmp.getSize(), pf);
            pBmp->copyPixels(TempBmp);
            FilterColorize(15, 50).applyInPlace(pBmp);
            FilterFlipRGB().applyInPlace(pBmp);
        } catch (Magick::Exception & ex) {
            cerr << ex.what() << endl;
            setFailed();
        }
    }

private:
    BitmapPtr createBmp(const IntPoint& Size, PixelFormat pf)
    {
        BitmapPtr pBmp;
        pBmp = BitmapPtr(new Bitmap(Size, pf));
        return pBmp;
    }
};

class FilterConvolTest: public GraphicsTest {
public:
    FilterConvolTest()
        : GraphicsTest("FilterConvolTest", 2)
    {
    }

    void runTests() 
    {
        runPFTests<Pixel24>(R8G8B8);
        runPFTests<Pixel32>(R8G8B8X8);
        //FIXME runPFTests<Pixel8>(I8);
    }

private:
    template<class PixelT>
    void runPFTests(PixelFormat PF)
    {
        BitmapPtr pBmp(new Bitmap(IntPoint(4, 4), PF));
        initBmp<PixelT>(pBmp);
        double Mat[9] = 
                {1,0,2,
                 0,1,0,
                 3,0,4};
        BitmapPtr pNewBmp = FilterConvol<PixelT>(&(Mat[0]),3,3).apply(pBmp);
        TEST(pNewBmp->getSize() == IntPoint(2,2));
        unsigned char * pLine0 = pNewBmp->getPixels();
        TEST(*(PixelT*)pLine0 == PixelT(1,0,0));
        TEST(*(((PixelT*)pLine0)+1) == PixelT(4,0,0));
        unsigned char * pLine1 = pNewBmp->getPixels()+pNewBmp->getStride();
        TEST(*(PixelT*)(pLine1) == PixelT(0,0,9));
        TEST(*((PixelT*)(pLine1)+1) == PixelT(0,0,16));
        
    }
    
    template<class PixelT>
    void initBmp(BitmapPtr pBmp) 
    {
        PixelT * pPixels = (PixelT *)(pBmp->getPixels());
        PixelT Color = PixelT(0,0,0);
        FilterFill<PixelT>(Color).applyInPlace(pBmp);
        pPixels[0] = PixelT(1,0,0);
        pPixels[3] = PixelT(2,0,0);
        pPixels = (PixelT*)((char *)pPixels+3*pBmp->getStride());
        pPixels[0] = PixelT(0,0,3);
        pPixels[3] = PixelT(0,0,4);
    }
};

class Filter3x3Test: public GraphicsTest {
public:
    Filter3x3Test()
        : GraphicsTest("Filter3x3Test", 2)
    {
    }

    void runTests() 
    {
        runPFTests<Pixel24>(R8G8B8);
        runPFTests<Pixel32>(R8G8B8X8);
    }

private:
    template<class PixelT>
    void runPFTests(PixelFormat PF)
    {
        BitmapPtr pBmp(new Bitmap(IntPoint(4, 4), PF));
        initBmp<PixelT>(pBmp);
        double Mat[3][3] = 
                {{1,0,2},
                 {0,1,0},
                 {3,0,4}};
        BitmapPtr pNewBmp = Filter3x3(Mat).apply(pBmp);
        TEST(pNewBmp->getSize() == IntPoint(2,2));
        unsigned char * pLine0 = pNewBmp->getPixels();
        TEST(*(PixelT*)pLine0 == PixelT(1,0,0));
        TEST(*(((PixelT*)pLine0)+1) == PixelT(4,0,0));
        unsigned char * pLine1 = pNewBmp->getPixels()+pNewBmp->getStride();
        TEST(*(PixelT*)(pLine1) == PixelT(0,0,9));
        TEST(*((PixelT*)(pLine1)+1) == PixelT(0,0,16));
        
    }
    
    template<class PixelT>
    void initBmp(BitmapPtr pBmp) 
    {
        PixelT * pPixels = (PixelT *)(pBmp->getPixels());
        PixelT Color = PixelT(0,0,0);
        FilterFill<PixelT>(Color).applyInPlace(pBmp);
        pPixels[0] = PixelT(1,0,0);
        pPixels[3] = PixelT(2,0,0);
        pPixels = (PixelT*)((char *)pPixels+3*pBmp->getStride());
        pPixels[0] = PixelT(0,0,3);
        pPixels[3] = PixelT(0,0,4);
    }
};
    
class HistoryPreProcessorTest: public GraphicsTest {
public:
    HistoryPreProcessorTest()
        : GraphicsTest("HistoryPreProcessor", 2)
    {
    }

    void runTests() 
    {
        BitmapPtr pBaseBmp = initBmp(I8);
        BitmapPtr pBmp = BitmapPtr(new Bitmap(*pBaseBmp));
        BitmapPtr nullBmp = FilterFill<Pixel8>(0).apply(pBmp);
        pBmp->copyPixels(*pBaseBmp);
        HistoryPreProcessor filt(pBaseBmp->getSize(), 1, true);
        pBmp = filt.apply(pBaseBmp);
        testEqual(*pBmp, *nullBmp, "HistoryPreprocessor1");
        for(int i=0;i<1;i++){
            pBmp = filt.apply(pBaseBmp);
            testEqual(*pBmp, *nullBmp, "HistoryPreprocessor2");
        }
    }

};
    
class FilterFastBandpassTest: public GraphicsTest {
public:
    FilterFastBandpassTest()
        : GraphicsTest("FilterFastBandpassTest", 2)
    {
    }

    void runTests()
    {
        BitmapPtr pBmp = BitmapPtr(new Bitmap(IntPoint(16,16), I8));
        FilterFill<Pixel8>(0).applyInPlace(pBmp);
        *(pBmp->getPixels()+pBmp->getStride()*7+7) = 255;
        BitmapPtr pDestBmp = FilterFastBandpass().apply(pBmp);
        testEqual(*pDestBmp, "FastBandpassResult", I8);
    }
};


class FilterHighpassTest: public GraphicsTest {
public:
    FilterHighpassTest()
        : GraphicsTest("FilterHighpassTest", 2)
    {
    }

    void runTests()
    {
        BitmapPtr pBmp = BitmapPtr(new Bitmap(IntPoint(16,16), I8));
        FilterFill<Pixel8>(0).applyInPlace(pBmp);
        *(pBmp->getPixels()+pBmp->getStride()*7+7) = 255;
        BitmapPtr pDestBmp = FilterHighpass().apply(pBmp);
        testEqual(*pDestBmp, "HighpassResult", I8);
    }
};


class FilterGaussTest: public GraphicsTest {
public:
    FilterGaussTest()
        : GraphicsTest("FilterGaussTest", 2)
    {
    }

    void runTests()
    {
        BitmapPtr pBmp = BitmapPtr(new Bitmap(IntPoint(16,16), I8));
        FilterFill<Pixel8>(0).applyInPlace(pBmp);
        *(pBmp->getPixels()+pBmp->getStride()*7+7) = 255;
//        FilterGauss(1).dumpKernel();
//        FilterGauss(3).dumpKernel();
//        FilterGauss(2.1).dumpKernel();
//        FilterGauss(1.9).dumpKernel();
//        FilterGauss(4).dumpKernel();
//        FilterGauss(5).dumpKernel();
        BitmapPtr pDestBmp = FilterGauss(3).apply(pBmp);
        testEqual(*pDestBmp, "Gauss3Result", I8);
        pDestBmp = FilterGauss(1).apply(pBmp);
        testEqual(*pDestBmp, "Gauss1Result", I8);
        pDestBmp = FilterGauss(1.5).apply(pBmp);
        testEqual(*pDestBmp, "Gauss15Result", I8);
        pDestBmp = FilterGauss(5).apply(pBmp);
        testEqual(*pDestBmp, "Gauss5Result", I8);
    }
};


class FilterBlurTest: public GraphicsTest {
public:
    FilterBlurTest()
        : GraphicsTest("FilterBlurTest", 2)
    {
    }

    void runTests()
    {
        BitmapPtr pBmp = BitmapPtr(new Bitmap(IntPoint(16,16), I8));
        FilterFill<Pixel8>(0).applyInPlace(pBmp);
        *(pBmp->getPixels()+pBmp->getStride()*7+7) = 255;
        BitmapPtr pDestBmp = FilterBlur().apply(pBmp);
        testEqual(*pDestBmp, "BlurResult", I8);
    }
};


class FilterBandpassTest: public GraphicsTest {
public:
    FilterBandpassTest()
        : GraphicsTest("FilterBandpassTest", 2)
    {
    }

    void runTests()
    {
        BitmapPtr pBmp = BitmapPtr(new Bitmap(IntPoint(16,16), I8));
        FilterFill<Pixel8>(0).applyInPlace(pBmp);
        *(pBmp->getPixels()+pBmp->getStride()*7+7) = 255;
        
        BitmapPtr pDestBmp = FilterBandpass(1.9,3).apply(pBmp);
        testEqual(*pDestBmp, "BandpassResult", I8);
    }
};

class FilterFastDownscaleTest: public GraphicsTest {
public:
    FilterFastDownscaleTest()
        : GraphicsTest("FilterFastDownscaleTest", 2)
    {
    }

    void runTests()
    {
        BitmapPtr pBmp = BitmapPtr(new Bitmap(IntPoint(4,4), I8));
        FilterFill<Pixel8>(0).applyInPlace(pBmp);
        *(pBmp->getPixels()+pBmp->getStride()*3+3) = 252;

        BitmapPtr pDestBmp = FilterFastDownscale(2).apply(pBmp);
        testEqual(*pDestBmp, "FastDownscaleResult", I8);
    }
};

class FilterMaskTest: public GraphicsTest {
public:
    FilterMaskTest()
        : GraphicsTest("FilterMaskTest", 2)
    {
    }

    void runTests()
    {
        runTestsWithBmp(initBmp(I8), "I8");
        runTestsWithBmp(initBmp(B8G8R8), "B8G8R8");
        runTestsWithBmp(initBmp(B8G8R8X8), "B8G8R8X8");
    }

private:
    void runTestsWithBmp(BitmapPtr pBmp, const string& sName)
    {
        BitmapPtr pMaskBmp = BitmapPtr(new Bitmap(pBmp->getSize(), I8));
        FilterFill<Pixel8>(0).applyInPlace(pMaskBmp);
        for (int y=0; y<pBmp->getSize().y; y++) {
            pMaskBmp->setPixel(IntPoint(1, y), Pixel8(128));
            pMaskBmp->setPixel(IntPoint(2, y), Pixel8(255));
            pMaskBmp->setPixel(IntPoint(3, y), Pixel8(255));
        }

        BitmapPtr pDestBmp = FilterMask(pMaskBmp).apply(pBmp);
        string sFName = string("baseline/MaskResult")+sName+".png";
//        pDestBmp->save(sFName);
        sFName = getSrcDirName()+sFName;
        BitmapPtr pRGBXBaselineBmp = BitmapPtr(new Bitmap(sFName));
        BitmapPtr pBaselineBmp = BitmapPtr(
                new Bitmap(pRGBXBaselineBmp->getSize(), pBmp->getPixelFormat()));
        pBaselineBmp->copyPixels(*pRGBXBaselineBmp);
        TEST(*pDestBmp == *pBaselineBmp);
    }
};

class FilterFloodfillTest: public GraphicsTest {
public:
    FilterFloodfillTest()
        : GraphicsTest("FilterFloodfillTest", 2)
    {
    }

    void runTests()
    {
        BitmapPtr pBmp = loadTestBmp("floodfill");
        BitmapPtr pDestBmp = FilterFloodfill<ColorTester>(
                ColorTester(Pixel32(255,255,255,255)), IntPoint(4,3)).apply(pBmp);
        testEqual(*pDestBmp, "FloodfillResult", B8G8R8A8, 0, 0);
    }

};

class FilterDilationTest: public GraphicsTest {
public:
    FilterDilationTest()
        : GraphicsTest("FilterDilationTest", 2)
    {
    }

    void runTests()
    {
        BitmapPtr pBmp = loadTestBmp("dilation", I8);
        BitmapPtr pDestBmp = FilterDilation().apply(pBmp);
        testEqual(*pDestBmp, "DilationResult", I8, 0, 0);
    }

};

class FilterErosionTest: public GraphicsTest {
public:
    FilterErosionTest()
        : GraphicsTest("FilterErosionTest", 2)
    {
    }

    void runTests()
    {
        BitmapPtr pBmp = loadTestBmp("erosion", I8);
        BitmapPtr pDestBmp = FilterErosion().apply(pBmp);
        testEqual(*pDestBmp, "ErosionResult", I8, 0, 0);
    }

};

class FilterAlphaTest: public GraphicsTest {
public:
    FilterAlphaTest()
        : GraphicsTest("FilterAlphaTest", 2)
    {
    }

    void runTests()
    {
        BitmapPtr pBmp = loadTestBmp("rgb24alpha-64x64", R8G8B8A8);
        BitmapPtr pAlphaBmp = FilterGetAlpha().apply(pBmp);
        testEqual(*pAlphaBmp, "GetAlphaResult", I8, 0, 0);
        BitmapPtr pDestBmp(new Bitmap(*pBmp));
        pDestBmp->setAlpha(*pAlphaBmp);
        testEqual(*pDestBmp, *pBmp, "SetAlphaResult", 0, 0);
    }

};

class FilterResizeBilinearTest: public GraphicsTest {
public:
    FilterResizeBilinearTest()
        : GraphicsTest("FilterResizeBilinearTest", 2)
    {
    }

    void runTests()
    {
        BitmapPtr pBmp = loadTestBmp("rgb24alpha-64x64", B8G8R8A8);
        runTestWithBitmap(pBmp);
        pBmp = loadTestBmp("rgb24-64x64", B8G8R8X8);
        runTestWithBitmap(pBmp);
    }

private:
    void runTestWithBitmap(BitmapPtr pBmp)
    {
        BitmapPtr pDestBmp = FilterResizeBilinear(IntPoint(32,32)).apply(pBmp);
        string sName = string("ResizeBilinearResult")+pBmp->getPixelFormatString();
        testEqual(*pDestBmp, sName, pBmp->getPixelFormat());
    }

};

class GraphicsTestSuite: public TestSuite {
public:
    GraphicsTestSuite() 
        : TestSuite("GraphicsTestSuite")
    {
        addTest(TestPtr(new BitmapTest));
        addTest(TestPtr(new Filter3x3Test));
        addTest(TestPtr(new FilterConvolTest));
        addTest(TestPtr(new FilterColorizeTest));
        addTest(TestPtr(new FilterGrayscaleTest));
        addTest(TestPtr(new FilterFillTest));
        addTest(TestPtr(new FilterFlipTest));
        addTest(TestPtr(new FilterFlipRGBTest));
        addTest(TestPtr(new FilterFlipUVTest));
        addTest(TestPtr(new FilterComboTest));
        addTest(TestPtr(new HistoryPreProcessorTest));
        addTest(TestPtr(new FilterHighpassTest));
        addTest(TestPtr(new FilterGaussTest));
        addTest(TestPtr(new FilterBlurTest));
        addTest(TestPtr(new FilterBandpassTest));
        addTest(TestPtr(new FilterFastBandpassTest));
        addTest(TestPtr(new FilterFastDownscaleTest));
        addTest(TestPtr(new FilterMaskTest));
        addTest(TestPtr(new FilterFloodfillTest));
        addTest(TestPtr(new FilterDilationTest));
        addTest(TestPtr(new FilterErosionTest));
        addTest(TestPtr(new FilterAlphaTest));
        addTest(TestPtr(new FilterResizeBilinearTest));
    }
};


int main(int nargs, char** args)
{
    GraphicsTest::createResultImgDir();
    GraphicsTestSuite Suite;
    Suite.runTests();
    bool bOK = Suite.isOk();

    if (bOK) {
        return 0;
    } else {
        return 1;
    }
}

