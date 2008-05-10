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

#include "GraphicsTest.h"
#include "FBOImage.h"
#include "GPUBrightnessFilter.h"
#include "GPUBlurFilter.h"
#include "OGLImagingContext.h"

#include "../base/TestSuite.h"

#include <SDL/SDL.h>

#include <iostream>

using namespace avg;
using namespace std;

class FBOTest: public GraphicsTest {
public:
    FBOTest()
        : GraphicsTest("FBOTest", 2)
    {
    }

    void runTests() 
    {
        runImageTests("i8-64x64", GL_UNSIGNED_BYTE, I8);
        runImageTests("rgb24-64x64", GL_UNSIGNED_BYTE);
        runImageTests("rgb24alpha-64x64", GL_UNSIGNED_BYTE);
        runImageTests("i8-64x64", GL_FLOAT, I8);
        runImageTests("rgb24-64x64", GL_FLOAT);
        runImageTests("rgb24alpha-64x64", GL_FLOAT);

        runParameterPBOTest();
    }

private:
    void runImageTests(const string& sFName, int precision, PixelFormat pf = R8G8B8X8)
    {
        BitmapPtr pBmp = loadTestBmp(sFName, pf);
        cerr << "    Testing " << sFName << " (" << pBmp->getPixelFormatString() << ")" << endl;
        cerr << "      PBO:" << endl;
        PBOImage pbo(pBmp->getSize(), pBmp->getPixelFormat(), precision);
        runPBOImageTest(pbo, pBmp, string("pbo_")+sFName);
        if (pf != I8) {
            cerr << "      FBO:" << endl;
            FBOImage fbo(pBmp->getSize(), pBmp->getPixelFormat(), precision);
            runPBOImageTest(fbo, pBmp, string("fbo_")+sFName);
        }
    }

    void runParameterPBOTest()
    {
        cerr << "    Testing parameter PBO" << endl;
        PBOImage pbo(IntPoint(11, 1), I8, GL_FLOAT);
        float data[11];
        pbo.setImage(data);
    }

    void runPBOImageTest(PBOImage& pbo, BitmapPtr pBmp, const string& sFName)
    {
        pbo.setImage(pBmp);
        BitmapPtr pNewBmp = pbo.getImage();
        testEqual(*pNewBmp, *pBmp, sFName);
    }

};

class BrightnessFilterTest: public GraphicsTest {
public:
    BrightnessFilterTest()
        : GraphicsTest("BrightnessFilterTest", 2)
    {
    }

    void runTests() 
    {
        runImageTests("rgb24-64x64");
        runImageTests("rgb24alpha-64x64");
    }

private:
    void runImageTests(const string& sFName)
    {
        cerr << "    Testing " << sFName << endl;
        BitmapPtr pBmp = loadTestBmp(sFName);
        BitmapPtr pDestBmp;
        pDestBmp = GPUBrightnessFilter(pBmp->getSize(), pBmp->getPixelFormat(), 1).apply(pBmp);
        testEqual(*pDestBmp, *pBmp, string("brightness_")+sFName);
    }
};

class BlurFilterTest: public GraphicsTest {
public:
    BlurFilterTest()
        : GraphicsTest("BlurFilterTest", 2)
    {
    }

    void runTests() 
    {
        BitmapPtr pBmp;
        BitmapPtr pDestBmp;
/*
        // This has the effect of printing out all the brightness differences for different
        // kernel sizes.
        pBmp = loadTestBmp("spike");
        for (double stddev = 0.5; stddev < 5; stddev += 0.25) {
            pDestBmp = GPUBlurFilter(pBmp->getSize(), pBmp->getPixelFormat(), stddev).apply(pBmp);
            testEqualBrightness(*pDestBmp, *pBmp, 1);
        }
*/
        cerr << "    Testing spike, stddev 0.5" << endl;
        pBmp = loadTestBmp("spike");
        pDestBmp = GPUBlurFilter(pBmp->getSize(), pBmp->getPixelFormat(), 0.5).apply(pBmp);
        testEqualBrightness(*pDestBmp, *pBmp, 5);
        testEqual(*pDestBmp, "blur05_spike");
        cerr << "    Testing spike, stddev 1" << endl;
        pDestBmp = GPUBlurFilter(pBmp->getSize(), pBmp->getPixelFormat(), 1).apply(pBmp);
//        testEqualBrightness(*pDestBmp, *pBmp, 5);
        testEqual(*pDestBmp, "blur1_spike");
        cerr << "    Testing spike, stddev 3" << endl;
        pDestBmp = GPUBlurFilter(pBmp->getSize(), pBmp->getPixelFormat(), 3).apply(pBmp);
//        testEqualBrightness(*pDestBmp, *pBmp, 5);
        testEqual(*pDestBmp, "blur5_spike");

        cerr << "    Testing flat, stddev 5" << endl;
        pBmp = loadTestBmp("flat");
        pDestBmp = GPUBlurFilter(pBmp->getSize(), pBmp->getPixelFormat(), 5).apply(pBmp);
        testEqualBrightness(*pDestBmp, *pBmp, 1);
        testEqual(*pDestBmp, *pBmp, "blur05_flat");

        runImageTests("rgb24-64x64");
        runImageTests("rgb24alpha-64x64");
    }

private:
    void runImageTests(const string& sFName)
    {
        cerr << "    Testing " << sFName << endl;
        BitmapPtr pBmp = loadTestBmp(sFName);
        BitmapPtr pDestBmp;
        pDestBmp = GPUBlurFilter(pBmp->getSize(), pBmp->getPixelFormat(), 10).apply(pBmp);
        testEqual(*pDestBmp, string("blur_")+sFName);
    }
};

class GPUTestSuite: public TestSuite {
public:
    GPUTestSuite() 
        : TestSuite("GPUTestSuite")
    {
        addTest(TestPtr(new FBOTest));
        addTest(TestPtr(new BrightnessFilterTest));
        addTest(TestPtr(new BlurFilterTest));
    }
};


int main(int nargs, char** args)
{
    GraphicsTest::createResultImgDir();

    if (SDL_InitSubSystem(SDL_INIT_VIDEO)==-1) {
        cerr << "Can't init SDL." << endl;
        return 1;
    }
    SDL_Surface * pScreen = SDL_SetVideoMode(16, 16, 24, SDL_OPENGL);
    if (!pScreen) {
        cerr << "Setting SDL video mode failed: " << SDL_GetError() << endl;
        return 1;
    }
    glproc::init();
    OGLImagingContext context(IntPoint(64, 64));

    bool bOK;
    if (!FBOImage::isFBOSupported()) {
        bOK = true;
        cerr << "  GL_EXT_framebuffer_object not supported. Skipping FBO test." << endl;
    } else {
        GPUTestSuite Suite;
        Suite.runTests();
        bOK = Suite.isOk();
    }

    SDL_QuitSubSystem(SDL_INIT_VIDEO);
    if (bOK) {
        return 0;
    } else {
        return 1;
    }
}

