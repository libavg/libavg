//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2006 Ulrich von Zadow
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

#include <iostream>
#include <stdio.h>
#include <stdlib.h>

using namespace avg;
using namespace std;

BitmapPtr initBmp(PixelFormat PF)
{
    BitmapPtr pBmp;
    if (PF == YCbCr422) {
        pBmp = BitmapPtr(new Bitmap(IntPoint(4,10), PF));
    } else {
        pBmp = BitmapPtr(new Bitmap(IntPoint(4,7), PF));
    }
    for(int y=0; y<7; ++y) {
        for (int x=0; x<4; ++x) {
            unsigned char * pPixel = 
                pBmp->getPixels()+y*pBmp->getStride()+x*pBmp->getBytesPerPixel();
            *(pPixel) = x;
            if (pBmp->getBytesPerPixel() > 1) {
                *(pPixel+1) = 0;
            }
            if (pBmp->getBytesPerPixel() > 2) {
                *(pPixel+2) = 16*y;
            }
            if (pBmp->getBytesPerPixel() > 3) {
                *(pPixel+3) = 0x80;
            }
        }
    }
    return pBmp;
}

class BitmapTest: public Test {
public:
    BitmapTest()
        : Test("BitmapTest", 2)
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
        runLineTest(R8G8B8A8, Pixel32(255,0,0,255));
        runLineTest(R8G8B8, Pixel24(255,0,0));
        runLineTest(I8, Pixel8(255));
        
        cerr << "    Testing OwnsBits." << endl;
        unsigned char pData[4*7*3];
        Bitmap Bmp1 = Bitmap(IntPoint(4,7), R8G8B8, pData, 12, true, "");
        Bitmap Bmp2 = Bitmap(IntPoint(4,7), R8G8B8, pData, 12, false, "");
        testEqual(Bmp1, Bmp2);
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
            testEqual(*pBmp, Bmp2);
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
            testEqual(*pBmp, *pBaselineBmp);
        }
        {
            cerr << "    Testing copyPixels - I8->I16->I8." << endl;
            BitmapPtr pBmp = initBmp(I8);
            BitmapPtr pBaselineBmp = initBmp(I8);
            BitmapPtr pCopyBmp = BitmapPtr(new Bitmap(IntPoint(4,7), I16));
            pCopyBmp->copyPixels(*pBmp);
            pBmp->copyPixels(*pCopyBmp);
            testEqual(*pBmp, *pBaselineBmp);

        }
        runSaveTest(R8G8B8A8);
        runSaveTest(R8G8B8X8);
    }
    
private:
    void runPFTests(PixelFormat PF) {
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
            testEqual(*pBmp, BmpCopy1);
        }
        {
            cerr << "      Testing assignment operator." << endl;
            Bitmap BmpCopy2 = *pBmp;
            testEqual(*pBmp, BmpCopy2);
        }
        {
            cerr << "      Testing sub-bitmap constructor." << endl;
            Bitmap BmpCopy3 (*pBmp, IntRect(IntPoint(0,0), pBmp->getSize()));
            testEqual(*pBmp, BmpCopy3);
        }
        if (PF == R8G8B8X8 || PF == R8G8B8) {
            cerr << "      Testing getNumDifferentPixels." << endl;
            Bitmap BmpCopy4 (*pBmp);
            TEST(pBmp->getNumDifferentPixels(BmpCopy4) == 0);
            
            unsigned char * pPixel = BmpCopy4.getPixels();
            *pPixel += 27;
            TEST(pBmp->getNumDifferentPixels(BmpCopy4) == 0);
            *pPixel = 255;
            *(pPixel+BmpCopy4.getStride()) = 255;
            TEST(!pBmp->getNumDifferentPixels(BmpCopy4) == 1);
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

    void runSaveTest(PixelFormat PF) {
        cerr << "    Testing save for " << Bitmap::getPixelFormatString(PF) << endl;
        BitmapPtr pBmp = initBmp(PF);
        pBmp->save("test.tif");
        Bitmap LoadedBmp("test.tif");
        ::remove("test.tif");
        testEqual(*pBmp, LoadedBmp);
    }

    template<class Pixel>
    void runLineTest(PixelFormat PF, Pixel Color) {
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
        string sFName = getSrcDir()+"testimages/LineResult"+Bitmap::getPixelFormatString(PF)+".png";
//        Bmp.save(sFName);
        Bitmap BaselineBmp(sFName);
        Bitmap BaselineBmp2(IntPoint(15,15), PF);
        BaselineBmp2.copyPixels(BaselineBmp);
        testEqual(Bmp, BaselineBmp2);
    }

    void testEqual(Bitmap& Bmp1, Bitmap& Bmp2) 
    {
        TEST(Bmp1 == Bmp2);
        if (!(Bmp1 == Bmp2)) {
            cerr << "Bmp1: " << endl;
            Bmp1.dump(true);
            cerr << "Bmp2: " << endl;
            Bmp2.dump(true);
        }
    }

};

class FilterColorizeTest: public Test {
public:
    FilterColorizeTest()
        : Test("FilterColorizeTest", 2)
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

class FilterGrayscaleTest: public Test {
public:
    FilterGrayscaleTest()
        : Test("FilterGrayscaleTest", 2)
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

class FilterFillTest: public Test {
public:
    FilterFillTest()
        : Test("FilterFillTest", 2)
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

class FilterFlipTest: public Test {
public:
    FilterFlipTest()
        : Test("FilterFlipTest", 2)
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

class FilterFlipRGBTest: public Test {
public:
    FilterFlipRGBTest()
        : Test("FilterFlipRGBTest", 2)
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

class FilterFlipUVTest: public Test {
public:
    FilterFlipUVTest()
        : Test("FilterFlipUVTest", 2)
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

class FilterComboTest: public Test {
public:
    FilterComboTest()
        : Test("FilterComboTest", 2)
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

class FilterConvolTest: public Test {
public:
    FilterConvolTest()
        : Test("FilterConvolTest", 2)
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

class Filter3x3Test: public Test {
public:
    Filter3x3Test()
        : Test("Filter3x3Test", 2)
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
    
class HistoryPreProcessorTest: public Test {
public:
    HistoryPreProcessorTest()
        : Test("HistoryPreProcessor", 2)
    {
    }
    void testEqual(Bitmap& Bmp1, Bitmap& Bmp2) 
    {
        TEST(Bmp1 == Bmp2);
        if (!(Bmp1 == Bmp2)) {
            cerr << "Bmp1: " << endl;
            Bmp1.dump(true);
            cerr << "Bmp2: " << endl;
            Bmp2.dump(true);
        }
    }

    void runTests() 
    {
        BitmapPtr pBaseBmp = initBmp(I8);
        BitmapPtr pBmp = BitmapPtr(new Bitmap(*pBaseBmp));
        BitmapPtr nullBmp = FilterFill<Pixel8>(0).apply(pBmp);
        pBmp->copyPixels(*pBaseBmp);
        HistoryPreProcessor filt(pBaseBmp->getSize());
        pBmp = filt.apply(pBaseBmp);
        testEqual(*pBmp, *nullBmp);
        for(int i=0;i<1;i++){
            pBmp = filt.apply(pBaseBmp);
            testEqual(*pBmp, *nullBmp);
        }
    }

};
    
class FilterFastBandpassTest: public Test {
public:
    FilterFastBandpassTest()
        : Test("FilterFastBandpassTest", 2)
    {
    }

    void runTests()
    {
        BitmapPtr pBmp = BitmapPtr(new Bitmap(IntPoint(16,16), I8));
        FilterFill<Pixel8>(0).applyInPlace(pBmp);
        *(pBmp->getPixels()+pBmp->getStride()*7+7) = 255;
        BitmapPtr pDestBmp = FilterFastBandpass().apply(pBmp);
//        pDestBmp->save("testimages/FastBandpassResult.png");
        string sFName = getSrcDir()+"testimages/FastBandpassResult.png";
        BitmapPtr pBaselineBmp = FilterGrayscale().apply(
                BitmapPtr(new Bitmap(sFName)));
        TEST(*pDestBmp == *pBaselineBmp);
    }
};


class FilterHighpassTest: public Test {
public:
    FilterHighpassTest()
        : Test("FilterHighpassTest", 2)
    {
    }

    void runTests()
    {
        BitmapPtr pBmp = BitmapPtr(new Bitmap(IntPoint(16,16), I8));
        FilterFill<Pixel8>(0).applyInPlace(pBmp);
        *(pBmp->getPixels()+pBmp->getStride()*7+7) = 255;
        BitmapPtr pDestBmp = FilterHighpass().apply(pBmp);
//        pDestBmp->save("testimages/HighpassResult.png");
        string sFName = getSrcDir()+"testimages/HighpassResult.png";
        BitmapPtr pBaselineBmp = FilterGrayscale().apply(
                BitmapPtr(new Bitmap(sFName)));
        TEST(*pDestBmp == *pBaselineBmp);
    }
};


class FilterGaussTest: public Test {
public:
    FilterGaussTest()
        : Test("FilterGaussTest", 2)
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
//        pDestBmp->save("testimages/Gauss3Result.png");
        string sFName = getSrcDir()+"testimages/Gauss3Result.png";
        BitmapPtr pBaselineBmp = FilterGrayscale().apply(
                BitmapPtr(new Bitmap(sFName)));
        TEST(*pDestBmp == *pBaselineBmp);
        pDestBmp = FilterGauss(1).apply(pBmp);
//        pDestBmp->save("testimages/Gauss1Result.png");
        sFName = getSrcDir()+"testimages/Gauss1Result.png";
        pBaselineBmp = FilterGrayscale().apply(
                BitmapPtr(new Bitmap(sFName)));
        TEST(*pDestBmp == *pBaselineBmp);
        pDestBmp = FilterGauss(1.5).apply(pBmp);
//        pDestBmp->save("testimages/Gauss15Result.png");
        sFName = getSrcDir()+"testimages/Gauss15Result.png";
        pBaselineBmp = FilterGrayscale().apply(
                BitmapPtr(new Bitmap(sFName)));
        TEST(*pDestBmp == *pBaselineBmp);
        pDestBmp = FilterGauss(5).apply(pBmp);
//        pDestBmp->save("testimages/Gauss5Result.png");
        sFName = getSrcDir()+"testimages/Gauss5Result.png";
        pBaselineBmp = FilterGrayscale().apply(
                BitmapPtr(new Bitmap(sFName)));
        TEST(*pDestBmp == *pBaselineBmp);
    }
};


class FilterBlurTest: public Test {
public:
    FilterBlurTest()
        : Test("FilterBlurTest", 2)
    {
    }

    void runTests()
    {
        BitmapPtr pBmp = BitmapPtr(new Bitmap(IntPoint(16,16), I8));
        FilterFill<Pixel8>(0).applyInPlace(pBmp);
        *(pBmp->getPixels()+pBmp->getStride()*7+7) = 255;
        BitmapPtr pDestBmp = FilterBlur().apply(pBmp);
//        pDestBmp->save("testimages/BlurResult.png");
        BitmapPtr pBaselineBmp = FilterGrayscale().apply(
                BitmapPtr(new Bitmap(getSrcDir()+"testimages/BlurResult.png")));
        TEST(*pDestBmp == *pBaselineBmp);
    }
};


class FilterBandpassTest: public Test {
public:
    FilterBandpassTest()
        : Test("FilterBandpassTest", 2)
    {
    }

    void runTests()
    {
        BitmapPtr pBmp = BitmapPtr(new Bitmap(IntPoint(16,16), I8));
        FilterFill<Pixel8>(0).applyInPlace(pBmp);
        *(pBmp->getPixels()+pBmp->getStride()*7+7) = 255;
        
        BitmapPtr pDestBmp = FilterBandpass(1.9,3).apply(pBmp);
//        pDestBmp->save("testimages/BandpassResult.png");
        string sFName = getSrcDir()+"testimages/BandpassResult.png";
        BitmapPtr pBaselineBmp = FilterGrayscale().apply(
                BitmapPtr(new Bitmap(sFName)));
        TEST(*pDestBmp == *pBaselineBmp);
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
    }
};


int main(int nargs, char** args)
{
    GraphicsTestSuite Suite;
    Suite.runTests();
    bool bOK = Suite.isOk();

    if (bOK) {
        return 0;
    } else {
        return 1;
    }
}

