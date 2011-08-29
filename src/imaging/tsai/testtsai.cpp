//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2011 Ulrich von Zadow
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

extern "C" {
#include "cal_main.h"
}

#include "../../base/TestSuite.h"

#include <stdio.h>
#include <stdlib.h>

#include <sstream>

using namespace avg;
using namespace std;

class TsaiTest: public Test {
    public:
        TsaiTest()
          : Test("TsaiTest", 2)
        {}

        void runTests()
        {
            camera_parameters cp;
            calibration_data cd;
            calibration_constants cc;
/*
            cp.dpx = 1;
            cp.dpy = 1;
            cp.Cx = 0.9;
            cp.Cy = 0.9;
            cp.sx = 1.0;
 
            cd.point_count = 0;
            addCDPoint(cd, 0, -0.1, 0,  0, 0);
            addCDPoint(cd, 0, 1, 0,     0, 1);
            addCDPoint(cd, 0, 2.1, 0,   0, 2);
            addCDPoint(cd, 1, 0, 0,     1, 0);
            addCDPoint(cd, 1, 1, 0,     1, 1);
            addCDPoint(cd, 1, 2, 0,     1, 2);
            addCDPoint(cd, 2, 0.1, 0,   2, 0);
            addCDPoint(cd, 2, 1, 0,     2, 1);
            addCDPoint(cd, 2, 1.9, 0,   2, 2);
            addCDPoint(cd, 3, 0.2, 0,   3, 0);
            addCDPoint(cd, 3, 1, 0,     3, 1);
            addCDPoint(cd, 3, 1.8, 0,   3, 2);
*/

            cp.dpx = 1;
            cp.dpy = 1;
            cp.Cx = 320;
            cp.Cy = 240;
            cp.sx = 1.0;
 
            cd.point_count = 0;
            addCDPoint(cd, 10, 10, 0,     0, 0);
            addCDPoint(cd, 330, 10, 0,   320, 0);
            addCDPoint(cd, 650, 10, 0,   640, 0);
            addCDPoint(cd, 10, 250, 0,     0, 240);
            addCDPoint(cd, 330, 250, 0,   320, 240);
            addCDPoint(cd, 650, 250, 0,   640, 240);
            addCDPoint(cd, 10, 490, 0,     0, 480);
            addCDPoint(cd, 330, 490, 0,   320, 480);
            addCDPoint(cd, 650, 490, 0,   640, 480);

//            coplanar_calibration(&cp, &cd, &cc);
            coplanar_calibration_with_full_optimization(&cp, &cd, &cc);

            print_cp_cc_data(stdout, &cp, &cc);
            print_error_stats(stdout);
        }

    private:
        void addCDPoint(calibration_data& cd, double xw, double yw, double zw, 
                double Xf, double Yf) 
        {
            int i = cd.point_count;
            cd.xw[i] = xw;
            cd.yw[i] = yw;
            cd.zw[i] = zw;
            cd.Xf[i] = Xf;
            cd.Yf[i] = Yf;
            cd.point_count++;
        }

};


int main(int nargs, char** args)
{
    TsaiTest test;
    test.runTests();
    bool bOK = test.isOk();

    if (bOK) {
        return 0;
    } else {
        return 1;
    }
}


