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

#include "Dynamics.h"

#include "../base/TestSuite.h"
#include "../base/MathHelper.h"

#include <stdlib.h>
#include <iostream>

using namespace avg;
using namespace std;

class LimiterTest: public Test {
public:
    LimiterTest()
        : Test("LimiterTest", 2)
    {
    }

    void runTests()
    {
        const int CHANNELS = 2;
        double fs = 44100.;
        int numSamples = int(fs * 0.1);

        // Setup a brickwall limiter
        typedef Dynamics<double, CHANNELS> TStereoLimiter;
        TStereoLimiter* d = new TStereoLimiter(fs);
        d->setThreshold(0.); // in dB
        d->setAttackTime(0.); // in seconds
        d->setReleaseTime(0.05); // in seconds
        d->setRmsTime(0.); // in seconds
        d->setRatio(std::numeric_limits<double>::infinity());
        d->setMakeupGain(0.); // in dB

        // Generate input and output test data
        double* pSamples = new double[CHANNELS*numSamples];
        for (int j = 0; j < numSamples; j++) {
            for (int i = 0; i < CHANNELS; i++) {
                pSamples[j*CHANNELS+i] = 2*sin(j*(440./44100)*PI);
            }
        }

        // Let the limiter work.
        for (int i=0; i<numSamples; ++i) {
            d->process(pSamples+i*CHANNELS);
        }

        // Check if everything is ok.
        bool bDiscontinuities = false;
        bool bAboveThreshold = false;
        for (int j = 1; j < numSamples; j++) {
            for (int i = 0; i < CHANNELS; i++) {
                // Test if anything is above the threshold.
                if (pSamples[j*CHANNELS+i] > 1) {
                    bAboveThreshold = true;
                }
                if (fabs(pSamples[j*CHANNELS+i]-pSamples[(j-1)*CHANNELS+i]) > 0.05) {
                    bDiscontinuities = true;
//                    cerr << j << ": " << outSamples[j*OUT_CHANNELS+i] << ", " << 
//                            outSamples[(j-1)*OUT_CHANNELS+i] << endl;
                }
            }
        }
        TEST(!bAboveThreshold);
        TEST(!bDiscontinuities);
/*
        // Save data to ascii file.
        FILE * pFile = fopen("data.txt", "w");
        for (int j = 0; j < numSamples; j++) {
            fprintf(pFile, "%f\n", pSamples[j*OUT_CHANNELS]);
        }
        fclose(pFile);
*/
        // Free memory
        delete d;
        delete[] pSamples;
    }
};

int main(int nargs, char** args)
{
    LimiterTest test;
    test.runTests();
    bool bOK = test.isOk();

    if (bOK) {
        return 0;
    } else {
        return 1;
    }
}
