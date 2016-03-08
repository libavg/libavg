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

#include "../graphics/Bitmap.h"
#include "../graphics/BitmapLoader.h"
#include "../graphics/Filterfliprgb.h"
#include "../graphics/Filtergrayscale.h"

#include "../base/Directory.h"
#include "../base/Exception.h"

#include <iostream>
#include <sstream>
#include <math.h>

namespace avg {

using namespace avg;
using namespace std;

GraphicsTest::GraphicsTest(const string& sName, int indentLevel)
        : Test(sName, indentLevel)
{
}

void GraphicsTest::createResultImgDir()
{
    Directory dir("resultimages");
    int ok = dir.open(true);
    if (ok == 0) {
        dir.empty();
    } else {
        stringstream s;
        s << "Could not create result image dir '" << dir.getName() << "'.";
        cerr << s.str() << endl;
        throw Exception(AVG_ERR_VIDEO_GENERAL, s.str());
    }
}

std::string GraphicsTest::getTestBmpName(const string& sFName)
{
    return getSrcDirName()+"media/"+sFName+".png";
}

BitmapPtr GraphicsTest::loadTestBmp(const string& sFName, PixelFormat pf)
{
    try {
        return loadBitmap(getTestBmpName(sFName), pf);
    } catch (Exception & ex) {
        cerr << ex.getStr() << endl;
        throw;
    }
}

void GraphicsTest::testEqual(Bitmap& resultBmp, const string& sFName, PixelFormat pf,
        float maxAverage, float maxStdDev)
{
    BitmapPtr pBaselineBmp;
    try {
        string sFullName = getSrcDirName()+"baseline/"+sFName+".png";
        pBaselineBmp = loadBitmap(sFullName, pf);
    } catch (Exception & ex) {
        cerr << ex.getStr() << endl;
        resultBmp.save("resultimages/"+sFName+".png");
        throw;
    }
    testEqual(resultBmp, *pBaselineBmp, sFName, maxAverage, maxStdDev);
}

void GraphicsTest::testEqual(Bitmap& resultBmp, Bitmap& baselineBmp,
        const string& sFName, float maxAverage, float maxStdDev)
{
    BitmapPtr pDiffBmp;
    try {
        pDiffBmp = resultBmp.subtract(baselineBmp);
    } catch (Exception& e) {
        TEST_FAILED("Error: " << e.getStr() << ". File: '" << sFName << "'.");
        string sResultName = "resultimages/"+sFName;
        resultBmp.save(sResultName+".png");
        baselineBmp.save(sResultName+"_baseline.png");
    }
    if (pDiffBmp) {
        float average = pDiffBmp->getAvg();
        float stdDev = pDiffBmp->getStdDev();
        if (average > maxAverage || stdDev > maxStdDev) {
            TEST_FAILED("Error: Decoded image differs from baseline '" <<
                    sFName << "'. average=" << average << ", stdDev=" << stdDev);
    //        resultBmp.dump();
    //        baselineBmp.dump();
            string sResultName = "resultimages/"+sFName;
            resultBmp.save(sResultName+".png");
            baselineBmp.save(sResultName+"_baseline.png");
            BitmapPtr pDiffBmp = resultBmp.subtract(baselineBmp);
            pDiffBmp->save(sResultName+"_diff.png");
        }
    }
}

void GraphicsTest::testEqualBrightness(Bitmap& resultBmp, Bitmap& baselineBmp,
        float epsilon)
{
    float diff = fabs(resultBmp.getAvg()-baselineBmp.getAvg());
    if (diff >= epsilon) {
        TEST_FAILED("Error: Baseline brightness: " << baselineBmp.getAvg()
                << ", Result brightness: " << resultBmp.getAvg() << ", difference: "
                << diff);
    }
}

int GraphicsTest::sumPixels(Bitmap& bmp)
{
    AVG_ASSERT(bmp.getBytesPerPixel() == 4);
    int sum = 0;
    IntPoint size = bmp.getSize();
    for (int y = 0; y < size.y; y++) {
        unsigned char * pLine = bmp.getPixels()+y*bmp.getStride();
        for (int x = 0; x < size.x; x++) {
            sum += pLine[x*4];
            sum += pLine[x*4+1];
            sum += pLine[x*4+2];
        }
    }
    return sum;
}

};

