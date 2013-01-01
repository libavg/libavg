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

#ifndef _AudioMsg_H_
#define _AudioMsg_H_

#include "../api.h"
#include "../base/Queue.h"

#include "AudioBuffer.h"

#include <boost/shared_ptr.hpp>

namespace avg {

class AVG_API AudioMsg {
public:
    enum MsgType {NONE, AUDIO, AUDIO_TIME, END_OF_FILE, ERROR, FRAME, VDPAU_FRAME, 
            SEEK_DONE};
    AudioMsg();
    void setAudio(AudioBufferPtr pAudioBuffer, float audioTime);
    void setAudioTime(float audioTime);
    void setEOF();
    void setError(const Exception& ex);
    void setSeekDone(float seekVideoFrameTime, float seekAudioFrameTime);

    virtual ~AudioMsg();

    MsgType getType();

    AudioBufferPtr getAudioBuffer() const;
    float getAudioTime() const;

    const Exception& getException() const;

    float getSeekVideoFrameTime();
    float getSeekAudioFrameTime();

    virtual void dump();

protected:
    void setType(MsgType msgType);

private:
    MsgType m_MsgType;

    // AUDIO
    AudioBufferPtr m_pAudioBuffer;
    float m_AudioTime;

    // ERROR
    Exception* m_pEx;

    // SEEK_DONE
    float m_SeekVideoFrameTime;
    float m_SeekAudioFrameTime;

};

typedef boost::shared_ptr<AudioMsg> AudioMsgPtr;
typedef Queue<AudioMsg> AudioMsgQueue;
typedef boost::shared_ptr<AudioMsgQueue> AudioMsgQueuePtr;

}
#endif 

