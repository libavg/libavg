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

#include "TrackerEventSource.h"

#include "../imaging/FakeCamera.h"
#include "../imaging/TrackerThread.h"
#include "../imaging/TrackerConfig.h"
#include "../base/Rect.h"

#include "../base/TimeSource.h"
#include "../base/TestSuite.h"
#include "../base/Exception.h"
#include "../base/Logger.h"

#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <iomanip>
#include <sstream>

using namespace avg;
using namespace std;

class TrackingTest: public Test{
public:
    TrackingTest()
        : Test("TrackingTest", 2)
    {
    }

    void runTests() 
    {
        std::vector<std::string> p = std::vector<std::string>();
        Logger::get()->setCategories(Logger::EVENTS|Logger::EVENTS2);
        for (int i=0; i<30; ++i) {
            stringstream s;
            s << "testimages/" <<setw(2)<<setfill('0')<<i<< ".png";
            p.push_back(s.str());
        }
        assert(p.size()>0);
        CameraPtr pCam = CameraPtr(new FakeCamera(p));
        BitmapPtr pBitmaps[NUM_TRACKER_IMAGES];
        for (int i=0; i<NUM_TRACKER_IMAGES; i++) {
            pBitmaps[i] = BitmapPtr(new Bitmap(pCam->getImgSize(), I8));
        }
        MutexPtr pMutex(new boost::mutex);

        TrackerConfig Config;
        Config.load("../imaging/avgtrackerrc.minimal");
        TrackerEventSourcePtr pTracker = TrackerEventSourcePtr(
                new TrackerEventSource(pCam, Config, IntPoint(640, 480), false));

        while(1){
            msleep(50);
            std::vector<EventPtr> e = pTracker->pollEvents();
            if(e.size()==0)
                cerr<<"no new events"<<endl;
            for(std::vector<EventPtr>::iterator it=e.begin();it!=e.end();++it){
                (*it)->trace();
            }
        }
    }
    

private:
    TrackerThread::CmdQueuePtr m_pCmdQ;
};

class TrackerTestSuite: public TestSuite {
public:
    TrackerTestSuite() 
        : TestSuite("TrackerTestSuite")
    {
        addTest(TestPtr(new TrackingTest));
    }
};


int main(int nargs, char** args)
{
    TrackerTestSuite Suite;
    Suite.runTests();
    bool bOK = Suite.isOk();

    if (bOK) {
        return 0;
    } else {
        return 1;
    }
}

