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

// #define GENERATE_BASELINE

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
            DeDistort IdentityDistort = DeDistort(DPoint(0,0), DPoint(1,1),
                Params, DPoint3(0,0,0), DPoint3(0,0,1), 0.0, 0.0,
                DPoint(0,0), DPoint(1,1));
            TEST(almostEqual(IdentityDistort.transform_point(DPoint(0,0)), DPoint(0,0)));
            TEST(almostEqual(IdentityDistort.transform_point(DPoint(1,2)), DPoint(1,2)));
//            cerr << IdentityDistort.inverse_transform_point(DPoint(0,0)) << endl;
//            cerr << IdentityDistort.transform_point(DPoint(1,2)) << endl;
            TEST(almostEqual(IdentityDistort.inverse_transform_point(DPoint(0,0)), 
                    DPoint(0,0)));
            TEST(almostEqual(IdentityDistort.inverse_transform_point(DPoint(1,2)), 
                    DPoint(1,2)));

            DeDistort Scaler = DeDistort(DPoint(0,0), DPoint(2,2),
                Params, DPoint3(0,0,0), DPoint3(0,0,1), 0, 0.0,
                DPoint(0,0), DPoint(1,1));
            TEST(almostEqual(Scaler.transform_point(DPoint(0,0)), DPoint(0,0)));
            TEST(almostEqual(Scaler.transform_point(DPoint(1,2)), DPoint(2,4)));
            TEST(almostEqual(Scaler.inverse_transform_point(DPoint(0,0)), DPoint(0,0)));
            TEST(almostEqual(Scaler.inverse_transform_point(DPoint(1,2)), DPoint(0.5,1)));

            DeDistort Shifter = DeDistort(DPoint(1,1), DPoint(1,1),
                Params, DPoint3(0,0,0), DPoint3(0,0,1), 0, 0.0,
                DPoint(0,0), DPoint(1,1));
            TEST(almostEqual(Shifter.transform_point(DPoint(0,0)), DPoint(1,1)));
            TEST(almostEqual(Shifter.transform_point(DPoint(1,2)), DPoint(2,3)));
            TEST(almostEqual(Shifter.inverse_transform_point(DPoint(0,0)), DPoint(-1,-1)));
            TEST(almostEqual(Shifter.inverse_transform_point(DPoint(1,2)), DPoint(0,1)));

            vector<double> Cubed;
            Cubed.push_back(0);
            Cubed.push_back(1);
            DeDistort Barreler = DeDistort(DPoint(0,0), DPoint(1,1),
                Cubed, DPoint3(0,0,0), DPoint3(0,0,1), 0, 0.0,
                DPoint(0,0), DPoint(1,1));
            for (double xp=0;xp<10;xp++){
                for(double yp=0;yp<10;yp++){
                    TEST(almostEqual(Barreler.inverse_transform_point(
                            Barreler.transform_point(DPoint(xp,yp))), DPoint(xp,yp)));
                }
            TEST(almostEqual(Barreler.transform_point(DPoint(1,1)), DPoint(1,1)));
            }
//            TEST(almostEqual(Barreler.transform_point(DPoint(1,0)), DPoint(3,0)));
//            TEST(almostEqual(Barreler.transform_point(DPoint(0,1)), DPoint(0,3)));
//            TEST(almostEqual(Barreler.inverse_transform_point(DPoint(0,0)), DPoint(0,0)));
//            TEST(almostEqual(Barreler.inverse_transform_point(DPoint(0,3)), DPoint(0,1)));

            DeDistort Rotator = DeDistort(DPoint(0,0), DPoint(1,1),
                Params, DPoint3(0,0,0), DPoint3(0,0,1), 0, M_PI/2,
                DPoint(0,0), DPoint(1,1));
            for (double xp=0;xp<10;xp++){
                for(double yp=0;yp<10;yp++){
                    TEST(almostEqual(Rotator.inverse_transform_point(
                            Rotator.transform_point(DPoint(xp,yp))), DPoint(xp,yp)));
                }
            }
//           TEST(almostEqual(Rotator.transform_point(DPoint(0,0)), DPoint(0,0)));
//          TEST(almostEqual(Rotator.transform_point(DPoint(1,2)), DPoint(-2,1)));
//            TEST(almostEqual(Rotator.inverse_transform_point(DPoint(0,0)), DPoint(0,0)));
//            TEST(almostEqual(Rotator.inverse_transform_point(DPoint(1,2)), DPoint(2,-1)));

            DeDistort ShifterScaler = DeDistort(DPoint(1,1), DPoint(2,2),
                Params, DPoint3(0,0,0), DPoint3(0,0,1), 0, 0.0,
                DPoint(0,0), DPoint(1,1));
            for (double xp=0;xp<10;xp++){
                for(double yp=0;yp<10;yp++){
                    TEST(almostEqual(ShifterScaler.inverse_transform_point(
                            ShifterScaler.transform_point(DPoint(xp,yp))), DPoint(xp,yp)));
                }
            }
//            TEST(almostEqual(ShifterScaler.transform_point(DPoint(0,0)), DPoint(2,2)));
//            TEST(almostEqual(ShifterScaler.transform_point(DPoint(1,2)), DPoint(4,6)));
//            TEST(almostEqual(ShifterScaler.inverse_transform_point(DPoint(0,0)), DPoint(-0.5,-0.5)));
//            TEST(almostEqual(ShifterScaler.inverse_transform_point(DPoint(1,2)), DPoint(0,0.5)));
            
    }
};

class DistortionTest: public Test {
    public:
        DistortionTest()
          : Test("DistortionTest",2)
        {}

        void runTests()
        {
            {
                cerr << "    Save and load TrackerConfig" << endl;
                TrackerConfig Config;
                Config.save("TempConfig.xml");
                Config.load("TempConfig.xml");
            }
            cerr << "    Identical Transformer" << endl;
            TrapezoidAndBarrel Transformer(IntRect(0,0,100,100), 0, 0, 1);
            TEST(Transformer.transform_point(DPoint(0,0)) == DPoint(0,0));
            TEST(Transformer.transform_point(DPoint(100,100)) == DPoint(100,100));
            
            cerr << "    Trapezoid Transformer 1" << endl;
            TrapezoidAndBarrel TrapT(IntRect(0,0,100,100), 0, 1, 1);
            DPoint TestPt(100,0);
            testNullTransform(TrapT, TestPt);
            TestPt = DPoint(0,0);
            testNullTransform(TrapT, TestPt);
            TestPt = DPoint(100,100);
            testNullTransform(TrapT, TestPt);
            TestPt = DPoint(100,50);
            testNullTransform(TrapT, TestPt);
            
            cerr << "    Trapezoid Transformer 2" << endl;
            TrapezoidAndBarrel TrapT1(IntRect(0,0,100,100), 0, 0.1, 1);
            TestPt = DPoint(100,10);
            testNullTransform(TrapT1, TestPt);

            cerr << "    Barrel Transformer" << endl;
            TrapezoidAndBarrel BarrelT(IntRect(0,0,100,100), 0.1, 0, 1.1);
            TestPt = DPoint(100,0);
            testNullTransform(BarrelT, TestPt);
            TestPt = DPoint(0,0);
            testNullTransform(BarrelT, TestPt);
            TestPt = DPoint(50,0);
            testNullTransform(BarrelT, TestPt);
            TestPt = DPoint(0,50);
            testNullTransform(BarrelT, TestPt);
            TestPt = DPoint(23,42);
            testNullTransform(BarrelT, TestPt);
            TestPt = DPoint(0,0);
            DPoint SamePt = BarrelT.transform_point(TestPt);
            TEST(SamePt.x - TestPt.x < 0.001 && SamePt.y - TestPt.y < 0.001);
            SamePt = BarrelT.inverse_transform_point(TestPt);
            TEST(SamePt.x - TestPt.x < 0.001 && SamePt.y - TestPt.y < 0.001);
            
            cerr << "    Combined Transformer" << endl;
            TrapezoidAndBarrel CombinedT(IntRect(0,0,100,100), 0.2, 0.1, 1.2);
            TestPt = DPoint(100,0);
            testNullTransform(BarrelT, TestPt);
            TestPt = DPoint(0,100);
            testNullTransform(BarrelT, TestPt);
            TestPt = DPoint(0,50);
            testNullTransform(BarrelT, TestPt);
            TestPt = DPoint(23,42);
            testNullTransform(BarrelT, TestPt);
#if 0
            BitmapPtr in_bmp = FilterGrayscale().apply(BitmapPtr(new Bitmap("testimages/squares.png")));
            FilterDistortion BarrelFilter = FilterDistortion(in_bmp->getSize(),CoordTransformerPtr();
            BitmapPtr pBarrelBmp = BarrelFilter.apply(in_bmp);
            FilterDistortion TrapezoidFilter = FilterDistortion(in_bmp->getSize(),0,0.3);
            BitmapPtr pTrapezoidBmp = TrapezoidFilter.apply(in_bmp);
            FilterDistortion CombinedFilter = FilterDistortion(in_bmp->getSize(),0.08,0.08);
            BitmapPtr pCombinedBmp = CombinedFilter.apply(in_bmp);
#ifdef GENERATE_BASELINE
            cerr << "    ---- WARNING: Generating new testimages images, not executing tests." 
                    << endl;
            pBarrelBmp->save("testimages/barrel.png");
            pTrapezoidBmp->save("testimages/trapezoid.png");
            pCombinedBmp->save("testimages/combined.png");
#else
            BitmapPtr pBarrelBaselineBmp = FilterGrayscale().apply(BitmapPtr(
                    new Bitmap("testimages/barrel.png")));
            TEST(*pBarrelBmp == *pBarrelBaselineBmp);
            BitmapPtr pTrapezoidBaselineBmp = FilterGrayscale().apply(BitmapPtr(
                    new Bitmap("testimages/trapezoid.png")));
            TEST(*pTrapezoidBmp == *pTrapezoidBaselineBmp);
            BitmapPtr pCombinedBaselineBmp = FilterGrayscale().apply(BitmapPtr(
                    new Bitmap("testimages/combined.png")));
            TEST(*pCombinedBmp == *pCombinedBaselineBmp);
#endif
#endif
        }

        void testNullTransform(CoordTransformer& T, DPoint& Pt)
        {
            DPoint TempPt = T.transform_point(Pt);
            DPoint SamePt = T.inverse_transform_point(TempPt);
            TEST(SamePt.x - Pt.x < 1 && SamePt.y - Pt.y < 1);
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
        BitmapPtr pBitmaps[NUM_TRACKER_IMAGES];
        for (int i=0; i<NUM_TRACKER_IMAGES; i++) {
            pBitmaps[i] = BitmapPtr(new Bitmap(pCam->getImgSize(), I8));
        }
        MutexPtr pMutex(new boost::mutex);
        //FilterIdPtr pp = FilterIdPtr(new FilterId());
        TrackerConfig config;
        m_pCmdQ = TrackerThread::CmdQueuePtr(new TrackerThread::CmdQueue);
        boost::thread Thread(
                TrackerThread(pCam, pBitmaps, pMutex,  *m_pCmdQ, this, true, config));
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
/*
                    TEST(pBlobs->size() == 1);
                    BlobInfoPtr pBlobInfo = (*pBlobs->begin())->getInfo();
                    cerr << pBlobInfo->m_Area << endl;
                    TEST(fabs(pBlobInfo->m_Area-32)<0.001);
                    TEST(fabs(pBlobInfo->m_Orientation)<0.001);
                    TEST(fabs(pBlobInfo->m_Center.x-11.5)<0.0001); 
                    TEST(fabs(pBlobInfo->m_Center.y-7.5)<0.0001);
                    cerr << pBlobInfo->m_BoundingBox << endl;
                    TEST(pBlobInfo->m_BoundingBox == IntRect(9,5,15,11)); 
*/
                }
                break;
            case 2:
//                TEST(pBlobs->size() == 2);
                break;
            case 3:
                break;
            case 4:
/*
                {
                    cerr << pBlobs->size() << endl;
                    TEST(pBlobs->size() == 1);
                    BlobInfoPtr pBlobInfo = (*pBlobs->begin())->getInfo();
                    TEST(fabs(pBlobInfo->m_Area-114)<0.001);
                    TEST(pBlobInfo->m_BoundingBox == IntRect(4,15,31,21)); 
                }
*/
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
};

class ImagingTestSuite: public TestSuite {
public:
    ImagingTestSuite() 
        : TestSuite("ImagingTestSuite")
    {
        addTest(TestPtr(new TrackingTest));
        addTest(TestPtr(new DeDistortTest));
        addTest(TestPtr(new DistortionTest));
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

