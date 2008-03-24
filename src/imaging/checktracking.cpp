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

#include "TrackerThread.h"
#include "FWCamera.h"
#include "FakeCamera.h"

#include "../graphics/Filtergrayscale.h"
#include "../graphics/FilterGauss.h"
#include "../graphics/FilterHighpass.h"
#include "../graphics/FilterBandpass.h"
#include "../graphics/FilterFastBandpass.h"
#include "../graphics/FilterBlur.h"

#include <sstream>
#include <iostream>
#include <math.h>

using namespace avg;
using namespace std;

class TestTracker: public IBlobTarget {

public:

TestTracker() 
{
    m_FrameNum = 0;
    std::vector<std::string> p = std::vector<std::string>();
    for (int i=1; i<4; ++i) {
        stringstream s;
        s << "camimages/img" << i << "_nohistory.png";
        p.push_back(s.str());
    }
    CameraPtr pCam = CameraPtr(new FakeCamera(p));
    for (int i=0; i<NUM_TRACKER_IMAGES; i++) {
        m_pBitmaps[i] = BitmapPtr(new Bitmap(pCam->getImgSize(), I8));
    }
    MutexPtr pMutex(new boost::mutex);
    TrackerConfig Config;
    Config.setParam("/tracker/touch/threshold/@value", "131");
    m_pCmdQ = TrackerThread::CmdQueuePtr(new TrackerThread::CmdQueue);
    IntRect ROI(0,0,pCam->getImgSize().x, pCam->getImgSize().y);
    boost::thread Thread(
            TrackerThread(ROI, pCam, m_pBitmaps, pMutex,  *m_pCmdQ, this, 
                    false, Config));
    Thread.join();
}

virtual ~TestTracker() 
{
}

virtual void update(BlobVectorPtr pBlobs, BitmapPtr) 
{
    m_FrameNum++;
    cerr << "Frame " << m_FrameNum << endl;
    for (int i=0; i<NUM_TRACKER_IMAGES; i++) {
        stringstream s;
        s << "camimages/img" << m_FrameNum << "_" << i << ".png";
        m_pBitmaps[i]->save(s.str());
    }
}

private:
    int m_FrameNum;
    TrackerThread::CmdQueuePtr m_pCmdQ;
    BitmapPtr m_pBitmaps[NUM_TRACKER_IMAGES];

};

void testBlur()
{
    BitmapPtr pBitmap(new Bitmap("camimages/img1_nohistory.png"));
    FilterGrayscale().applyInPlace(pBitmap);
    BitmapPtr pBmpBandpass = FilterFastBandpass().apply(pBitmap);
    pBmpBandpass->save("camimages/img1_bandpass_test.png");
}

int main(int argc, char **argv)
{
//    TestTracker t;
    testBlur();
}
