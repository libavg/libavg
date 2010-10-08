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

#include "TrackerCalibrator.h"

#include "../base/Point.h"

#include "../base/TestSuite.h"
#include "../base/Exception.h"
#include "../base/Logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

using namespace avg;
using namespace std;

class CalibratorTest: public Test {
public:
    CalibratorTest()
        : Test("CalibratorTest", 2)
    {
    }

    void runTests() 
    {
        DeDistortPtr pTrafo;
        {
            TrackerCalibrator calibrator(IntPoint(640, 480), IntPoint(640,480));
            bool bDone = false;
            while (!bDone) {
                IntPoint displayPoint(calibrator.getDisplayPoint());
                calibrator.setCamPoint(DPoint(displayPoint));
                bDone = !calibrator.nextPoint();
            }
            pTrafo = calibrator.makeTransformer();
            TEST(  calcDist(pTrafo->transformBlobToScreen( DPoint(1.00,1.00) ) , DPoint(1.00,1.00))<1);
//            cerr << "scale: " << scale << ", offset: " << offset << endl;
            TEST(checkTransform(pTrafo, DPoint(0,0), DPoint(0,0)));
            TEST(checkTransform(pTrafo, DPoint(640, 480), DPoint(640, 480)));
        }
        {
            TrackerCalibrator calibrator(IntPoint(640, 480), IntPoint(1280,720));
            bool bDone = false;
            while (!bDone) {
                IntPoint displayPoint(calibrator.getDisplayPoint());
                calibrator.setCamPoint(DPoint(displayPoint.x/2, displayPoint.y/1.5));
                bDone = !calibrator.nextPoint();
            }
            pTrafo = calibrator.makeTransformer();
            TEST(  calcDist( pTrafo->transformBlobToScreen( DPoint(1.00,1.00) ), DPoint(2.00,1.50)) <1 );
//            cerr << "scale: " << scale << ", offset: " << offset << endl;
            TEST(checkTransform(pTrafo, DPoint(0,0), DPoint(0,0)));
            TEST(checkTransform(pTrafo, DPoint(640, 480), DPoint(640, 480)));
            TEST(checkBlobToScreen(pTrafo, DPoint(0,0), DPoint(0,0)));
            TEST(checkBlobToScreen(pTrafo, DPoint(640, 480), DPoint(1280, 720)));
        }
    }

    bool checkTransform(CoordTransformerPtr pTrafo, const DPoint& srcPt, 
            const DPoint& destPt) 
    {
        DPoint ResultPt = pTrafo->transform_point(srcPt);
//        cerr << srcPt << " -> " << ResultPt << ", expected " << destPt << endl;
        return ((fabs(ResultPt.x-destPt.x) < 0.1) && (fabs(ResultPt.y-destPt.y) < 0.1));
    }

    bool checkBlobToScreen(DeDistortPtr pTrafo, 
            const DPoint& srcPt, const DPoint& destPt)
    {
        DPoint ResultPt = pTrafo->transformBlobToScreen(pTrafo->transform_point(srcPt));
//        cerr << srcPt << " -> " << ResultPt << ", expected " << destPt << endl;
        return ((fabs(ResultPt.x-destPt.x) < 1) && (fabs(ResultPt.y-destPt.y) < 1));
    }
   
};

class CalibratorTestSuite: public TestSuite {
public:
    CalibratorTestSuite() 
        : TestSuite("CalibratorTestSuite")
    {
        addTest(TestPtr(new CalibratorTest));
    }
};


int main(int nargs, char** args)
{
    CalibratorTestSuite suite;
    suite.runTests();
    bool bOK = suite.isOk();

    if (bOK) {
        return 0;
    } else {
        return 1;
    }
}

