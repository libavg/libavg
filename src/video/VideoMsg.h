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

#ifndef _VideoMsg_H_
#define _VideoMsg_H_

#include "../api.h"
#include "../base/Queue.h"
#include "../graphics/Bitmap.h"

#include "../audio/AudioMsg.h"

#include <boost/shared_ptr.hpp>

struct vdpau_render_state;
struct AVPacket;

namespace avg {

class AVG_API VideoMsg: public AudioMsg {
public:
    VideoMsg();
    void setFrame(const std::vector<BitmapPtr>& pBmps, float frameTime);
    void setVDPAUFrame(vdpau_render_state* pRenderState, float frameTime);
    void setPacket(AVPacket* pPacket);

    virtual ~VideoMsg();

    BitmapPtr getFrameBitmap(int i);
    float getFrameTime();
    AVPacket* getPacket();
    void freePacket();

    vdpau_render_state* getRenderState();

private:
    // FRAME
    std::vector<BitmapPtr> m_pBmps;
    float m_FrameTime;

    // VDPAU_FRAME
    vdpau_render_state* m_pRenderState;
    
    // PACKET
    AVPacket * m_pPacket;
};

typedef boost::shared_ptr<VideoMsg> VideoMsgPtr;
typedef Queue<VideoMsg> VideoMsgQueue;
typedef boost::shared_ptr<VideoMsgQueue> VideoMsgQueuePtr;

}
#endif 

