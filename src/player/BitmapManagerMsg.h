//
//  libavg - Media Playback Engine.
//  Copyright (C) 2003-2021 Ulrich von Zadow
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

#ifndef _BitmapManagerMsg_H_
#define _BitmapManagerMsg_H_

#include "WrapPython.h"

#include "../api.h"
#include "../base/Queue.h"
#include "../base/UTF8String.h"
#include "../base/Exception.h"

#include "../graphics/PixelFormat.h"

#include <boost/shared_ptr.hpp>
#include <boost/python.hpp>


namespace avg {

class Bitmap;
typedef boost::shared_ptr<Bitmap> BitmapPtr;
class IBitmapLoadedListener;

class AVG_API BitmapManagerMsg
{
public:
    enum MsgType {REQUEST, BITMAP, ERROR};

    BitmapManagerMsg(const UTF8String& sFilename,
            const boost::python::object& onLoadedCb, PixelFormat pf);
    BitmapManagerMsg(const UTF8String& sFilename,
            IBitmapLoadedListener* pLoadedListener, PixelFormat pf);
    virtual ~BitmapManagerMsg();
    void init(const UTF8String& sFilename, PixelFormat pf);

    void executeCallback();
    const UTF8String getFilename();
    float getStartTime();
    PixelFormat getPixelFormat();
    void setBitmap(BitmapPtr pBmp);
    void setError(const Exception& ex);

    MsgType getType() { return m_MsgType; };

private:
    UTF8String m_sFilename;
    float m_StartTime;
    BitmapPtr m_pBmp;
    boost::python::object m_OnLoadedCb;
    IBitmapLoadedListener* m_pLoadedListener;
    PixelFormat m_PF;
    MsgType m_MsgType;
    Exception* m_pEx;
};

typedef boost::shared_ptr<BitmapManagerMsg> BitmapManagerMsgPtr;
typedef Queue<BitmapManagerMsg> BitmapManagerMsgQueue;
typedef boost::shared_ptr<BitmapManagerMsgQueue> BitmapManagerMsgQueuePtr;
}

#endif
