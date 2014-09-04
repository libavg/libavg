//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2014 Ulrich von Zadow
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

#ifndef _VideoNode_H_
#define _VideoNode_H_

// Python docs say python.h should be included before any standard headers (!)
#include "../api.h"
#include "WrapPython.h" 

#include "Node.h"
#include "RasterNode.h"

#include "../base/GLMHelper.h"
#include "../base/IFrameEndListener.h"
#include "../base/UTF8String.h"

#include "../video/VideoDecoder.h"

namespace avg {

class VideoDecoder;
class TextureMover;
typedef boost::shared_ptr<TextureMover> TextureMoverPtr;
class MCTexture;
typedef boost::shared_ptr<MCTexture> MCTexturePtr;

class AVG_API VideoNode: public RasterNode, IFrameEndListener
{
    public:
        enum VideoAccelType {NONE, VDPAU};

        static void registerType();
        
        VideoNode(const ArgList& args);
        virtual ~VideoNode();
        
        virtual void connectDisplay();
        virtual void connect(CanvasPtr pCanvas);
        virtual void disconnect(bool bKill);

        void play();
        void stop();
        void pause();

        const UTF8String& getHRef() const;
        void setHRef(const UTF8String& href);
        float getVolume();
        void setVolume(float volume);
        float getFPS() const;
        int getQueueLength() const;
        void checkReload();

        int getNumFrames() const;
        int getCurFrame() const;
        int getNumFramesQueued() const;
        void seekToFrame(int frameNum);
        std::string getStreamPixelFormat() const;
        long long getDuration() const;
        long long getVideoDuration() const;
        long long getAudioDuration() const;
        int getBitrate() const;
        std::string getContainerFormat() const;
        std::string getVideoCodec() const;
        std::string getAudioCodec() const;
        int getAudioSampleRate() const;
        int getNumAudioChannels() const;

        long long getCurTime() const;
        void seekToTime(long long time);
        bool getLoop() const;
        bool isThreaded() const;
        bool hasAudio() const;
        bool hasAlpha() const;
        void setEOFCallback(PyObject * pEOFCallback);
        bool isAccelerated() const;

        virtual void preRender(const VertexArrayPtr& pVA, bool bIsParentActive, 
                float parentEffectiveOpacity);
        virtual void render();
        virtual void onFrameEnd();
        
        virtual IntPoint getMediaSize();

        static VideoAccelType getVideoAccelConfig();

    private:
        bool renderFrame();
        FrameAvailableCode renderToSurface();
        void seek(long long destTime);
        void onEOF();
        void updateStatusDueToDecoderEOF();
        void dumpFramesTooLate();

        void open();
        void startDecoding();
        void createTextures(IntPoint size);
        void close();
        enum VideoState {Unloaded, Paused, Playing};
        void changeVideoState(VideoState NewVideoState);
        PixelFormat getPixelFormat() const;
        long long getNextFrameTime() const;
        void exceptionIfNoAudio(const std::string& sFuncName) const;
        void exceptionIfUnloaded(const std::string& sFuncName) const;

        VideoState m_VideoState;

        bool m_bFrameAvailable;
        bool m_bFirstFrameDecoded;

        UTF8String m_href;
        std::string m_Filename;
        bool m_bLoop;
        bool m_bThreaded;
        float m_FPS;
        int m_QueueLength;
        bool m_bEOFPending;
        PyObject * m_pEOFCallback;
        int m_FramesTooLate;
        int m_FramesInRowTooLate;
        int m_FramesPlayed;
        bool m_bSeekPending;
        long long m_SeekBeforeCanRenderTime;

        long long m_StartTime;
        long long m_PauseTime;
        long long m_PauseStartTime;
        float m_JitterCompensation;

        VideoDecoder * m_pDecoder;
        float m_Volume;
        bool m_bUsesHardwareAcceleration;
        bool m_bEnableSound;
        int m_AudioID;

        MCTexturePtr m_pTextures[4];
};

}

#endif 

