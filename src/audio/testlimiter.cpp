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

#include "Dynamics.h"

#include "../base/TestSuite.h"

#include <stdlib.h>
#include <iostream>
#include <sys/time.h>

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
        typedef double TSample;
        const int IN_CHANNELS = 2;
        const int OUT_CHANNELS = 2;
        TSample fs = 44100.;
        int numSamples = int(fs * 500);

        // Setup a brickwall limiter
        typedef Dynamics<TSample, IN_CHANNELS, OUT_CHANNELS> TStereoLimiter;
        TStereoLimiter* d = new TStereoLimiter(fs);
        d->setThreshold(0.); // in dB
        d->setAttackTime(0.); // in seconds
        d->setReleaseTime(0.05); // in seconds
        d->setRmsTime(0.); // in seconds
        d->setRatio(std::numeric_limits<TSample>::infinity());
        d->setMakeupGain(0.); // in dB

        // Generate some input test data
        TSample* inSamples = new TSample[IN_CHANNELS*numSamples];
        for (int j = 0; j < numSamples; j++) {
            for (int i = 0; i < IN_CHANNELS; i++) {
                inSamples[j*IN_CHANNELS+i] = 2*sin(j*(440./44100)*M_PI);
                //          inSamples[j*IN_CHANNELS+i] = 2*TDouble(rand())/RAND_MAX;
            }
        }

        // Initialize output data
        TSample* outSamples = new TSample[OUT_CHANNELS*numSamples];
        for (int j = 0; j < numSamples; j++) {
            for (int i = 0; i < OUT_CHANNELS; i++) {
                outSamples[j*OUT_CHANNELS+i] = 3.;
            }
        }

        long long startMSecs;
        struct timeval now;
        gettimeofday(&now, NULL);
        startMSecs=((long long)now.tv_sec)*1000+now.tv_usec/1000;
        // Limit it
        cerr << "start" << endl;
        for (int i=0; i<numSamples; ++i) {
            d->Process(inSamples+i*IN_CHANNELS, outSamples+i*OUT_CHANNELS);
        }

        long long endMSecs;
        gettimeofday(&now, NULL);
        endMSecs=((long long)now.tv_sec)*1000+now.tv_usec/1000;
        cerr << "Needed " << endMSecs-startMSecs << " msecs." << endl;

        // Check it
        bool bDiscontinuities = false;
        bool bAboveThreshold = false;
        for (int j = 1; j < numSamples; j++) {
            for (int i = 0; i < OUT_CHANNELS; i++) {
                // Test if anything is above the threshold.
                if (outSamples[j*OUT_CHANNELS+i] > 1) {
                    bAboveThreshold = true;
                }
                if (fabs(outSamples[j*OUT_CHANNELS+i]-outSamples[(j-1)*OUT_CHANNELS+i]) > 0.05) {
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
            fprintf(pFile, "%f %f\n", inSamples[j*OUT_CHANNELS], outSamples[j*OUT_CHANNELS]);
        }
        fclose(pFile);
*/
        // Free memory
        delete d;
        delete[] inSamples;
        delete[] outSamples;
    }
};

int main(int nargs, char** args)
{
    LimiterTest Test;
    Test.runTests();
    bool bOK = Test.isOk();

    if (bOK) {
        return 0;
    } else {
        return 1;
    }
}
