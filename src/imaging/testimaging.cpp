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

#include "FakeCamera.h"
#include "TrackerThread.h"
#include "TrackerConfig.h"
#include "FilterDistortion.h"
#include "DeDistort.h"
#include "../graphics/Filtergrayscale.h"
#include "../graphics/FilterId.h"

#include "../base/TestSuite.h"
#include "../base/Exception.h"

#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <sstream>

using namespace avg;
using namespace std;

class DeDistortTest: public Test {
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
                Params, 0, M_PI/2,
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

class SerializeTest: public Test {
public:
    SerializeTest()
        : Test("SerializeTest", 2)
    {
    }

    void runTests() 
    {
        vector<double> Params;
        Params.push_back(0);
        Params.push_back(0);
        DeDistortPtr pScaler = DeDistortPtr(new DeDistort(DPoint(1,1),
                Params, 0, 0.0,
                DPoint(0,0), DPoint(2,2)));
        TrackerConfig Config;
        Config.m_pTrafo = pScaler;
        Config.save("temptrackerrc");
        TrackerConfig LoadedConfig;
        LoadedConfig.load("temptrackerrc");
        DeDistortPtr pTrafo = LoadedConfig.m_pTrafo;
        TEST(almostEqual(pTrafo->transform_point(DPoint(0,0)), DPoint(0,0)));
        TEST(almostEqual(pTrafo->transformBlobToScreen(DPoint(1,2)), DPoint(2,4)));
    }
};



class TrackingTest: public Test, public IBlobTarget {
public:
    TrackingTest()
        : Test("TrackingTest", 2)
    {
    }

    void runTests() 
    {
        std::vector<std::string> p = std::vector<std::string>();
        for (int i=0; i<6; ++i) {
            stringstream s;
            s << "../imaging/testimages/Blob" << i << ".png";
            p.push_back(s.str());
        }
        CameraPtr pCam = CameraPtr(new FakeCamera(p));
        for (int i=0; i<NUM_TRACKER_IMAGES; i++) {
            m_pBitmaps[i] = BitmapPtr(new Bitmap(pCam->getImgSize(), I8));
        }
        MutexPtr pMutex(new boost::mutex);
        TrackerConfig config;
        m_pCmdQ = TrackerThread::CmdQueuePtr(new TrackerThread::CmdQueue);
        boost::thread Thread(
                TrackerThread(pCam, m_pBitmaps, pMutex,  *m_pCmdQ, this, true, config));
        Thread.join();
    }
    
    virtual void update(BlobListPtr pBlobs, BitmapPtr) {
        static int BmpIndex = 0;
        switch(BmpIndex) {
            case 0:
                TEST(pBlobs->size() == 0);
                break;
            case 1:
                {
                    TEST(pBlobs->size() == 1);
                    BlobInfoPtr pBlobInfo = (*pBlobs->begin())->getInfo();
                    TEST(fabs(pBlobInfo->m_Orientation)<0.001);
                }
                break;
            case 2:
//                TEST(pBlobs->size() == 2);
                break;
            case 3:
                break;
            case 4:
                {
                    TEST(pBlobs->size() == 1);
                    BlobInfoPtr pBlobInfo = (*pBlobs->begin())->getInfo();
                    TEST(fabs(pBlobInfo->m_Area-114)<0.001);
                }
                break;
            case 5:
                TEST(pBlobs->size() == 0);
                m_pCmdQ->push(Command<TrackerThread>(boost::bind(&TrackerThread::stop, _1)));
                break;
            default:
                break;
        }
        BmpIndex++;
    }

private:
    TrackerThread::CmdQueuePtr m_pCmdQ;
    BitmapPtr m_pBitmaps[NUM_TRACKER_IMAGES];
};

class ImagingTestSuite: public TestSuite {
public:
    ImagingTestSuite() 
        : TestSuite("ImagingTestSuite")
    {
        addTest(TestPtr(new TrackingTest));
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

