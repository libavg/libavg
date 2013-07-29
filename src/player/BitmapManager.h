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

#ifndef _BitmapManager_H_
#define _BitmapManager_H_

#include "BitmapManagerThread.h"
#include "BitmapManagerMsg.h"

#include "../api.h"
#include "../base/Queue.h"
#include "../base/IFrameEndListener.h"

#include <boost/thread.hpp>

#include <vector>

namespace avg {

class AVG_API BitmapManager : public IFrameEndListener
{
    public:
        BitmapManager();
        ~BitmapManager();
        static BitmapManager* get();
        void loadBitmap(const UTF8String& sUtf8FileName,
                const boost::python::object& pyFunc, PixelFormat pf=NO_PIXELFORMAT);
        void setNumThreads(int numThreads);

        virtual void onFrameEnd();
        
    private:
        void startThreads(int numThreads);
        void stopThreads();

        static BitmapManager * s_pBitmapManager;

        std::vector<boost::thread*> m_pBitmapManagerThreads;
        BitmapManagerThread::CQueuePtr m_pCmdQueue;
        BitmapManagerMsgQueuePtr m_pMsgQueue;
};

}

#endif
