//
//  libavg - Media Playback Engine.
//  Copyright (C) 2003-2014 Ulrich von Zadow
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

#include "../graphics/GLTexture.h"
#include "../graphics/BitmapLoader.h"
#include "../graphics/GPUBrightnessFilter.h"
#include "../graphics/GPUBlurFilter.h"
#include "../graphics/GPUBandpassFilter.h"
#include "../graphics/GPUChromaKeyFilter.h"
#include "../graphics/GPUHueSatFilter.h"
#include "../graphics/GPUInvertFilter.h"
#include "../graphics/GPURGB2YUVFilter.h"
#include "../graphics/FilterResizeBilinear.h"
#include "../graphics/GLContext.h"
#include "../graphics/GLContextManager.h"
#include "../graphics/ShaderRegistry.h"
#include "../graphics/BmpTextureMover.h"
#include "../graphics/PBO.h"
#include "../graphics/ImageCache.h"
#include "../graphics/CachedImage.h"

#include "../base/TestSuite.h"
#include "../base/Exception.h"
#include "../base/Test.h"
#include "../base/StringHelper.h"
#include "../base/FileHelper.h"

#include <math.h>
#include <iostream>

#include <glib-object.h>

using namespace avg;
using namespace std;


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
        testEqual(*pDestBmp, "graphics/bandpass_"+sFName, pf, 0.2, 0.5);
        TEST(pDestBmp->getPixelFormat() == pf);
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
/*
        // This has the effect of printing out all the brightness differences for
        //different kernel sizes.
        BitmapPtr pDestBmp;
        pBmp = loadTestBmp("spike");
        for (float stddev = 0.5f; stddev < 5; stddev += 0.25f) {
            pDestBmp = GPUBlurFilter(pBmp->getSize(), pBmp->getPixelFormat(), stddev)
                    .apply(pBmp);
            testEqualBrightness(*pDestBmp, *pBmp, 1);
        }
*/
        runFilterTests(false);
        if (GLTexture::isFloatFormatSupported()) {
            runFilterTests(true);
        }
    }

private:
    void runFilterTests(bool bUseFloat)
    {
        BitmapPtr pBmp;
        GPUBlurFilterPtr pFilter;
        pBmp = loadTestBmp("spike");
        PixelFormat destPF;
        if (bUseFloat) {
            destPF = R32G32B32A32F;
        } else {
            destPF = B8G8R8A8;
        }
        pFilter = GPUBlurFilterPtr(new GPUBlurFilter(pBmp->getSize(),
                pBmp->getPixelFormat(), destPF, 0.5f, false));
        runImageTest(pBmp, pFilter, 0.5f, "graphics/blur05_spike");
        runImageTest(pBmp, pFilter, 1, "graphics/blur1_spike");
        runImageTest(pBmp, pFilter, 3, "graphics/blur3_spike");

        pBmp = loadTestBmp("flat");
        pFilter = GPUBlurFilterPtr(new GPUBlurFilter(pBmp->getSize(),
                pBmp->getPixelFormat(), destPF, 5, false));
        runImageTest(pBmp, pFilter, 5, "graphics/blur05_flat", true);

        runImageTest("rgb24-64x64", destPF);
        runImageTest("rgb24alpha-64x64", destPF);

    }

    void runImageTest(const string& sFName, PixelFormat destPF)
    {
        BitmapPtr pBmp = loadTestBmp(sFName);
        GPUBlurFilterPtr pFilter(new GPUBlurFilter(pBmp->getSize(),
                pBmp->getPixelFormat(), destPF, 2, false));
        runImageTest(pBmp, pFilter, 2, string("graphics/blur_")+sFName, true);
    }

    void runImageTest(BitmapPtr pBmp, GPUBlurFilterPtr pFilter, float stdDev,
            string sBmpName, bool bIgnoreBrightness = false)
    {
        cerr << "    Testing " << sBmpName << ", stddev " << stdDev << endl;
        pFilter->setStdDev(stdDev);
        BitmapPtr pDestBmp = pFilter->apply(pBmp);
        if (!bIgnoreBrightness) {
            testEqualBrightness(*pDestBmp, *pBmp, 0.03);
        }
        testEqual(*pDestBmp, sBmpName, pDestBmp->getPixelFormat(), 0.1, 0.3);
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
        PixelFormat pf = pBmp->getPixelFormat();
        cerr << "      Source Bmp: " << pf << endl;
        BitmapPtr pDestBmp;
        pDestBmp = GPUBrightnessFilter(pBmp->getSize(), pixelFormatHasAlpha(pf), 1)
                .apply(pBmp);
        float maxAverage, maxStdDev;
        if (GLContext::getCurrent()->isGLES()) {
            // less strict (lower floating point precision?)
            maxAverage = 0.5;
            maxStdDev = 1.5;
        }
        else {
            maxAverage = 0.2;
            maxStdDev = 0.5;
        }
        testEqual(*pDestBmp, *pBmp, string("brightness_")+sFName, maxAverage, maxStdDev);
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
        GPUChromaKeyFilter filter(pBmp->getSize());
        for (int erosion = 0; erosion < 3; ++erosion) {
            filter.setParams(Pixel32(0,255,0), 0.1, 0.2, 0.1, 0.1, erosion, 0);
            pDestBmp = filter.apply(pBmp);
            testEqual(*pDestBmp, "graphics/ChromaKeyResult"+toString(erosion), B8G8R8A8, 0.3,
                    0.7);
        }
        filter.setParams(Pixel32(0,255,0), 0.0, 0.0, 0.0, 0.0, 0, 0.1);
        pDestBmp = filter.apply(pBmp);
        testEqual(*pDestBmp, "graphics/ChromaKeySpillResult1", B8G8R8A8, 0.3, 0.7);
        filter.setParams(Pixel32(0,255,0), 0.1, 0.1, 0.1, 0.0, 0, 0.1);
        pDestBmp = filter.apply(pBmp);
        testEqual(*pDestBmp, "graphics/ChromaKeySpillResult2", B8G8R8A8, 0.3, 0.7);
        filter.setParams(Pixel32(0,255,0), 0.1, 0.1, 0.1, 0.0, 0, 0.2);
        pDestBmp = filter.apply(pBmp);
        testEqual(*pDestBmp, "graphics/ChromaKeySpillResult3", B8G8R8A8, 0.3, 0.7);

        pBmp = loadTestBmp("chromakey-median");
        filter.setParams(Pixel32(0,255,0), 0.1, 0.1, 0.1, 0.0, 0, 0.0);
        pDestBmp = filter.apply(pBmp);
        testEqual(*pDestBmp, "graphics/ChromaKeyMedianResult", B8G8R8A8, 1, 6);
    }
};


class HueSatFilterTest: public GraphicsTest {
public:
    HueSatFilterTest()
        : GraphicsTest("HueSatFilterTest", 2)
    {
    }

    void runTests()
    {
        BitmapPtr pBmp = loadTestBmp("hsl");
        BitmapPtr pDestBmp;
        GPUHueSatFilter filter(pBmp->getSize(), true);
        //Test hue functionality
        for (int run = 0; run < 3; run++) {
            filter.setParams(run*90);
            pDestBmp = filter.apply(pBmp);
            testEqual(*pDestBmp, "graphics/HslHueResult"+toString(run),
                    NO_PIXELFORMAT, 0, 0);
        }
        //Test colorize functionality
        for (int run = 0; run < 3; run++) {
            filter.setParams(run*90, 1, 0, true);
            pDestBmp = filter.apply(pBmp);
            testEqual(*pDestBmp, "graphics/HslColorizeResult"+toString(run),
                    NO_PIXELFORMAT, 0, 0);
        }
    }
};


class InvertFilterTest: public GraphicsTest {
public:
    InvertFilterTest()
        : GraphicsTest("InvertFilterTest", 2)
    {
    }

    void runTests()
    {
        runImageTests("rgb24-64x64");
    }

private:
    void runImageTests(const string& sFName)
    {
        cerr << "    Testing " << sFName << endl;
        BitmapPtr pBmp = loadTestBmp(sFName);
        BitmapPtr pDestBmp;
        pDestBmp = GPUInvertFilter(pBmp->getSize(), false).apply(pBmp);
        float maxAverage, maxStdDev;
        if (GLContext::getCurrent()->isGLES()) {
            // less strict (lower floating point precision?)
            maxAverage = 0.6;
            maxStdDev = 2.0;
        }
        else {
            maxAverage = 0.0;
            maxStdDev = 0.0;
        }
        testEqual(*pDestBmp, string("graphics/invert_")+sFName,
                pBmp->getPixelFormat(), maxAverage, maxStdDev);
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
        GLContextManager* pCM = GLContextManager::get();
        MCTexturePtr pTex = pCM->createTextureFromBmp(pOrigBmp);
        pCM->uploadData();
        GPURGB2YUVFilter f(pOrigBmp->getSize());
        GLContext* pContext = GLContext::getCurrent();
        f.apply(pContext, pTex->getTex(pContext));
        BitmapPtr pResultBmp = f.getResults(pContext);
        pResultBmp = convertYUVX444ToRGB(pResultBmp);
        testEqual(*pResultBmp, *pOrigBmp, "RGB2YUV", 1, 2);
    }

    BitmapPtr convertYUVX444ToRGB(const BitmapPtr& pYUVBmp)
    {
        // This is a weird pixel format that's not used anywhere else, so support
        // hasn't been moved to the Bitmap class.
        BitmapPtr pRGBBmp(new Bitmap(pYUVBmp->getSize(), B8G8R8X8));
        int height = pRGBBmp->getSize().y;
        int width = pRGBBmp->getSize().y;
        int strideInPixels = pRGBBmp->getStride()/4;

        for (int y = 0; y < height; ++y) {
            const unsigned char * pSrc = pYUVBmp->getPixels() + pYUVBmp->getStride()*y;
            Pixel32 * pDest = (Pixel32*)pRGBBmp->getPixels() + strideInPixels*y;
            for (int x = 0; x < width; x++) {
                YUVJtoBGR32Pixel(pDest, pSrc[0], pSrc[1], pSrc[2]);
                pSrc += 4;
                pDest ++;
            }
        }
        return pRGBBmp;
    }
};


class TextureMoverTest: public GraphicsTest {
public:
    TextureMoverTest()
        : GraphicsTest("TextureMoverTest", 2)
    {
    }

    void runTests()
    {
        for (int i=1; i<2; ++i) {
            bool bPOT = (i==1);
            if (GLContext::getCurrent()->arePBOsSupported()) {
                runImageTest(bPOT, MM_PBO, "rgb24-65x65");
                runImageTest(bPOT, MM_PBO, "rgb24alpha-64x64");
            }
            runImageTest(bPOT, MM_OGL, "rgb24-65x65");
            runImageTest(bPOT, MM_OGL, "rgb24alpha-64x64");
        }
        runCompressionTest(MM_OGL, "rgb24-65x65");
        if (GLContext::getCurrent()->arePBOsSupported()) {
            runCompressionTest(MM_PBO, "rgb24-65x65");
            runMipmapTest(MM_PBO, "rgb24alpha-64x64");
            runMipmapTest(MM_PBO, "rgb24-65x65");
        }
        runMipmapTest(MM_OGL, "rgb24-64x64");
        runMipmapTest(MM_OGL, "rgb24alpha-64x64");
        runMipmapTest(MM_OGL, "rgb24-65x65");
    }

private:
    void runImageTest(bool bPOT, OGLMemoryMode memoryMode, const string& sFName)
    {
        string sResultFName = sFName + "-" + oglMemoryMode2String(memoryMode) + "-";
        if (bPOT) {
            sResultFName += "pot";
        } else {
            sResultFName += "npot";
        }
        cerr << "    Testing " << sResultFName << endl;
        BitmapPtr pOrigBmp = loadTestBmp(sFName);
        {
            GLContextManager* pCM = GLContextManager::get();
            MCTexturePtr pMCTex = pCM->createTextureFromBmp(pOrigBmp, false, bPOT, 0);
            pCM->uploadData();
            BitmapPtr pDestBmp = pMCTex->getTex(GLContext::getCurrent())->
                    moveTextureToBmp();
            testEqual(*pDestBmp, *pOrigBmp, sResultFName+"-move", 0.01, 0.1);
        }
    }

    void runMipmapTest(OGLMemoryMode memoryMode, const string& sFName)
    {
        cerr << "    Testing mipmap support, " << sFName << ", " <<
                oglMemoryMode2String(memoryMode) << endl;
        BitmapPtr pOrigBmp = loadTestBmp(sFName);
        GLContextManager* pCM = GLContextManager::get();
        MCTexturePtr pMCTex = pCM->createTextureFromBmp(pOrigBmp, true);
        pCM->uploadData();
        GLTexturePtr pTex = pMCTex->getTex(GLContext::getCurrent());
        pTex->generateMipmaps();

        if (GLContext::getCurrent()->isGLES()) {
            // GLES doesn't support attaching texture mipmap levels other than 0 to
            // FBOs, so moveTextureToBmp() will fail. Skip result image comparison.
            return;
        }
        BitmapPtr pResultBmp = pTex->moveTextureToBmp(1);
        IntPoint newSize(pOrigBmp->getSize()/2);
        TEST(pResultBmp->getSize() == newSize);
        FilterResizeBilinear resizer(newSize);
        BitmapPtr pBaselineBmp = resizer.apply(pOrigBmp);
        string sName;
        if (memoryMode == MM_PBO) {
            sName = "pbo-mipmap";
        } else {
            sName = "ogl-mipmap";
        }
        testEqual(*pResultBmp, *pBaselineBmp, sName, 7, 15);
    }

    void runCompressionTest(OGLMemoryMode memoryMode, const string& sFName)
    {
        cerr << "    Testing B5G6R5 compression, " << sFName << ", " <<
                oglMemoryMode2String(memoryMode) << endl;
        BitmapPtr pFileBmp = loadTestBmp(sFName);
        BitmapPtr pOrigBmp(new Bitmap(pFileBmp->getSize(), B5G6R5));
        pOrigBmp->copyPixels(*pFileBmp);
        GLContextManager* pCM = GLContextManager::get();
        MCTexturePtr pMCTex = pCM->createTextureFromBmp(pOrigBmp);
        pCM->uploadData();

        BitmapPtr pDestBmp = pMCTex->getTex(GLContext::getCurrent())->moveTextureToBmp();
    }
};


class ImageCacheTest: public GraphicsTest {
public:
    ImageCacheTest()
        : GraphicsTest("ImageCacheTest", 2)
    {
    }

    void runTests()
    {
        ImageCache* pCache = ImageCache::get();
        cerr << "    Testing no cache" << endl;
        pCache->setCapacity(0, 0);
        loadImages();
        TEST(pCache->getNumCPUImages() == 0);
        TEST(pCache->getNumGPUImages() == 0);
        cerr << "    Testing CPU cache" << endl;
        pCache->setCapacity(20000, 0);
        loadImages();
        TEST(pCache->getNumCPUImages() == 1);
        TEST(pCache->getNumGPUImages() == 0);
        cerr << "    Testing GPU cache" << endl;
        if (GLContext::getCurrent()->isGLES()) {
            // GLES size is larger because of POT textures.
            pCache->setCapacity(20000, 80000);
        } else {
            pCache->setCapacity(20000, 20000);
        }
        loadImages();
        TEST(pCache->getNumCPUImages() == 1);
        TEST(pCache->getNumGPUImages() == 1);
        pCache->setCapacity(0, 0);
        TEST(pCache->getNumCPUImages() == 0);
        TEST(pCache->getNumGPUImages() == 0);
    }

private:
    void loadImages()
    {
        GLContextManager* pCM = GLContextManager::get();
        ImageCache* pCache = ImageCache::get();
        CachedImagePtr pImage1a = pCache->getImage(getTestBmpName("rgb24-65x65"),
                TEXCOMPRESSION_NONE);
        TEST(pCache->getNumCPUImages() == 1);
        CachedImagePtr pImage1b = pCache->getImage(getTestBmpName("rgb24-65x65"),
                TEXCOMPRESSION_NONE);
        TEST(pCache->getNumCPUImages() == 1);
        BitmapPtr pFileBmp = loadTestBmp("rgb24-65x65");
        BitmapPtr pBmp = pImage1b->getBmp();
        testEqual(*pBmp, *pFileBmp, "rgb24-65x65");
        CachedImagePtr pImage2a = pCache->getImage(getTestBmpName("rgb24-64x64"),
                TEXCOMPRESSION_B5G6R5);
        TEST(pCache->getNumCPUImages() == 2);
        CachedImagePtr pImage2b = pCache->getImage(getTestBmpName("rgb24-64x64"),
                TEXCOMPRESSION_NONE);

        pImage2b->decBmpRef();
        pImage2a->decBmpRef();

        pImage1a->incTexRef(false);
        pImage1a->incTexRef(true);
        pCM->uploadData();
        pImage1b->decTexRef();
        pImage1b->decBmpRef();
        pImage1a->decTexRef();
        pImage1a->decBmpRef();
        pCM->uploadData();
    }
};


class GPUTestSuite: public TestSuite {
public:
    GPUTestSuite(const string& sVariant, const string& sSrcDir)
        : TestSuite("GPUTestSuite ("+sVariant+")", sSrcDir)
    {
        addTest(TestPtr(new TextureMoverTest));
        addTest(TestPtr(new ImageCacheTest));
        addTest(TestPtr(new BrightnessFilterTest));
        addTest(TestPtr(new HueSatFilterTest));
        addTest(TestPtr(new InvertFilterTest));
        if (GLContext::getCurrent()->getShaderUsage() == GLConfig::FULL) {
            addTest(TestPtr(new RGB2YUVFilterTest));
            addTest(TestPtr(new ChromaKeyFilterTest));
            addTest(TestPtr(new BlurFilterTest));
            if (GLTexture::isFloatFormatSupported()) {
                addTest(TestPtr(new BandpassFilterTest));
            }
        }
    }
};


bool runTests(bool bGLES, GLConfig::ShaderUsage su, const string& sSrcDir)
{
    GLContextManager cm;
    if (fileExists(sSrcDir+"/../graphics/shaders")) {
        ShaderRegistry::setShaderPath(sSrcDir+"/../graphics/shaders");
    } else {
        // TODO: Is this still needed
        ShaderRegistry::setShaderPath("../shaders");
    }
    GLContext* pContext = cm.createContext(GLConfig(bGLES, false, true, 1, su, true));
    string sVariant = string("GLES: ") + toString(bGLES) + ", ShaderUsage: " +
            GLConfig::shaderUsageToString(pContext->getShaderUsage());
    cerr << "---------------------------------------------------" << endl;
    cerr << sVariant << endl;
    cerr << "---------------------------------------------------" << endl;
    pContext->enableErrorChecks(true);
    glDisable(GL_BLEND);
    GLContext::checkError("glDisable(GL_BLEND)");
    try {
        GPUTestSuite suite(sVariant, sSrcDir);
        suite.runTests();
        delete pContext;
        return suite.isOk();
    } catch (Exception& ex) {
        cerr << "Exception: " << ex.getStr() << endl;
        delete pContext;
        return false;
    }
}


int main(int nargs, char** args)
{
    assert(nargs == 2);
    bool bOK = true;
    try {
#ifndef AVG_ENABLE_EGL
        BitmapLoader::init(true);
        bOK = runTests(false, GLConfig::AUTO, args[1]);
        bOK &= runTests(false, GLConfig::MINIMAL, args[1]);
#endif
        if (GLContextManager::isGLESSupported()) {
            BitmapLoader::init(false);
            bOK &= runTests(true, GLConfig::MINIMAL, args[1]);
        } else {
            cerr << "Skipping GLES test because GLES isn't supported on this machine."
                    << endl;
        }
    } catch (Exception& ex) {
        cerr << ex.getStr() << endl;
        bOK = false;
    }

    if (bOK) {
        cerr << "testgpu succeeded" << endl;
        return 0;
    } else {
        cerr << "testgpu failed" << endl;
        return 1;
    }
}

