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
#include "Bitmap.h"
#include "Filterfliprgb.h"
#include "Filtergrayscale.h"

#include "../base/Directory.h"

#include <Magick++.h>

#include <iostream>

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
        // TODO: Disable saving of test images.
        cerr << "GraphicsTest: Could not create result image directory." << endl;
    }
}

BitmapPtr GraphicsTest::loadTestBmp(const std::string& sFName, PixelFormat pf)
{
   BitmapPtr pBmp(new Bitmap(string("testfiles/")+sFName+".png"));
   if (pf == I8) {
       return FilterGrayscale().apply(pBmp);
   } else {
       return FilterFlipRGB().apply(pBmp);
  }
}

void GraphicsTest::testEqual(Bitmap& ResultBmp, const string& sFName) 
{
    BitmapPtr pBaselineBmp;
    try {
        pBaselineBmp = BitmapPtr(new Bitmap(string("baseline/")+sFName+".png"));
        FilterFlipRGB().applyInPlace(pBaselineBmp);
    } catch (Magick::Exception & ex) {
        cerr << ex.what() << endl;
        ResultBmp.save(string("resultimages/")+sFName+".png");
        throw;
    }
    testEqual(ResultBmp, *pBaselineBmp, sFName);
}

void GraphicsTest::testEqual(Bitmap& ResultBmp, Bitmap& BaselineBmp,
        const string& sFName)
{
    TEST(ResultBmp == BaselineBmp);
    if (!(ResultBmp == BaselineBmp)) {
        string sResultName = string("resultimages/")+sFName;
        cerr << "Saving result image to " << sResultName << endl;
        ResultBmp.save(sResultName+".png");
        BaselineBmp.save(sResultName+"_expected.png");
        Bitmap DiffBmp(ResultBmp);
        DiffBmp.subtract(&BaselineBmp);
        DiffBmp.save(sResultName+"_diff.png");
    }
}

void GraphicsTest::testEqualBrightness(Bitmap& ResultBmp, Bitmap& BaselineBmp, int epsilon)
{
    int ResultBrightness = sumPixels(ResultBmp);
    int BaselineBrightness = sumPixels(BaselineBmp);
    TEST(abs(ResultBrightness - BaselineBrightness) < epsilon);
    if (abs(ResultBrightness - BaselineBrightness) >= epsilon) {
        cerr << "Baseline brightness: " << BaselineBrightness << ", Result brightness: " 
                << ResultBrightness << endl;
    }
}

int GraphicsTest::sumPixels(Bitmap& Bmp)
{
    assert(Bmp.getBytesPerPixel() == 4);
    int sum = 0;
    IntPoint size = Bmp.getSize();
    for (int y = 0; y < size.y; y++) {
        unsigned char * pLine = Bmp.getPixels()+y*Bmp.getStride();
        for (int x = 0; x < size.x; x++) { 
            sum += pLine[x*4];
            sum += pLine[x*4+1];
            sum += pLine[x*4+2];
        }
    }
    return sum;
}

};

