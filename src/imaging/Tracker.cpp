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

#include "Tracker.h"
#include "CameraCmd.h"
#include "CameraUtils.h"
#include "ConnectedComps.h"

#include <iostream>

using namespace std;

namespace avg {

Tracker::Tracker(std::string sDevice, double FrameRate, std::string sMode, IBlobTarget *target)
{
#if defined(AVG_ENABLE_1394) || defined(AVG_ENABLE_1394_2)
    IntPoint ImgDimensions = getCamImgSize(getCamMode(sMode));
#else
    IntPoint ImgDimensions(640,480);
#endif
    for (int i=0; i<NUM_TRACKER_IMAGES; i++) {
        m_pBitmaps[i] = BitmapPtr(new Bitmap(ImgDimensions, I8));
    }
    m_pMutex = MutexPtr(new boost::mutex);
    m_pCmdQueue = TrackerCmdQueuePtr(new TrackerCmdQueue());
    m_pThread = new boost::thread(
            TrackerThread(sDevice, 
                FrameRate, 
                sMode, 
                m_pBlobList, 
                m_pBitmaps, 
                m_pMutex,
                m_pCmdQueue,
                target
                )
            );
}

Tracker::~Tracker()
{
    // TODO: Send stop to thread, join()
    delete m_pThread;
}


Bitmap * Tracker::getImage(TrackerImageID ImageID) const
{
    boost::mutex::scoped_lock Lock(*m_pMutex);
    return new Bitmap(*m_pBitmaps[ImageID]);
}
bool Tracker::setConfig(TrackerConfig config)
{
    TrackerCmdPtr cmd = TrackerCmdPtr( new TrackerCmd(TrackerCmd::CONFIG) );
    //FIXME convert the numbers from logical to physical!!
    cmd->config = TrackerConfigPtr(new TrackerConfig(config));
    m_pCmdQueue->push(cmd);
    return true;
}

/*
BlobListPtr Tracker::getTouches()
{
    // Returns the current touch info list and sets up a new one to fill for
    // the tracker thread.
    boost::mutex::scoped_lock Lock(*m_pMutex);
    //need to copy the list before passing it out.
    BlobListPtr pTempBlobList = BlobListPtr(new BlobList(*m_pBlobList));
    return pTempBlobList;
}
*/
}
