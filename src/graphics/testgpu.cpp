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

#include "FBOImage.h"
#include "OGLHelper.h"
#include "Filterfliprgb.h"
#include "GPUBrightnessFilter.h"

#include "../base/TestSuite.h"
#include "../base/Exception.h"

#include <SDL/SDL.h>

#include <iostream>

using namespace avg;
using namespace std;

class BmpTest: public Test {
public:
    BmpTest(const string& sName)
        : Test(sName, 2)
    {
    }


protected:
    void testEqual(Bitmap& Bmp1, Bitmap& Bmp2) 
    {
        TEST(Bmp1 == Bmp2);
        if (!(Bmp1 == Bmp2)) {
            Bmp2.save("result.png");
        }
    }
};

class FBOTest: public BmpTest {
public:
    FBOTest()
        : BmpTest("FBOTest")
    {
    }

    void runTests() 
    {
        runImageTests("../test/rgb24-64x64.png");
        runImageTests("../test/rgb24alpha-64x64.png");
    }

private:
    void runImageTests(const string& sFName)
    {
        cerr << "    Testing " << sFName << endl;
        BitmapPtr pBmp(new Bitmap(sFName));
        FilterFlipRGB().applyInPlace(pBmp);
        cerr << "      PBO:" << endl;
        PBOImage pbo(pBmp->getSize(), pBmp->getPixelFormat());
        runPBOImageTest(pbo, pBmp);
        
        cerr << "      FBO:" << endl;
        FBOImage fbo(pBmp->getSize(), pBmp->getPixelFormat());
        runPBOImageTest(fbo, pBmp);
    }

    void runPBOImageTest(PBOImage& pbo, BitmapPtr pBmp)
    {
        pbo.setImage(pBmp);
        BitmapPtr pNewBmp = pbo.getImage();
        testEqual(*pBmp, *pNewBmp);
    }

};

class BrightnessFilterTest: public BmpTest {
public:
    BrightnessFilterTest()
        : BmpTest("BrightnessFilterTest")
    {
    }

    void runTests() 
    {
        runImageTests("../test/rgb24-64x64.png");
        runImageTests("../test/rgb24alpha-64x64.png");
    }

private:
    void runImageTests(const string& sFName)
    {
        cerr << "    Testing " << sFName << endl;
        BitmapPtr pBmp(new Bitmap(sFName));
        FilterFlipRGB().applyInPlace(pBmp);
        BitmapPtr pDestBmp;
        pDestBmp = GPUBrightnessFilter(pBmp->getSize(), pBmp->getPixelFormat(), 1).apply(pBmp);
        testEqual(*pBmp, *pDestBmp);
    }
};

class GPUTestSuite: public TestSuite {
public:
    GPUTestSuite() 
        : TestSuite("GPUTestSuite")
    {
        addTest(TestPtr(new FBOTest));
        addTest(TestPtr(new BrightnessFilterTest));
    }
};


int main(int nargs, char** args)
{
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

    // TODO: Check which of these calls are necessary
    glViewport(0, 0, 64, 64);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "glViewport()");
    glMatrixMode(GL_PROJECTION);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "glMatrixMode()");
    glLoadIdentity();
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "glLoadIdentity()");
    gluOrtho2D(0, 64, 64, 0);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "gluOrtho2D()");
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); 
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "glTexEnvf()");
    glBlendFunc(GL_ONE, GL_ZERO);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "glBlendFunc()");
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "glLoadIdentity()");
    glEnable(GL_BLEND);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "glEnable(GL_BLEND)");
    glShadeModel(GL_FLAT);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "glShadeModel(GL_FLAT)");
    glDisable(GL_DEPTH_TEST);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "glDisable(GL_DEPTH_TEST)");
    glDisable(GL_STENCIL_TEST);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "glDisable(GL_STENCIL_TEST)");
    glEnable(GL_TEXTURE_RECTANGLE_ARB);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "glEnable(GL_TEXTURE_RECTANGLE_ARB);");
    glDisable(GL_MULTISAMPLE);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "glDisable(GL_MULTISAMPLE);");

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

