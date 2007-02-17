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

#include "../graphics/Rect.h"
#include "TrackerCalibrator.h"

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
        DPoint offset;
        DPoint scale;
        {
            TrackerCalibrator Calibrator(IntPoint(640, 480), 
                    IntRect(0,0,640,480), IntPoint(640,480));
            bool bDone = false;
            while (!bDone) {
                IntPoint DisplayPoint(Calibrator.getDisplayPointX(), 
                        Calibrator.getDisplayPointY());
                Calibrator.setCamPoint(DisplayPoint.x, DisplayPoint.y);
                bDone = !Calibrator.nextPoint();
            }
            Calibrator.makeTransformer(pTrafo, scale, offset);
            TEST(fabs(scale.x - 1.0)  < 0.01 && fabs(scale.y - 1.0) < 0.01);
            TEST(offset.x < 0.1 && offset.y < 0.1);
//            cerr << "scale: " << scale << ", offset: " << offset << endl;
            TEST(checkTransform(pTrafo, DPoint(0,0), DPoint(0,0)));
            TEST(checkTransform(pTrafo, DPoint(640, 480), DPoint(640, 480)));
        }
        {
            TrackerCalibrator Calibrator(IntPoint(640, 480), 
                    IntRect(0,0,640,480), IntPoint(1280,720));
            bool bDone = false;
            while (!bDone) {
                IntPoint DisplayPoint(Calibrator.getDisplayPointX(), 
                        Calibrator.getDisplayPointY());
                Calibrator.setCamPoint(DisplayPoint.x/2, DisplayPoint.y/1.5);
                bDone = !Calibrator.nextPoint();
            }
            Calibrator.makeTransformer(pTrafo, scale, offset);
            TEST(fabs(scale.x - 2.0)  < 0.01 && fabs(scale.y - 1.5) < 0.01);
            TEST(offset.x < 0.1 && offset.y < 0.1);
//            cerr << "scale: " << scale << ", offset: " << offset << endl;
            TEST(checkTransform(pTrafo, DPoint(0,0), DPoint(0,0)));
            TEST(checkTransform(pTrafo, DPoint(640, 480), DPoint(640, 480)));
            TEST(checkScaleOffset(pTrafo, scale, offset, DPoint(0,0), DPoint(0,0)));
            TEST(checkScaleOffset(pTrafo, scale, offset, DPoint(640, 480), DPoint(1280, 720)));
        }

/*
        {
            TrackerCalibrator Calibrator(IntPoint(640, 480), 
                    IntRect(0,0,640,480), IntPoint(640,480));
            bool bDone = false;
            while (!bDone) {
                IntPoint DisplayPoint(Calibrator.getDisplayPointX(), 
                        Calibrator.getDisplayPointY());
                Calibrator.setCamPoint(DisplayPoint.x+DisplayPoint.y, DisplayPoint.y);
                bDone = !Calibrator.nextPoint();
            }
            Calibrator.makeTransformer(pTrafo, scale, offset);
            cerr << "scale: " << scale << ", offset: " << offset << endl;
            DPoint ScreenPt = pTrafo->transform_point(DPoint(0,0));
            cerr << "(0,0) -> " << ScreenPt << endl;
            ScreenPt = pTrafo->transform_point(DPoint(640, 480));
            cerr << "(640, 480) -> " << ScreenPt << endl;
        }
*/        
    }

    bool checkTransform(CoordTransformerPtr pTrafo, const DPoint& SrcPt, 
            const DPoint& DestPt) 
    {
        DPoint ResultPt = pTrafo->transform_point(SrcPt);
//        cerr << SrcPt << " -> " << ResultPt << ", expected " << DestPt << endl;
        return ((fabs(ResultPt.x-DestPt.x) < 0.1) && (fabs(ResultPt.y-DestPt.y) < 0.1));
    }

    bool checkScaleOffset(CoordTransformerPtr pTrafo, 
            const DPoint& Scale, const DPoint& Offset, 
            const DPoint& SrcPt, const DPoint& DestPt)
    {
        DPoint ResultPt = pTrafo->transform_point(SrcPt);
        ResultPt = DPoint(ResultPt.x*Scale.x, ResultPt.y*Scale.y);
        ResultPt += Offset;
        cerr << SrcPt << " -> " << ResultPt << ", expected " << DestPt << endl;
        return ((fabs(ResultPt.x-DestPt.x) < 0.1) && (fabs(ResultPt.y-DestPt.y) < 0.1));
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
    CalibratorTestSuite Suite;
    Suite.runTests();
    bool bOK = Suite.isOk();

    if (bOK) {
        return 0;
    } else {
        return 1;
    }
}

