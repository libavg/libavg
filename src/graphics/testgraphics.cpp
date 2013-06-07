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

#include "GraphicsTest.h"
#include "Bitmap.h"
#include "BitmapLoader.h"
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
#include "FilterThreshold.h"
#include "FilterFloodfill.h"
#include "FilterDilation.h"
#include "FilterErosion.h"
#include "FilterGetAlpha.h"
#include "FilterResizeBilinear.h"
#include "FilterUnmultiplyAlpha.h"

#include "../base/TestSuite.h"
#include "../base/Exception.h"
#include "../base/MathHelper.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable: 4251)
#endif
#ifdef _WIN32
#pragma warning(pop)
#endif

#include <cstring>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include <glib-object.h>

using namespace avg;
using namespace std;

BitmapPtr initBmp(PixelFormat pf)
{
    int height;
    if (pf == YCbCr422) {
        height = 10;
    } else {
        height = 7;

    }
    BitmapPtr pBmp(new Bitmap(IntPoint(4, height), pf));
    int bpp = pBmp->getBytesPerPixel();
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < 4; ++x) {
            unsigned char * pPixel = 
                    pBmp->getPixels()+y*pBmp->getStride()+x*pBmp->getBytesPerPixel();
            *(pPixel) = x;
            if (bpp > 1) {
                *(pPixel+1) = 0;
            }
            if (bpp > 2) {
                *(pPixel+2) = x;
                *(pPixel) = 16*y;
            }
            if (bpp > 3) {
                *(pPixel+3) = 0x80;
            }
        }
    }
    return pBmp;
}

// TODO: This is very incomplete!
class PixelTest: public GraphicsTest {
public:
    PixelTest()
      : GraphicsTest("PixelTest", 2)
    {
    }

    void runTests()
    {
        float h, s, l;
        Pixel32(255, 0, 0).toHSL(h, s, l);
        TEST(h == 0 && almostEqual(s, 1.0f) && almostEqual(l, 0.5f));
        Pixel32(0, 255, 0).toHSL(h, s, l);
        TEST(h == 120 && almostEqual(s, 1.0f) && almostEqual(l, 0.5f));
        Pixel32(0, 0, 255).toHSL(h, s, l);
        TEST(h == 240 && almostEqual(s, 1.0f) && almostEqual(l, 0.5f));
        Pixel32(255, 255, 0).toHSL(h, s, l);
        TEST(h == 60 && almostEqual(s, 1.0f) && almostEqual(l, 0.5f));
        Pixel32(255, 0, 255).toHSL(h, s, l);
        TEST(h == 300 && almostEqual(s, 1.0f) && almostEqual(l, 0.5f));
        Pixel32(0, 255, 255).toHSL(h, s, l);
        TEST(h == 180 && almostEqual(s, 1.0f) && almostEqual(l, 0.5f));
        Pixel32(0, 0, 0).toHSL(h, s, l);
        TEST(s == 0.0f && l == 0.0f);
        Pixel32(255, 255, 255).toHSL(h, s, l);
        TEST(s == 0.0f && l == 1.0f);
        Pixel32(128, 128, 128).toHSL(h, s, l);
        TEST(s == 0.0f && almostEqual(l, 0.5f, 0.02f));
    }
};

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
        for (int i = 0; i < 4*7*3; ++i) {
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
            for(int y = 0; y < 7; ++y) {
                for (int x = 0; x < 4; ++x) {
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
            for(int y = 0; y < 7; ++y) {
                for (int x = 0; x < 4; ++x) {
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
            cerr << "    Testing copyPixels - B5G6R5->B8G8R8->B5G6R5." << endl;
            BitmapPtr pBmp = initBmp(B5G6R5);
            BitmapPtr pBaselineBmp = initBmp(B5G6R5);
            BitmapPtr pCopyBmp = BitmapPtr(new Bitmap(IntPoint(4,7), B8G8R8));
            pCopyBmp->copyPixels(*pBmp);
            pBmp->copyPixels(*pCopyBmp);
            testEqual(*pBmp, *pBaselineBmp, "BmpCopyPixels_4");
        }
        testCopyToGreyscale(R8G8B8X8);
        testCopyToGreyscale(B8G8R8X8);
        testSubtract();
        {
            cerr << "    Testing statistics." << endl;
            cerr << "      I8" << endl;
            testStatistics(I8, Pixel8(0), Pixel8(0), Pixel8(2), Pixel8(2));
            cerr << "      R8G8B8A8" << endl;
            testStatistics(R8G8B8A8, Pixel32(0,0,0,0), Pixel32(0,0,0,0), 
                    Pixel32(255,255,255,255), Pixel32(255,255,255,255), 127.5, 90.1561);
            cerr << "      R8G8B8X8" << endl;
            testStatistics(R8G8B8X8, Pixel32(0,0,0,255), Pixel32(0,0,0,255), 
                    Pixel32(2,2,2,255), Pixel32(2,2,2,255));
            cerr << "      R8G8B8" << endl;
            testStatistics(R8G8B8, Pixel24(0,0,0), Pixel24(0,0,0), 
                    Pixel24(2,2,2), Pixel24(2,2,2));
            cerr << "      ChannelAvg" << endl;
            testChannelAvg();
        }
        {
            cerr << "    Testing YUV->RGB conversion." << endl;
            testYUV2RGB();
        }
        runSaveTest(B8G8R8A8);
        runSaveTest(B8G8R8X8);
    }
    
private:
    void runPFTests(PixelFormat pf)
    {
        cerr << "    Testing " << pf << endl;
        BitmapPtr pBmp = initBmp(pf);
        {
            cerr << "      Testing size." <<endl;
            if (pf == YCbCr422) {
                TEST(pBmp->getSize() == IntPoint(4,10));
            } else {
                TEST(pBmp->getSize() == IntPoint(4,7));
            }
        }
        {
            cerr << "      Testing copy constructor." << endl;
            Bitmap bmpCopy1(*pBmp);
            testEqual(bmpCopy1, *pBmp, "BmpCopyConstructor");
        }
        {
            cerr << "      Testing assignment operator." << endl;
            Bitmap bmpCopy2 = *pBmp;
            testEqual(bmpCopy2, *pBmp, "BmpAssignment");
        }
        {
            cerr << "      Testing sub-bitmap constructor." << endl;
            Bitmap bmpCopy3 (*pBmp, IntRect(IntPoint(0,0), pBmp->getSize()));
            testEqual(bmpCopy3, *pBmp, "BmpSubBmpCtor");
        }
        if (pf == I8) {
            cerr << "      Testing getHistogram." << endl;
            HistogramPtr pHist = pBmp->getHistogram();
            TEST((*pHist)[0] == 7);
            TEST((*pHist)[1] == 7);
            TEST((*pHist)[2] == 7);
            TEST((*pHist)[3] == 7);
            bool bOk = true;
            for (int i = 4; i < 256; ++i) {
                if (bOk) {
                    bOk = ((*pHist)[i] == 0);
                }
            }
            TEST(bOk);   
        }
    }

    void runSaveTest(PixelFormat pf)
    {
        cerr << "    Testing save for " << pf << endl;
        BitmapPtr pBmp = initBmp(pf);
        pBmp->save("test.png");
        BitmapPtr pLoadedBmp = loadBitmap("test.png");
        ::remove("test.png");
        testEqual(*pLoadedBmp, *pBmp, "BmpSave");
    }

    template<class PIXEL>
    void runLineTest(PixelFormat pf, PIXEL color)
    {
        cerr << "    Testing line drawing for " << pf << endl;
        Bitmap bmp(IntPoint(15, 15), pf);
        memset(bmp.getPixels(), 0, bmp.getStride()*15);
        bmp.drawLine(IntPoint(7,7), IntPoint( 0, 2), color);
        bmp.drawLine(IntPoint(7,7), IntPoint( 0,12), color);
        bmp.drawLine(IntPoint(7,7), IntPoint( 2, 0), color);
        bmp.drawLine(IntPoint(7,7), IntPoint( 2,14), color);
        bmp.drawLine(IntPoint(7,7), IntPoint(12, 0), color);
        bmp.drawLine(IntPoint(7,7), IntPoint(12,14), color);
        bmp.drawLine(IntPoint(7,7), IntPoint(14, 2), color);
        bmp.drawLine(IntPoint(7,7), IntPoint(14,12), color);
        string sFName = getSrcDirName() + "baseline/LineResult" + getPixelFormatString(pf)
                + ".png";
        BitmapPtr pBaselineBmp = loadBitmap(sFName, pf);
        testEqual(bmp, *pBaselineBmp, "BmpLineDraw");
    }
    
    void testCopyToGreyscale(PixelFormat pf)
    {
        cerr << "    Testing copyPixels - " << pf << "->I8." << endl;
        BitmapPtr pBmp(new Bitmap(IntPoint(4,4), pf));
        for (int y=0; y<4; ++y) {
            for (int x=0; x<4; ++x) {
                unsigned char * pPixel = 
                    pBmp->getPixels()+y*pBmp->getStride()+x*pBmp->getBytesPerPixel();
                pPixel[0] = x*64;
                pPixel[1] = 128;
                pPixel[2] = y*64;
                pPixel[3] = 255;
            }
        }
        BitmapPtr pCopyBmp = BitmapPtr(new Bitmap(IntPoint(4,4), I8));
        pCopyBmp->copyPixels(*pBmp);
        testEqual(*pCopyBmp, string("copyPixels_")+getPixelFormatString(pf)+"_I8",
                I8, 0.5, 0.5);
    }
   
    void testSubtract()
    {
        BitmapPtr pBmp1(new Bitmap(IntPoint(4,1), I8));
        BitmapPtr pBmp2(new Bitmap(IntPoint(4,1), I8));
        for (int x=0; x<4; ++x) {
            pBmp1->getPixels()[x] = x;
            pBmp2->getPixels()[x] = 0;
        }
        BitmapPtr pDiffBmp = pBmp1->subtract(*pBmp2);
        testEqual(*pDiffBmp, *pBmp1, "BmpSubtract1");
        pDiffBmp = pBmp2->subtract(*pBmp1);
        testEqual(*pDiffBmp, *pBmp1, "BmpSubtract2");
    }

    template<class PIXEL>
    void testStatistics(PixelFormat pf, const PIXEL& p00, const PIXEL& p01,
            const PIXEL& p10, const PIXEL& p11, float avg=1, float stdDev=1)
    {
        BitmapPtr pBmp = BitmapPtr(new Bitmap(IntPoint(2,2), pf));
        pBmp->setPixel(IntPoint(0,0), p00);
        pBmp->setPixel(IntPoint(0,1), p01);
        pBmp->setPixel(IntPoint(1,0), p10);
        pBmp->setPixel(IntPoint(1,1), p11);
        TEST(almostEqual(pBmp->getAvg(), avg, 0.001));
        TEST(almostEqual(pBmp->getStdDev(), stdDev, 0.001));
    }

    void testChannelAvg()
    {
        BitmapPtr pBmp = BitmapPtr(new Bitmap(IntPoint(2,2), R8G8B8));
        pBmp->setPixel(IntPoint(0,0), Pixel24(0,0,0));
        pBmp->setPixel(IntPoint(0,1), Pixel24(128,0,0));
        pBmp->setPixel(IntPoint(1,0), Pixel24(128,128,0));
        pBmp->setPixel(IntPoint(1,1), Pixel24(128,128,128));
        TEST(almostEqual(pBmp->getChannelAvg(0), 96));
        TEST(almostEqual(pBmp->getChannelAvg(1), 64));
        TEST(almostEqual(pBmp->getChannelAvg(2), 32));
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
        pRGBBmp->copyYUVPixels(*pYBmp, *pUBmp, *pVBmp, false);
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
    void runPFTests(PixelFormat pf) {
        BitmapPtr pBmp = initBmp(pf);
        FilterColorize(15, 0).applyInPlace(pBmp);
        FilterColorize(100, 50).applyInPlace(pBmp);
        FilterColorize(50, 100).applyInPlace(pBmp);
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
    void runPFTests(PixelFormat pf) {
        BitmapPtr pBmp = initBmp(pf);
        FilterGrayscale().applyInPlace(pBmp);
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
    void runPFTests(PixelFormat pf, PixelC color) {
        BitmapPtr pBmp = initBmp(pf);
        FilterFill<PixelC>(color).applyInPlace(pBmp);
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
    void runPFTests(PixelFormat pf) {
        BitmapPtr pBmp = initBmp(pf);
        {
            BitmapPtr pBmp1 = FilterFlip().apply(pBmp);
            BitmapPtr pBmp2 = FilterFlip().apply(pBmp1);
            TEST(*pBmp == *pBmp2);
        }
        pBmp = initBmp(pf);
        { 
            Bitmap baselineBmp = *pBmp;
            FilterFlip().applyInPlace(pBmp);
            FilterFlip().applyInPlace(pBmp);
            TEST(*pBmp == baselineBmp);
        }
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
    void runPFTests(PixelFormat pf) {
        BitmapPtr pBmp = initBmp(pf);
        {
            BitmapPtr pBmp1 = FilterFlipRGB().apply(pBmp);
            BitmapPtr pBmp2 = FilterFlipRGB().apply(pBmp1);
            TEST(*pBmp == *pBmp2);
        }
        { 
            Bitmap baselineBmp = *pBmp;
            FilterFlipRGB().applyInPlace(pBmp);
            FilterFlipRGB().applyInPlace(pBmp);
            TEST(*pBmp == baselineBmp);
        }
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
            Bitmap baselineBmp = *pBmp;
            FilterFlipUV().applyInPlace(pBmp);
            FilterFlipUV().applyInPlace(pBmp);
            TEST(*pBmp == baselineBmp);
        }
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
            sFilename += "../test/media/rgb24-64x64.png";
            BitmapPtr pBmp = loadBitmap(sFilename, R8G8B8);
            FilterColorize(15, 50).applyInPlace(pBmp);
            FilterFlipRGB().applyInPlace(pBmp);
        } catch (Exception & ex) {
            cerr << ex.getStr() << endl;
            setFailed();
        }
    }

private:
    BitmapPtr createBmp(const IntPoint& size, PixelFormat pf)
    {
        BitmapPtr pBmp;
        pBmp = BitmapPtr(new Bitmap(size, pf));
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
    template<class PIXEL>
    void runPFTests(PixelFormat pf)
    {
        BitmapPtr pBmp(new Bitmap(IntPoint(4, 4), pf));
        initBmp<PIXEL>(pBmp);
        float mat[9] = 
                {1,0,2,
                 0,1,0,
                 3,0,4};
        BitmapPtr pNewBmp = FilterConvol<PIXEL>(&(mat[0]),3,3).apply(pBmp);
        TEST(pNewBmp->getSize() == IntPoint(2,2));
        unsigned char * pLine0 = pNewBmp->getPixels();
        TEST(*(PIXEL*)pLine0 == PIXEL(1,0,0));
        TEST(*(((PIXEL*)pLine0)+1) == PIXEL(4,0,0));
        unsigned char * pLine1 = pNewBmp->getPixels()+pNewBmp->getStride();
        TEST(*(PIXEL*)(pLine1) == PIXEL(0,0,9));
        TEST(*((PIXEL*)(pLine1)+1) == PIXEL(0,0,16));
        
    }
    
    template<class PIXEL>
    void initBmp(BitmapPtr pBmp) 
    {
        PIXEL * pPixels = (PIXEL *)(pBmp->getPixels());
        PIXEL color = PIXEL(0,0,0);
        FilterFill<PIXEL>(color).applyInPlace(pBmp);
        pPixels[0] = PIXEL(1,0,0);
        pPixels[3] = PIXEL(2,0,0);
        pPixels = (PIXEL*)((char *)pPixels+3*pBmp->getStride());
        pPixels[0] = PIXEL(0,0,3);
        pPixels[3] = PIXEL(0,0,4);
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
    template<class PIXEL>
    void runPFTests(PixelFormat pf)
    {
        BitmapPtr pBmp(new Bitmap(IntPoint(4, 4), pf));
        initBmp<PIXEL>(pBmp);
        float mat[3][3] = 
                {{1,0,2},
                 {0,1,0},
                 {3,0,4}};
        BitmapPtr pNewBmp = Filter3x3(mat).apply(pBmp);
        TEST(pNewBmp->getSize() == IntPoint(2,2));
        unsigned char * pLine0 = pNewBmp->getPixels();
        TEST(*(PIXEL*)pLine0 == PIXEL(1,0,0));
        TEST(*(((PIXEL*)pLine0)+1) == PIXEL(4,0,0));
        unsigned char * pLine1 = pNewBmp->getPixels()+pNewBmp->getStride();
        TEST(*(PIXEL*)(pLine1) == PIXEL(0,0,9));
        TEST(*((PIXEL*)(pLine1)+1) == PIXEL(0,0,16));
        
    }
    
    template<class PIXEL>
    void initBmp(BitmapPtr pBmp) 
    {
        PIXEL * pPixels = (PIXEL *)(pBmp->getPixels());
        PIXEL color = PIXEL(0,0,0);
        FilterFill<PIXEL>(color).applyInPlace(pBmp);
        pPixels[0] = PIXEL(1,0,0);
        pPixels[3] = PIXEL(2,0,0);
        pPixels = (PIXEL*)((char *)pPixels+3*pBmp->getStride());
        pPixels[0] = PIXEL(0,0,3);
        pPixels[3] = PIXEL(0,0,4);
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
        for (int y = 0; y < pBmp->getSize().y; y++) {
            pMaskBmp->setPixel(IntPoint(1, y), Pixel8(128));
            pMaskBmp->setPixel(IntPoint(2, y), Pixel8(255));
            pMaskBmp->setPixel(IntPoint(3, y), Pixel8(255));
        }

        BitmapPtr pDestBmp = FilterMask(pMaskBmp).apply(pBmp);
        string sFName = string("baseline/MaskResult")+sName+".png";
//        pDestBmp->save(sFName);
        sFName = getSrcDirName()+sFName;
        BitmapPtr pBaselineBmp = loadBitmap(sFName, pBmp->getPixelFormat());
        TEST(*pDestBmp == *pBaselineBmp);
    }
};

class FilterThresholdTest: public GraphicsTest {
public:
    FilterThresholdTest()
        : GraphicsTest("FilterThresholdTest", 2)
    {
    }

    void runTests()
    {
        BitmapPtr pBmp(initBmp(I8));
        BitmapPtr pDestBmp = FilterThreshold(1).apply(pBmp);
        string sFName = "baseline/ThresholdResult.png";
//        pDestBmp->save(sFName);
        sFName = getSrcDirName()+sFName;
        BitmapPtr pBaselineBmp = loadBitmap(sFName, pBmp->getPixelFormat());
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
        pBmp = loadTestBmp("rgb24-65x65", B8G8R8);
        runTestWithBitmap(pBmp);
    }

private:
    void runTestWithBitmap(BitmapPtr pBmp)
    {
        BitmapPtr pDestBmp = FilterResizeBilinear(IntPoint(32,32)).apply(pBmp);
        string sName = string("ResizeBilinearResult")
                +getPixelFormatString(pBmp->getPixelFormat());
        testEqual(*pDestBmp, sName, pBmp->getPixelFormat());
    }

};

class FilterUnmultiplyAlphaTest: public GraphicsTest {
public:
    FilterUnmultiplyAlphaTest()
        : GraphicsTest("FilterUnmultiplyAlphaTest", 2)
    {
    }

    void runTests()
    {
        BitmapPtr pBmp(new Bitmap(IntPoint(16, 1), B8G8R8A8));
        for (int x = 0; x < 16; ++x) {
            unsigned char * pPixel = pBmp->getPixels()+x*4;
            unsigned char val = 17*x;
            *(pPixel+REDPOS) = val;
            *(pPixel+GREENPOS) = val;
            *(pPixel+BLUEPOS) = val;
            *(pPixel+ALPHAPOS) = val;
        }
        FilterUnmultiplyAlpha().applyInPlace(pBmp);
        QUIET_TEST(*(pBmp->getPixels()+ALPHAPOS) == 0);
        for (int x = 1; x < 16; ++x) {
            unsigned char * pPixel = pBmp->getPixels()+x*4;
            QUIET_TEST(*(pPixel+REDPOS) == 255);
            QUIET_TEST(*(pPixel+GREENPOS) == 255);
            QUIET_TEST(*(pPixel+BLUEPOS) == 255);
            QUIET_TEST(*(pPixel+ALPHAPOS) == 17*x);
        }
    }

private:

};

class GraphicsTestSuite: public TestSuite {
public:
    GraphicsTestSuite() 
        : TestSuite("GraphicsTestSuite")
    {
        addTest(TestPtr(new PixelTest));
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
        addTest(TestPtr(new FilterThresholdTest));
        addTest(TestPtr(new FilterFloodfillTest));
        addTest(TestPtr(new FilterDilationTest));
        addTest(TestPtr(new FilterErosionTest));
        addTest(TestPtr(new FilterAlphaTest));
        addTest(TestPtr(new FilterResizeBilinearTest));
        addTest(TestPtr(new FilterUnmultiplyAlphaTest));
    }
};


int main(int nargs, char** args)
{
    g_type_init();
    BitmapLoader::init(true);
    GraphicsTest::createResultImgDir();
    GraphicsTestSuite suite;
    suite.runTests();
    bool bOK = suite.isOk();

    if (bOK) {
        return 0;
    } else {
        return 1;
    }
}

