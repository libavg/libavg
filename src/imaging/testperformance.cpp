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
#include "../graphics/FilterId.h"
#include "../graphics/HistoryPreProcessor.h"
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

class TrackingTest: public Test, public IBlobTarget {
public:
    TrackingTest()
        : Test("TrackingTest", 2)
    {
    }

    void runTests() 
    {
        std::vector<std::string> p = std::vector<std::string>();
        for (int i=0; i<3; ++i) {
            stringstream s;
            s << "../imaging/testimages/Big" << i << ".png";
            p.push_back(s.str());
        }
        CameraPtr pCam = CameraPtr(new FakeCamera(p));
        BitmapPtr pBitmaps[NUM_TRACKER_IMAGES];
        for (int i=0; i<NUM_TRACKER_IMAGES; i++) {
            pBitmaps[i] = BitmapPtr(new Bitmap(pCam->getImgSize(), I8));
        }
        MutexPtr pMutex(new boost::mutex);
        m_pCmdQ = TrackerThread::CmdQueuePtr(new TrackerThread::CmdQueue);
        //FilterIdPtr pp = FilterIdPtr(new FilterId());
        HistoryPreProcessorPtr pp = HistoryPreProcessorPtr(new HistoryPreProcessor(pCam->getImgSize()));
        boost::thread Thread(
                TrackerThread(pCam, pBitmaps, pMutex,  *m_pCmdQ, this,&*pp));
        // TODO: Set TrackerConfig.
        Thread.join();
    }
    
    virtual void update(BlobListPtr pBlobs) {
        static int BmpIndex = 0;
        switch(BmpIndex) {
            case 0:
                TEST(pBlobs->size() == 0);
                break;
            case 1:
                {
                    TEST(pBlobs->size() >10);
                //    BlobInfoPtr pBlobInfo = (*pBlobs->begin())->getInfo();
                //    cerr<<pBlobInfo->m_Orientation<<endl;
                //    TEST(fabs(pBlobInfo->m_Area-16)<0.001);
                //    TEST(fabs(pBlobInfo->m_Orientation)<0.001);
                //    TEST(fabs(pBlobInfo->m_Center.x-11.5)<0.0001); 
                //    TEST(fabs(pBlobInfo->m_Center.y-7.5)<0.0001);
                //    TEST(pBlobInfo->m_BoundingBox == IntRect(10,6,14,10)); 
                }
                break;
            case 2:
                //TEST(pBlobs->size() == 2);
                m_pCmdQ->push(Command<TrackerThread>(boost::bind(&TrackerThread::stop, _1)));
                break;
            case 3:
                break;
            case 4:
                {
                //    TEST(pBlobs->size() == 1);
                //    BlobInfoPtr pBlobInfo = (*pBlobs->begin())->getInfo();
                //    cerr<<pBlobInfo->m_Orientation<<endl;
                //    TEST(fabs(pBlobInfo->m_Area-114)<0.001);
                //    TEST(pBlobInfo->m_BoundingBox == IntRect(4,15,31,21)); 
                }
                break;
            case 5:
                //TEST(pBlobs->size() == 0);
                //m_pCmdQ->push(Command<TrackerThread>(boost::bind(&TrackerThread::stop, _1)));
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

