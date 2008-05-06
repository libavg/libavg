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

#include "../base/TestSuite.h"

#include <SDL/SDL.h>

#include <iostream>

using namespace avg;
using namespace std;

class FBOTest: public Test {
public:
    FBOTest()
        : Test("FBOTest", 2)
    {
    }

    void runTests() 
    {
        if (!FBOImage::isFBOSupported()) {
            cerr << "  GL_EXT_framebuffer_object not supported. Skipping FBO test." << endl;
        }
        runImageTests("../test/rgb24-64x64.png");
        runImageTests("../test/rgb24alpha-64x64.png");
    }

private:
    void runImageTests(const string& sFName) {
        cerr << "    Testing " << sFName << endl;
        BitmapPtr pBmp(new Bitmap(sFName));
        FilterFlipRGB().applyInPlace(pBmp);
        FBOImage fbo(pBmp->getSize(), pBmp->getPixelFormat());
        fbo.setImage(pBmp);
        BitmapPtr pNewBmp = fbo.getImage();
        testEqual(*pBmp, *pNewBmp);
    }

    void testEqual(Bitmap& Bmp1, Bitmap& Bmp2) 
    {
        TEST(Bmp1 == Bmp2);
        if (!(Bmp1 == Bmp2)) {
            Bmp2.save("result.png");
        }
    }
};

class GPUTestSuite: public TestSuite {
public:
    GPUTestSuite() 
        : TestSuite("GPUTestSuite")
    {
        addTest(TestPtr(new FBOTest));
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

    GPUTestSuite Suite;
    Suite.runTests();
    bool bOK = Suite.isOk();

    SDL_QuitSubSystem(SDL_INIT_VIDEO);
    if (bOK) {
        return 0;
    } else {
        return 1;
    }
}

