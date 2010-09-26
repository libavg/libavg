//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2008 Ulrich von Zadow
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

#include "FakeCamera.h"
#include "TrackerThread.h"
#include "TrackerConfig.h"
#include "DeDistort.h"
#include "FilterWipeBorder.h"
#include "FilterClearBorder.h"

#include "../graphics/GraphicsTest.h"
#include "../graphics/Filtergrayscale.h"
#include "../graphics/FilterId.h"

#include "../base/TestSuite.h"
#include "../base/Exception.h"
#include "../base/FileHelper.h"
#include "../base/MathHelper.h"

#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <sstream>

using namespace avg;
using namespace std;

class FilterWipeBorderTest: public GraphicsTest
{
public:
    FilterWipeBorderTest()
        : GraphicsTest("FilterWipeBorderTest", 2)
    {
    }

    void runTests()
    {
        BitmapPtr pBmp = loadTestBmp("filterwipeborder", I8);
        BitmapPtr pDestBmp = FilterWipeBorder(0).apply(pBmp);
        testEqual(*pDestBmp, *pBmp, "FilterWipeBorderResult0", 0, 0);
        pDestBmp = FilterWipeBorder(1).apply(pBmp);
        testEqual(*pDestBmp, "FilterWipeBorderResult1", I8);
        pDestBmp = FilterWipeBorder(3).apply(pBmp);
        testEqual(*pDestBmp, "FilterWipeBorderResult3", I8);
    }
};

class FilterClearBorderTest: public GraphicsTest
{
public:
    FilterClearBorderTest()
        : GraphicsTest("FilterClearBorderTest", 2)
    {
    }

    void runTests()
    {
        BitmapPtr pBmp = loadTestBmp("filterwipeborder", I8);
        BitmapPtr pDestBmp = FilterClearBorder(0).apply(pBmp);
        testEqual(*pDestBmp, *pBmp, "FilterClearBorderResult0", 0, 0);
        pDestBmp = FilterClearBorder(1).apply(pBmp);
        testEqual(*pDestBmp, "FilterClearBorderResult1", I8);
        pDestBmp = FilterClearBorder(3).apply(pBmp);
        testEqual(*pDestBmp, "FilterClearBorderResult3", I8);
    }
};

class DeDistortTest: public Test
{
public:
    DeDistortTest()
      : Test("DeDistortTest", 2)
    {}

    void runTests()
    {
        vector<double> params;
        params.push_back(0);
        params.push_back(0);
        DeDistort IdentityDistort = DeDistort(DPoint(1,1),
            params, 0.0, 0.0,
            DPoint(0,0), DPoint(1,1));
        TEST(almostEqual(IdentityDistort.transform_point(DPoint(0,0)), DPoint(0,0)));
        TEST(almostEqual(IdentityDistort.transform_point(DPoint(1,2)), DPoint(1,2)));
        TEST(almostEqual(IdentityDistort.transformBlobToScreen(DPoint(0,0)),
                DPoint(0,0)));
        TEST(almostEqual(IdentityDistort.transformBlobToScreen(DPoint(1,2)),
                DPoint(1,2)));
        TEST(almostEqual(IdentityDistort.inverse_transform_point(DPoint(0,0)), 
                DPoint(0,0)));
        TEST(almostEqual(IdentityDistort.inverse_transform_point(DPoint(1,2)), 
                DPoint(1,2)));
        TEST(almostEqual(IdentityDistort.transformScreenToBlob(DPoint(0,0)),
                DPoint(0,0)));
        TEST(almostEqual(IdentityDistort.transformScreenToBlob(DPoint(1,2)), 
                DPoint(1,2)));
        TEST(IdentityDistort.getDisplayArea(DPoint(1280,720)) == DRect(0,0,1280,720));

        DeDistort scaler = DeDistort(DPoint(1,1), params, 0, 0.0, DPoint(0,0), 
                DPoint(2,2));
        TEST(almostEqual(scaler.transform_point(DPoint(0,0)), DPoint(0,0)));
        TEST(almostEqual(scaler.transformBlobToScreen(DPoint(1,2)), DPoint(2,4)));
        TEST(almostEqual(scaler.inverse_transform_point(DPoint(0,0)), DPoint(0,0)));
        TEST(almostEqual(scaler.transformScreenToBlob(DPoint(1,2)), DPoint(0.5,1)));

        DeDistort shifter = DeDistort(DPoint(1,1), params, 0, 0.0, DPoint(1,1), 
                DPoint(1,1));
        TEST(almostEqual(shifter.transformBlobToScreen(DPoint(0,0)), DPoint(1,1)));
        TEST(almostEqual(shifter.transformBlobToScreen(DPoint(1,2)), DPoint(2,3)));
        TEST(almostEqual(shifter.transformScreenToBlob(DPoint(0,0)), DPoint(-1,-1)));
        TEST(almostEqual(shifter.transformScreenToBlob(DPoint(1,2)), DPoint(0,1)));
        TEST(shifter.getDisplayArea(DPoint(1,1)) == DRect(-1, -1, 0, 0));

        vector<double> cubed;
        cubed.push_back(0);
        cubed.push_back(1);
        DeDistort barreler = DeDistort(DPoint(1,1), cubed, 0, 0.0, DPoint(0,0), 
                DPoint(1,1));
        for (double xp = 0; xp < 10; xp++) {
            for(double yp = 0; yp < 10; yp++) {
                QUIET_TEST(almostEqual(barreler.inverse_transform_point(
                        barreler.transform_point(DPoint(xp,yp))), DPoint(xp,yp)));
            }
        }
        TEST(almostEqual(barreler.transform_point(DPoint(1,1)), DPoint(1,1)));

        DeDistort rotator = DeDistort(DPoint(1,1), params, 0, PI/2, DPoint(0,0),
                DPoint(1,1));
        for (double xp = 0; xp < 10; xp++) {
            for(double yp = 0; yp < 10; yp++) {
                QUIET_TEST(almostEqual(rotator.inverse_transform_point(
                        rotator.transform_point(DPoint(xp,yp))), DPoint(xp,yp)));
            }
        }

        DeDistort shifterScaler = DeDistort(DPoint(1,1), params, 0, 0.0, DPoint(1,1),
                DPoint(2,2));
        for (double xp = 0; xp < 10; xp++) {
            for(double yp = 0; yp < 10; yp++) {
                QUIET_TEST(almostEqual(shifterScaler.inverse_transform_point(
                        shifterScaler.transform_point(DPoint(xp,yp))), DPoint(xp,yp)));
            }
        }
    }
};

#ifdef _WIN32
#pragma warning(disable: 4996)
#endif
class SerializeTest: public Test
{
public:
    SerializeTest()
        : Test("SerializeTest", 2)
    {
    }

    void runTests() 
    {
        TrackerConfig Config;
        copyFile(getSrcDirName()+"avgtrackerrc.minimal", "avgtrackerrc");
        Config.load();
        
        Config.setParam("/transform/distortionparams/@p2", "0");
        Config.setParam("/transform/distortionparams/@p3", "0");
        Config.setParam("/transform/trapezoid/@value", "0");
        Config.setParam("/transform/angle/@value", "0");
        Config.setParam("/transform/displaydisplacement/@x", "0");
        Config.setParam("/transform/displaydisplacement/@y", "0");
        Config.setParam("/transform/displayscale/@x", "2");
        Config.setParam("/transform/displayscale/@y", "2");

        if (getSrcDirName() == "./") {
            Config.save();

            TrackerConfig loadedConfig;
            loadedConfig.load();
            DPoint scale = loadedConfig.getPointParam("/transform/displayscale/");
            TEST(almostEqual(scale, DPoint(2,2)));
            unlink("avgtrackerrc.bak");
        }
        unlink("avgtrackerrc");
    }
};

class ImagingTestSuite: public TestSuite
{
public:
    ImagingTestSuite() 
        : TestSuite("ImagingTestSuite")
    {
        addTest(TestPtr(new FilterWipeBorderTest));
        addTest(TestPtr(new FilterClearBorderTest));
        addTest(TestPtr(new DeDistortTest));
        addTest(TestPtr(new SerializeTest));
    }
};


int main(int nargs, char** args)
{
    ImagingTestSuite Suite;
    Suite.runTests();
    bool bOK = Suite.isOk();

    if (bOK) {
        return 0;
    } else {
        return 1;
    }
}

