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

#include "../base/TestSuite.h"
#include "../base/Exception.h"

#include <iostream>

#include <stdio.h>

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
        runPFTests(YCbCr422);
        
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
        runSaveTest(R8G8B8A8);
        runSaveTest(R8G8B8X8);
    }
    
private:
    void runPFTests(PixelFormat PF) {
        cerr << "    Testing " << Bitmap::getPixelFormatString(PF) << endl;
        BitmapPtr pBmp = initBmp(PF);
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
    }

    void runSaveTest(PixelFormat PF) {
        cerr << "    Testing save for " << Bitmap::getPixelFormatString(PF) << endl;
        BitmapPtr pBmp = initBmp(PF);
        pBmp->save("test.tif");
        Bitmap LoadedBmp("test.tif");
        ::remove("test.tif");
        testEqual(*pBmp, LoadedBmp);
    }

    void testEqual(Bitmap& Bmp1, Bitmap& Bmp2) 
    {
        TEST(Bmp1 == Bmp2);
        if (!(Bmp1 == Bmp2)) {
            cerr << "Bmp1: " << endl;
            Bmp1.dump();
            cerr << "Bmp2: " << endl;
            Bmp2.dump();
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
        Bitmap TempBmp("../test/rgb24.png");
        PixelFormat pf = R8G8B8;    
        BitmapPtr pBmp;
        pBmp = createBmp(TempBmp.getSize(), pf);
        pBmp->copyPixels(TempBmp);
        FilterColorize(15, 50).applyInPlace(pBmp);
        FilterFlipRGB().applyInPlace(pBmp);
    }

private:
    BitmapPtr createBmp(const IntPoint& Size, PixelFormat pf)
    {
        BitmapPtr pBmp;
        pBmp = BitmapPtr(new Bitmap(Size, pf));
        return pBmp;
    }
};

class GraphicsTestSuite: public TestSuite {
public:
    GraphicsTestSuite() 
        : TestSuite("GraphicsTestSuite")
    {
        addTest(TestPtr(new BitmapTest));
        addTest(TestPtr(new FilterColorizeTest));
        addTest(TestPtr(new FilterGrayscaleTest));
        addTest(TestPtr(new FilterFillTest));
        addTest(TestPtr(new FilterFlipTest));
        addTest(TestPtr(new FilterFlipRGBTest));
        addTest(TestPtr(new FilterFlipUVTest));
        addTest(TestPtr(new FilterComboTest));
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

