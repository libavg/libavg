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
#include "FilterDistortion.h"
#include "DeDistort.h"
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

class DeDistortTest: public Test
{
public:
    DeDistortTest()
      : Test("DeDistortTest", 2)
    {}

    void runTests()
    {
        vector<double> Params;
        Params.push_back(0);
        Params.push_back(0);
        DeDistort IdentityDistort = DeDistort(DPoint(1,1),
            Params, 0.0, 0.0,
            DPoint(0,0), DPoint(1,1));
        TEST(almostEqual(IdentityDistort.transform_point(DPoint(0,0)), DPoint(0,0)));
        TEST(almostEqual(IdentityDistort.transform_point(DPoint(1,2)), DPoint(1,2)));
        TEST(almostEqual(IdentityDistort.transformBlobToScreen(DPoint(0,0)), DPoint(0,0)));
        TEST(almostEqual(IdentityDistort.transformBlobToScreen(DPoint(1,2)), DPoint(1,2)));
        TEST(almostEqual(IdentityDistort.inverse_transform_point(DPoint(0,0)), 
                DPoint(0,0)));
        TEST(almostEqual(IdentityDistort.inverse_transform_point(DPoint(1,2)), 
                DPoint(1,2)));
        TEST(almostEqual(IdentityDistort.transformScreenToBlob(DPoint(0,0)), DPoint(0,0)));
        TEST(almostEqual(IdentityDistort.transformScreenToBlob(DPoint(1,2)), DPoint(1,2)));
        TEST(IdentityDistort.getActiveBlobArea(DPoint(1280,720)) == DRect(0,0,1280,720));

        DeDistort Scaler = DeDistort(DPoint(1,1),
            Params, 0, 0.0,
            DPoint(0,0), DPoint(2,2));
        TEST(almostEqual(Scaler.transform_point(DPoint(0,0)), DPoint(0,0)));
        TEST(almostEqual(Scaler.transformBlobToScreen(DPoint(1,2)), DPoint(2,4)));
        TEST(almostEqual(Scaler.inverse_transform_point(DPoint(0,0)), DPoint(0,0)));
        TEST(almostEqual(Scaler.transformScreenToBlob(DPoint(1,2)), DPoint(0.5,1)));

        DeDistort Shifter = DeDistort(DPoint(1,1),
            Params, 0, 0.0,
            DPoint(1,1), DPoint(1,1));
        TEST(almostEqual(Shifter.transformBlobToScreen(DPoint(0,0)), DPoint(1,1)));
        TEST(almostEqual(Shifter.transformBlobToScreen(DPoint(1,2)), DPoint(2,3)));
        TEST(almostEqual(Shifter.transformScreenToBlob(DPoint(0,0)), DPoint(-1,-1)));
        TEST(almostEqual(Shifter.transformScreenToBlob(DPoint(1,2)), DPoint(0,1)));
        TEST(Shifter.getActiveBlobArea(DPoint(1,1)) == DRect(-1, -1, 0, 0));

        vector<double> Cubed;
        Cubed.push_back(0);
        Cubed.push_back(1);
        DeDistort Barreler = DeDistort(DPoint(1,1),
            Cubed, 0, 0.0,
            DPoint(0,0), DPoint(1,1));
        for (double xp=0;xp<10;xp++){
            for(double yp=0;yp<10;yp++){
                QUIET_TEST(almostEqual(Barreler.inverse_transform_point(
                        Barreler.transform_point(DPoint(xp,yp))), DPoint(xp,yp)));
            }
        }
        TEST(almostEqual(Barreler.transform_point(DPoint(1,1)), DPoint(1,1)));

        DeDistort Rotator = DeDistort(DPoint(1,1),
            Params, 0, PI/2,
            DPoint(0,0), DPoint(1,1));
        for (double xp=0;xp<10;xp++){
            for(double yp=0;yp<10;yp++){
                QUIET_TEST(almostEqual(Rotator.inverse_transform_point(
                        Rotator.transform_point(DPoint(xp,yp))), DPoint(xp,yp)));
            }
        }

        DeDistort ShifterScaler = DeDistort(DPoint(1,1),
            Params, 0, 0.0,
            DPoint(1,1), DPoint(2,2));
        for (double xp=0;xp<10;xp++){
            for(double yp=0;yp<10;yp++){
                QUIET_TEST(almostEqual(ShifterScaler.inverse_transform_point(
                        ShifterScaler.transform_point(DPoint(xp,yp))), DPoint(xp,yp)));
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

            TrackerConfig LoadedConfig;
            LoadedConfig.load();
            DPoint Scale = LoadedConfig.getPointParam("/transform/displayscale/");
            TEST(almostEqual(Scale, DPoint(2,2)));
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

