//
// $Id$
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

#include "../base/TestSuite.h"
#include "../base/Exception.h"

#include <iostream>

#include <stdio.h>

using namespace avg;
using namespace std;

BitmapPtr initBmp(PixelFormat PF)
{
    BitmapPtr pBmp(new Bitmap(IntPoint(5,5), PF));
    for(int y=0; y<5; ++y) {
        for (int x=0; x<5; ++x) {
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
                *(pPixel+3) = 0xFF;
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
        
        unsigned char pData[5*5*3];
        Bitmap Bmp1 = Bitmap(IntPoint(5,5), R8G8B8, pData, 15, true, "");
        Bitmap Bmp2 = Bitmap(IntPoint(5,5), R8G8B8, pData, 15, false, "");
        testEqual(Bmp1, Bmp2);
        {
            BitmapPtr pBmp = initBmp(R8G8B8X8);
            Bitmap Bmp1(IntPoint(5,5), R8G8B8);
            Bmp1.copyPixels(*pBmp);
            Bitmap Bmp2(IntPoint(5,5), R8G8B8X8);
            Bmp2.copyPixels(Bmp1);
            for(int y=0; y<5; ++y) {
                for (int x=0; x<5; ++x) {
                    *(Bmp2.getPixels()+y*Bmp2.getStride()+x*Bmp2.getBytesPerPixel()+3) = 0xFF;
                }
            }
            testEqual(*pBmp, Bmp2);
        }
        {
            BitmapPtr pBmp = initBmp(R8G8B8A8);
            pBmp->save("test.tiff");
            Bitmap LoadedBmp("test.tiff");
            ::remove("test.tiff");
            testEqual(*pBmp, LoadedBmp);
        }
    }
    
private:
    void runPFTests(PixelFormat PF) {
        BitmapPtr pBmp = initBmp(PF);
        {
            Bitmap BmpCopy1(*pBmp);
            testEqual(*pBmp, BmpCopy1);
        }
        {
            Bitmap BmpCopy2 = *pBmp;
            testEqual(*pBmp, BmpCopy2);
        }
        {
            Bitmap BmpCopy3 (*pBmp, IntRect(0,0,5,5));
            testEqual(*pBmp, BmpCopy3);
        }
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
            FilterFlip().applyInPlace(pBmp);
            FilterFlip().applyInPlace(pBmp);
            TEST(*pBmp == BmpBaseline);
        }
//        pBmp->dump();
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

