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

BitmapPtr GraphicsTest::loadTestBmp(const std::string& sFName)
{
   return BitmapPtr(new Bitmap(string("testfiles/")+sFName+".png"));
}

void GraphicsTest::testEqual(Bitmap& ResultBmp, const string& sFName) 
{
    BitmapPtr BaselineBmp;
    try {
        BaselineBmp = BitmapPtr(new Bitmap(string("baseline/")+sFName));
    } catch (Magick::Exception & ex) {
        cerr << ex.what() << endl;
        TEST(false);
        ResultBmp.save(string("resultimages/")+sFName+"_result.png");
    }
    testEqual(ResultBmp, *BaselineBmp, sFName);
}

void GraphicsTest::testEqual(Bitmap& ResultBmp, Bitmap& BaselineBmp,
        const string& sFName)
{
    TEST(ResultBmp == BaselineBmp);
    if (!(ResultBmp == BaselineBmp)) {
        string sResultName = string("resultimages/")+sFName;
        cerr << "Saving result image to " << sResultName << endl;
        ResultBmp.save(sResultName+"_result.png");
        BaselineBmp.save(sResultName+"_expected.png");
    }
}

};

