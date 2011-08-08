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

#ifndef _VideoMsg_H_
#define _VideoMsg_H_

#include "../api.h"
#include "../base/Queue.h"
#include "../graphics/Bitmap.h"

#include "../audio/AudioBuffer.h"

#include <boost/shared_ptr.hpp>

struct vdpau_render_state;

namespace avg {

class AVG_API VideoMsg {
public:
    enum MsgType {NONE, AUDIO, END_OF_FILE, ERROR, FRAME, SEEK_DONE, VDPAU_FRAME};
    VideoMsg();
    void setAudio(AudioBufferPtr pAudioBuffer, double audioTime);
    void setEOF();
    void setError(const Exception& ex);
    void setFrame(const std::vector<BitmapPtr>& pBmps, double frameTime);
    void setVDPAUFrame(vdpau_render_state* m_pRenderState, double frameTime);
    void setSeekDone(double seekVideoFrameTime, double seekAudioFrameTime);

    virtual ~VideoMsg();

    MsgType getType();

    AudioBufferPtr getAudioBuffer() const;
    double getAudioTime() const;

    const Exception& getException() const;

    BitmapPtr getFrameBitmap(int i);
    double getFrameTime();

    double getSeekVideoFrameTime();
    double getSeekAudioFrameTime();

    vdpau_render_state* getRenderState();

private:
    MsgType m_MsgType;

    // AUDIO
    AudioBufferPtr m_pAudioBuffer;
    double m_AudioTime;

    // ERROR
    Exception* m_pEx;

    // FRAME
    std::vector<BitmapPtr> m_pBmps;
    double m_FrameTime;

    // VDPAU_FRAME
    vdpau_render_state* m_pRenderState;

    // SEEK_DONE
    double m_SeekVideoFrameTime;
    double m_SeekAudioFrameTime;

};

typedef boost::shared_ptr<VideoMsg> VideoMsgPtr;
typedef Queue<VideoMsg> VideoMsgQueue;
typedef boost::shared_ptr<VideoMsgQueue> VideoMsgQueuePtr;

}
#endif 

