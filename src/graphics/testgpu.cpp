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
#include "GLTexture.h"
#include "GPUBrightnessFilter.h"
#include "GPUBlurFilter.h"
#include "GPUBandpassFilter.h"
#include "GPUChromaKeyFilter.h"
#include "GPUHueSatFilter.h"
#include "GPURGB2YUVFilter.h"
#include "FilterResizeBilinear.h"
#include "OGLImagingContext.h"
#include "BmpTextureMover.h"
#include "PBO.h"

#include "../base/TestSuite.h"
#include "../base/Exception.h"
#include "../base/Test.h"
#include "../base/StringHelper.h"

#include <math.h>
#include <iostream>

using namespace avg;
using namespace std;


class TextureMoverTest: public GraphicsTest {
public:
    TextureMoverTest()
        : GraphicsTest("TextureMoverTest", 2)
    {
    }

    void runTests() 
    {
        for (int i=0; i<2; ++i) {
            bool bPOT = (i==1);
            runImageTest(bPOT, MM_PBO, "rgb24-65x65");
            runImageTest(bPOT, MM_OGL, "rgb24-65x65");
            runImageTest(bPOT, MM_PBO, "rgb24alpha-64x64");
            runImageTest(bPOT, MM_OGL, "rgb24alpha-64x64");
        }
        runMipmapTest(MM_OGL, "rgb24alpha-64x64");
        runMipmapTest(MM_PBO, "rgb24alpha-64x64");
        runMipmapTest(MM_OGL, "rgb24-65x65");
        runMipmapTest(MM_PBO, "rgb24-65x65");
    }

private:
    void runImageTest(bool bPOT, OGLMemoryMode memoryMode, const string& sFName)
    {
        cerr << "    Testing " << sFName << ", " << oglMemoryMode2String(memoryMode);
        if (bPOT) {
            cerr << ", POT" << endl;
        } else {
            cerr << ", NPOT" << endl;
        }
        BitmapPtr pOrigBmp = loadTestBmp(sFName);
        {
            cerr << "      move functions." << endl;
            GLTexturePtr pTex = GLTexturePtr(new GLTexture(pOrigBmp->getSize(), 
                    pOrigBmp->getPixelFormat(), false, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE,
                    bPOT));
            TextureMoverPtr pWriteMover = TextureMover::create(memoryMode, 
                    pOrigBmp->getSize(), pOrigBmp->getPixelFormat(), GL_DYNAMIC_DRAW);
            pWriteMover->moveBmpToTexture(pOrigBmp, *pTex);
            BitmapPtr pDestBmp = readback(memoryMode, pOrigBmp, pTex);
            testEqual(*pDestBmp, *pOrigBmp, "pbo", 0.01, 0.1);
        }

        {
            cerr << "      lock functions." << endl;
            GLTexturePtr pTex = GLTexturePtr(new GLTexture(pOrigBmp->getSize(), 
                    pOrigBmp->getPixelFormat(), false, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE,
                    bPOT));
            TextureMoverPtr pMover = TextureMover::create(memoryMode, 
                    pOrigBmp->getSize(), pOrigBmp->getPixelFormat(), GL_DYNAMIC_DRAW);
            BitmapPtr pTransferBmp = pMover->lock();
            pTransferBmp->copyPixels(*pOrigBmp);
            pMover->unlock();
            pMover->moveToTexture(*pTex);
            BitmapPtr pDestBmp = readback(memoryMode, pOrigBmp, pTex);
            testEqual(*pDestBmp, *pOrigBmp, "pbo", 0.01, 0.1);
        }
    }

    void runMipmapTest(OGLMemoryMode memoryMode, const string& sFName)
    {
        cerr << "    Testing mipmap support, " << sFName << ", " << 
                oglMemoryMode2String(memoryMode) << endl;
        BitmapPtr pOrigBmp = loadTestBmp(sFName);
        GLTexturePtr pTex = GLTexturePtr(new GLTexture(pOrigBmp->getSize(), 
                    pOrigBmp->getPixelFormat(), true));
        pTex->moveBmpToTexture(pOrigBmp);
        pTex->generateMipmaps();
        TextureMoverPtr pReadMover = TextureMover::create(memoryMode, 
                pOrigBmp->getSize(), pOrigBmp->getPixelFormat(), GL_DYNAMIC_READ);
        BitmapPtr pResultBmp = pReadMover->moveTextureToBmp(*pTex, 1);
        IntPoint newSize(pOrigBmp->getSize()/2);
        TEST(pResultBmp->getSize() == newSize);
        FilterResizeBilinear resizer(newSize);
        BitmapPtr pBaselineBmp = resizer.apply(pOrigBmp);
        testEqual(*pResultBmp, *pBaselineBmp, "pbo-mipmap", 2, 5);
    }

    BitmapPtr readback(OGLMemoryMode memoryMode, const BitmapPtr& pOrigBmp, 
            const GLTexturePtr& pTex)
    {
        TextureMoverPtr pReadMover = TextureMover::create(memoryMode, 
                pTex->getGLSize(), pOrigBmp->getPixelFormat(), GL_DYNAMIC_READ);
        return pReadMover->moveTextureToBmp(*pTex);
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
        pDestBmp = GPUBrightnessFilter(pBmp->getSize(), pBmp->getPixelFormat(), 1)
                .apply(pBmp);
        testEqual(*pDestBmp, *pBmp, string("brightness_")+sFName, 0.2, 0.5);
    }
};

class ChromaKeyFilterTest: public GraphicsTest {
public:
    ChromaKeyFilterTest()
        : GraphicsTest("ChromaKeyFilterTest", 2)
    {
    }

    void runTests()
    {
        BitmapPtr pBmp = loadTestBmp("chromakey");
        BitmapPtr pDestBmp;
        GPUChromaKeyFilter filter(pBmp->getSize(), pBmp->getPixelFormat());
        for (int erosion = 0; erosion < 3; ++erosion) {
            filter.setParams(Pixel32(0,255,0), 0.1, 0.2, 0.1, 0.1, erosion, 0);
            pDestBmp = filter.apply(pBmp);
            testEqual(*pDestBmp, "ChromaKeyResult"+toString(erosion), R8G8B8X8, 0.3, 
                    0.7);
        }
        filter.setParams(Pixel32(0,255,0), 0.0, 0.0, 0.0, 0.0, 0, 0.1);
        pDestBmp = filter.apply(pBmp);
        testEqual(*pDestBmp, "ChromaKeySpillResult1", R8G8B8X8, 0.3, 0.7);
        filter.setParams(Pixel32(0,255,0), 0.1, 0.1, 0.1, 0.0, 0, 0.1);
        pDestBmp = filter.apply(pBmp);
        testEqual(*pDestBmp, "ChromaKeySpillResult2", R8G8B8X8, 0.3, 0.7);
        filter.setParams(Pixel32(0,255,0), 0.1, 0.1, 0.1, 0.0, 0, 0.2);
        pDestBmp = filter.apply(pBmp);
        testEqual(*pDestBmp, "ChromaKeySpillResult3", R8G8B8X8, 0.3, 0.7);

        pBmp = loadTestBmp("chromakey-median");
        filter.setParams(Pixel32(0,255,0), 0.1, 0.1, 0.1, 0.0, 0, 0.0);
        pDestBmp = filter.apply(pBmp);
        testEqual(*pDestBmp, "ChromaKeyMedianResult", R8G8B8X8, 1, 6);
    }
};

class HslColorFilterTest: public GraphicsTest {
public:
    HslColorFilterTest()
        : GraphicsTest("HslColorFilterTest", 2)
    {
    }

    void runTests()
    {
        BitmapPtr pBmp = loadTestBmp("hsl");
        BitmapPtr pDestBmp;
        GPUHueSatFilter filter(pBmp->getSize(), pBmp->getPixelFormat());
        //Test hue functionality
        for (int run = 0; run < 3; run++) {
            filter.setParams(run*90);
            pDestBmp = filter.apply(pBmp);
            testEqual(*pDestBmp, "HslHueResult"+toString(run), R8G8B8X8, 0, 0);
        }
        //Test colorize functionality
        for (int run = 0; run < 3; run++) {
            filter.setParams(run*90, 1, 0, true);
            pDestBmp = filter.apply(pBmp);
            testEqual(*pDestBmp, "HslColorizeResult"+toString(run), R8G8B8X8, 0, 0);
        }
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
        // This has the effect of printing out all the brightness differences for
        //different kernel sizes.
        pBmp = loadTestBmp("spike");
        for (double stddev = 0.5; stddev < 5; stddev += 0.25) {
            pDestBmp = GPUBlurFilter(pBmp->getSize(), pBmp->getPixelFormat(), stddev)
                    .apply(pBmp);
            testEqualBrightness(*pDestBmp, *pBmp, 1);
        }
*/
        pBmp = loadTestBmp("spike");
        GPUBlurFilter filter(pBmp->getSize(), pBmp->getPixelFormat(), R32G32B32A32F, 0.5,
                false);
        runImageTest(pBmp, filter, 0.5, "blur05_spike");
        runImageTest(pBmp, filter, 1, "blur1_spike");
        runImageTest(pBmp, filter, 3, "blur3_spike");

        pBmp = loadTestBmp("flat");
        filter = GPUBlurFilter(pBmp->getSize(), pBmp->getPixelFormat(), R32G32B32A32F, 5,
                false);
        runImageTest(pBmp, filter, 5, "blur05_flat", true);

        runImageTest("rgb24-64x64");
        runImageTest("rgb24alpha-64x64");
    }

private:
    void runImageTest(const string& sFName)
    {
        BitmapPtr pBmp = loadTestBmp(sFName);
        GPUBlurFilter filter(pBmp->getSize(), pBmp->getPixelFormat(), R32G32B32A32F, 2,
                false);
        runImageTest(pBmp, filter, 2, string("blur_")+sFName, true);
    }

    void runImageTest(BitmapPtr pBmp, GPUBlurFilter& filter, double stdDev, 
            string sBmpName, bool bIgnoreBrightness = false)
    {
        cerr << "    Testing " << sBmpName << ", stddev " << stdDev << endl;
        filter.setStdDev(stdDev);
        BitmapPtr pDestBmp = filter.apply(pBmp);
        if (!bIgnoreBrightness) {
            testEqualBrightness(*pDestBmp, *pBmp, 0.03);
        }
        testEqual(*pDestBmp, sBmpName, B8G8R8X8, 0.01, 0.1);
    }
};

class BandpassFilterTest: public GraphicsTest {
public:
    BandpassFilterTest()
        : GraphicsTest("BandpassFilterTest", 2)
    {
    }

    void runTests() 
    {
        runImageTests("spike", B8G8R8X8);
        runImageTests("i8-64x64", I8);
    }

private:
    void runImageTests(const string& sFName, PixelFormat pf)
    {
        cerr << "    Testing " << sFName << endl;
        BitmapPtr pBmp = loadTestBmp(sFName, pf);
        GPUBandpassFilter f(pBmp->getSize(), pf, 0.5, 1.5, 1, false);
        BitmapPtr pDestBmp = f.apply(pBmp);
        TEST(fabs(pDestBmp->getAvg() -128) < 0.06);
        testEqual(*pDestBmp, "bandpass_"+sFName, pf, 0.2, 0.5);
        TEST(pDestBmp->getPixelFormat() == pf);
    }
};

class RGB2YUVFilterTest: public GraphicsTest {
public:
    RGB2YUVFilterTest()
        : GraphicsTest("RGB2YUVFilterTest", 2)
    {
    }

    void runTests() 
    {
        BitmapPtr pOrigBmp = loadTestBmp("rgb24-64x64");
        GLTexturePtr pTex = GLTexturePtr(new GLTexture(pOrigBmp->getSize(), 
                pOrigBmp->getPixelFormat()));
        pTex->moveBmpToTexture(pOrigBmp);
        GPURGB2YUVFilter f(pOrigBmp->getSize());
        f.apply(pTex);
        vector<BitmapPtr> pResultBmps = f.getResults();
//        pResultBmps[0]->save("foo.png");
//        pResultBmps[1]->save("fooU.png");
//        pResultBmps[2]->save("fooV.png");
        BitmapPtr pDestBmp(new Bitmap(pOrigBmp->getSize(), B8G8R8X8));
        pDestBmp->copyYUVPixels(*pResultBmps[0], *pResultBmps[1], *pResultBmps[2], true);
//        pDestBmp->save("fooDest.png");
        testEqual(*pDestBmp, *pOrigBmp, "gpu", 5, 10);
        
    }
};


class GPUTestSuite: public TestSuite {
public:
    GPUTestSuite() 
        : TestSuite("GPUTestSuite")
    {
        addTest(TestPtr(new TextureMoverTest));
        addTest(TestPtr(new BrightnessFilterTest));
        addTest(TestPtr(new RGB2YUVFilterTest));
        if (GLTexture::isFloatFormatSupported()) {
            addTest(TestPtr(new ChromaKeyFilterTest));
            addTest(TestPtr(new HslColorFilterTest));
            addTest(TestPtr(new BlurFilterTest));
            addTest(TestPtr(new BandpassFilterTest));
        } else {
            cerr << "Skipping some GPU tests since float textures are not supported by "
                    << endl << "the OpenGL configuration." << endl;
        }
    }
};


int main(int nargs, char** args)
{
    bool bOK = true;
    try {
        OGLImagingContext context;
        try {
            if (!queryOGLExtension("GL_ARB_fragment_shader")) {
                throw Exception(AVG_ERR_UNSUPPORTED, 
                        "Fragment shaders not supported on this Machine. ");
            }
            GPUTestSuite suite;
            suite.runTests();
            bOK = suite.isOk();
        } catch (Exception& ex) {
            cerr << "Exception: " << ex.getStr() << endl;
        }
    } catch (Exception& ex) {
        if (ex.getCode() == AVG_ERR_ASSERT_FAILED) {
            cerr << ex.getStr() << endl;
            bOK = false;
        } else {
            cerr << "Skipping GPU imaging test." << endl;
            cerr << "Reason: " << ex.getStr() << endl;
            bOK = true;
        }
    }

    if (bOK) {
        return 0;
    } else {
        return 1;
    }
}

