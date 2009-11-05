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
#include "Video.h"
#include "DisplayEngine.h"
#include "Player.h"
#include "OGLTiledSurface.h"
#include "NodeDefinition.h"
#include "SDLDisplayEngine.h"

#include "../base/Exception.h"
#include "../base/Logger.h"
#include "../base/ScopeTimer.h"
#include "../base/XMLHelper.h"

#include "../graphics/Filterfill.h"

#include "../audio/AudioEngine.h"

#include "../video/AsyncVideoDecoder.h"
#include "../video/FFMpegDecoder.h"

#include <iostream>
#include <sstream>

#ifndef _WIN32
#include <unistd.h>
#endif

using namespace boost::python;
using namespace std;

namespace avg {

NodeDefinition Video::createDefinition()
{
    return NodeDefinition("video", Node::buildNode<Video>)
        .extendDefinition(RasterNode::createDefinition())
        .addArg(Arg<string>("href", "", false, offsetof(Video, m_href)))
        .addArg(Arg<bool>("loop", false, false, offsetof(Video, m_bLoop)))
        .addArg(Arg<bool>("threaded", true, false, offsetof(Video, m_bThreaded)))
        .addArg(Arg<double>("fps", 0.0, false, offsetof(Video, m_FPS)))
        .addArg(Arg<double>("volume", 1.0, false, offsetof(Video, m_Volume)))
        ;
}

Video::Video(const ArgList& Args)
    : m_VideoState(Unloaded),
      m_bFrameAvailable(false),
      m_bFirstFrameDecoded(false),
      m_Filename(""),
      m_bEOFPending(false),
      m_pEOFCallback(0),
      m_FramesTooLate(0),
      m_FramesPlayed(0),
      m_pDecoder(0),
      m_Volume(1.0)
{
    Args.setMembers(this);
    m_Filename = m_href;
    initFilename(m_Filename);
    if (m_bThreaded) {
        VideoDecoderPtr pSyncDecoder = VideoDecoderPtr(new FFMpegDecoder());
        m_pDecoder = new AsyncVideoDecoder(pSyncDecoder);
    } else {
        m_pDecoder = new FFMpegDecoder();
    }
    ObjectCounter::get()->incRef(&typeid(*this));
}

Video::~Video()
{
    if (m_pDecoder) {
        delete m_pDecoder;
        m_pDecoder = 0;
    }
    if (m_pEOFCallback) {
        Py_DECREF(m_pEOFCallback);
    }
    ObjectCounter::get()->decRef(&typeid(*this));
}

void Video::setRenderingEngines(DisplayEngine * pDisplayEngine, 
        AudioEngine * pAudioEngine)
{
    checkReload();
    RasterNode::setRenderingEngines(pDisplayEngine, pAudioEngine);
    long long CurTime = Player::get()->getFrameTime(); 
    if (m_VideoState != Unloaded) {
        startDecoding();
        m_StartTime = CurTime;
        m_PauseTime = 0;
    }
    if (m_VideoState == Paused) {
        m_PauseStartTime = CurTime;
    } 
}

void Video::connect()
{
    Player::get()->registerFrameEndListener(this);
    RasterNode::connect();
}

void Video::disconnect(bool bKill)
{
    Player::get()->unregisterFrameEndListener(this);
    changeVideoState(Unloaded);
    RasterNode::disconnect(bKill);
}

void Video::play()
{
    changeVideoState(Playing);
}

void Video::stop()
{
    changeVideoState(Unloaded);
}

void Video::pause()
{
    changeVideoState(Paused);
}

int Video::getNumFrames() const
{
    exceptionIfUnloaded("getNumFrames");
    return m_pDecoder->getVideoInfo().m_NumFrames;
}

int Video::getCurFrame() const
{
    exceptionIfUnloaded("getCurFrame");
    return m_pDecoder->getCurFrame();
}

int Video::getNumFramesQueued() const
{
    exceptionIfUnloaded("getNumFramesQueued");
    return m_pDecoder->getNumFramesQueued();
}

void Video::seekToFrame(int FrameNum)
{
    exceptionIfUnloaded("seekToFrame");
    if (getCurFrame() != FrameNum) {
        long long DestTime = (long long)(FrameNum*1000.0/m_pDecoder->getNominalFPS());
        seek(DestTime);
    }
}

std::string Video::getStreamPixelFormat() const
{
    exceptionIfUnloaded("getStreamPixelFormat");
    return m_pDecoder->getVideoInfo().m_sPixelFormat;
}

long long Video::getDuration() const
{
    exceptionIfUnloaded("getDuration");
    return m_pDecoder->getVideoInfo().m_Duration;
}

int Video::getBitrate() const
{
    exceptionIfUnloaded("getBitrate");
    return m_pDecoder->getVideoInfo().m_Bitrate;
}

string Video::getVideoCodec() const
{
    exceptionIfUnloaded("getVideoCodec");
    return m_pDecoder->getVideoInfo().m_sVCodec;
}

std::string Video::getAudioCodec() const
{
    exceptionIfNoAudio("getAudioCodec");
    return m_pDecoder->getVideoInfo().m_sACodec;
}

int Video::getAudioSampleRate() const
{
    exceptionIfNoAudio("getAudioSampleRate");
    return m_pDecoder->getVideoInfo().m_SampleRate;
}

int Video::getNumAudioChannels() const
{
    exceptionIfNoAudio("getNumAudioChannels");
    return m_pDecoder->getVideoInfo().m_NumAudioChannels;
}

long long Video::getCurTime() const
{
    exceptionIfUnloaded("getCurTime");
    return m_pDecoder->getCurTime();
}

void Video::seekToTime(long long Time)
{
    exceptionIfUnloaded("seekToTime");
    seek(Time);
    m_bSeekPending = true;
}

bool Video::getLoop() const
{
    return m_bLoop;
}

bool Video::isThreaded() const
{
    return m_bThreaded;
}

bool Video::hasAudio() const
{
    exceptionIfUnloaded("hasAudio");
    return m_pDecoder->getVideoInfo().m_bHasAudio;
}

void Video::setEOFCallback(PyObject * pEOFCallback)
{
    if (m_pEOFCallback) {
        Py_DECREF(m_pEOFCallback);
    }
    Py_INCREF(pEOFCallback);
    m_pEOFCallback = pEOFCallback;
}

const string& Video::getHRef() const
{
    return m_Filename;
}

void Video::setHRef(const string& href)
{
    m_href = href;
    if (m_VideoState != Unloaded) {
        checkReload();
    }
}

double Video::getVolume()
{
    return m_Volume;
}

void Video::setVolume(double Volume)
{
    if (Volume < 0) {
        Volume = 0;
    }
    m_Volume = Volume;
    if (m_VideoState != Unloaded && hasAudio()) {
        m_pDecoder->setVolume(Volume);
    }
}

void Video::checkReload()
{
    string fileName (m_href);
    if (m_href != "") {
        initFilename(fileName);
        if (fileName != m_Filename) {
            changeVideoState(Unloaded);
            m_Filename = fileName;
            changeVideoState(Paused);
        }
    } else {
        changeVideoState(Unloaded);
        m_Filename = "";
    }
    RasterNode::checkReload();
}

void Video::onFrameEnd()
{
    if (m_bEOFPending) {
        onEOF();
        m_bEOFPending = false;
    }
}

int Video::fillAudioBuffer(AudioBufferPtr pBuffer)
{
    assert(m_bThreaded);
    if (m_VideoState == Playing) {
        return m_pDecoder->fillAudioBuffer(pBuffer);
    } else {
        return 0;
    }
}

void Video::changeVideoState(VideoState NewVideoState)
{
    if (m_VideoState == NewVideoState) {
        return;
    }
    if (m_VideoState == Unloaded) {
        open();
    }
    if (NewVideoState == Unloaded) {
        close();
    }
    if (getState() == NS_CANRENDER) {
        long long CurTime = Player::get()->getFrameTime(); 
        if (m_VideoState == Unloaded) {
            startDecoding();
            m_StartTime = CurTime;
            m_PauseTime = 0;
        }
        if (NewVideoState == Paused) {
            m_PauseStartTime = CurTime;
        } else if (NewVideoState == Playing && m_VideoState == Paused) {
            m_PauseTime += (CurTime-m_PauseStartTime
                    - (long long)(1000.0/m_pDecoder->getFPS()));
        }
    }
    m_VideoState = NewVideoState;
}

void Video::seek(long long DestTime) 
{
    m_pDecoder->seek(DestTime);
    m_StartTime = Player::get()->getFrameTime() - DestTime;
    m_PauseTime = 0;
    m_PauseStartTime = Player::get()->getFrameTime();
    m_bFrameAvailable = false;
    m_bSeekPending = true;
}

void Video::open() 
{
    m_FramesTooLate = 0;
    m_FramesInRowTooLate = 0;
    m_FramesPlayed = 0;
    m_pDecoder->open(m_Filename, m_bThreaded);
    m_pDecoder->setVolume(m_Volume);
    VideoInfo videoInfo = m_pDecoder->getVideoInfo();
    if (!videoInfo.m_bHasVideo) {
        throw Exception(AVG_ERR_VIDEO_GENERAL, 
                string("Video: Opening "+m_Filename+" failed. No video stream found."));
    }

    m_bFirstFrameDecoded = false;
    m_bFrameAvailable = false;
}

void Video::startDecoding()
{
    const AudioParams * pAP = 0;
    if (getAudioEngine()) {
        pAP = getAudioEngine()->getParams();
    }
    m_pDecoder->startDecoding(getDisplayEngine()->isUsingShaders(), pAP);
    VideoInfo videoInfo = m_pDecoder->getVideoInfo();
    if (m_FPS != 0.0) {
        if (videoInfo.m_bHasAudio) {
            AVG_TRACE(Logger::WARNING, 
                    getID() + ": Can't set FPS if video contains audio. Ignored.");
        } else {
            m_pDecoder->setFPS(m_FPS);
        }
    }
    if (videoInfo.m_bHasAudio && getAudioEngine()) {
        getAudioEngine()->addSource(this);
    }
    m_bSeekPending = true;
    
    setViewport(-32767, -32767, -32767, -32767);
    PixelFormat pf = getPixelFormat();
    getSurface()->create(videoInfo.m_Size, pf);
    if (pf == B8G8R8X8 || pf == B8G8R8A8) {
        FilterFill<Pixel32> Filter(Pixel32(0,0,0,255));
        Filter.applyInPlace(getSurface()->lockBmp());
        getSurface()->unlockBmps();
    }
}

void Video::close()
{
    if (hasAudio() && getAudioEngine()) {
        getAudioEngine()->removeSource(this);
    }
    m_pDecoder->close();
    if (m_FramesTooLate > 0) {
        AVG_TRACE(Logger::PROFILE, "Missed video frames for " << getID() << ": " 
                << m_FramesTooLate << " of " << m_FramesPlayed);
    }
}

PixelFormat Video::getPixelFormat() 
{
    return m_pDecoder->getPixelFormat();
}

IntPoint Video::getMediaSize()
{
    if (m_pDecoder && m_pDecoder->getState() != IVideoDecoder::CLOSED) {
        return m_pDecoder->getSize();
    } else {
        return IntPoint(0,0);
    }
}

double Video::getFPS() const
{
    return m_pDecoder->getFPS();
}

long long Video::getNextFrameTime() const
{
    switch (m_VideoState) {
        case Unloaded:
            return 0;
        case Paused:
            return m_PauseStartTime-m_StartTime;
        case Playing:
            return Player::get()->getFrameTime()-m_StartTime-m_PauseTime;
        default:
            assert(false);
            return 0;
    }
}

void Video::exceptionIfNoAudio(const std::string& sFuncName) const
{
    exceptionIfUnloaded(sFuncName);
    if (!hasAudio()) {
        throw Exception(AVG_ERR_VIDEO_GENERAL, 
                string("Video.")+sFuncName+" failed: no audio stream.");
    }
}

void Video::exceptionIfUnloaded(const std::string& sFuncName) const
{
    if (m_VideoState == Unloaded) {
        throw Exception(AVG_ERR_VIDEO_GENERAL, 
                string("Video.")+sFuncName+" failed: video not loaded.");
    }
}

void Video::preRender()
{
    Node::preRender();
    if (getEffectiveOpacity() <= 0.01 && m_VideoState == Playing) {
        // Throw away frames that are not visible to make sure the video keeps in sync.
        m_pDecoder->throwAwayFrame(getNextFrameTime());
    }
}

static ProfilingZone RenderProfilingZone("Video::render");

void Video::render(const DRect& Rect)
{
    switch(m_VideoState) {
        case Playing:
            {
                bool bNewFrame = renderToSurface(getSurface());
                m_bFrameAvailable = m_bFrameAvailable | bNewFrame;
                if (m_bFrameAvailable) {
                    m_bFirstFrameDecoded = true;
                }
                if (m_bFirstFrameDecoded) {
                    getSurface()->blt32(getSize(), getEffectiveOpacity(), getBlendMode());
                }
            }
            break;
        case Paused:
            if (!m_bFrameAvailable) {
                m_bFrameAvailable = renderToSurface(getSurface());
            }
            if (m_bFrameAvailable) {
                m_bFirstFrameDecoded = true;
            }
            if (m_bFirstFrameDecoded) {
                getSurface()->blt32(getSize(), getEffectiveOpacity(), getBlendMode());
            }
            break;
        case Unloaded:
            break;
    }
}

bool Video::renderToSurface(OGLTiledSurface * pSurface)
{
    ScopeTimer Timer(RenderProfilingZone);
    PixelFormat PF = m_pDecoder->getPixelFormat();
    FrameAvailableCode FrameAvailable;
    if (PF == YCbCr420p || PF == YCbCrJ420p) {
        BitmapPtr pBmp = pSurface->lockBmp(0);
        FrameAvailable = m_pDecoder->renderToYCbCr420p(pBmp,
                pSurface->lockBmp(1), pSurface->lockBmp(2), getNextFrameTime());
    } else {
        BitmapPtr pBmp = pSurface->lockBmp();
        FrameAvailable = m_pDecoder->renderToBmp(pBmp, getNextFrameTime());
    }
    pSurface->unlockBmps();
    if (FrameAvailable == FA_NEW_FRAME) {
        m_FramesPlayed++;
        m_FramesInRowTooLate = 0;
        pSurface->bind();
        m_bSeekPending = false;
        calcMaskPos();
    } else if (FrameAvailable == FA_STILL_DECODING) {
        m_FramesPlayed++;
        m_FramesTooLate++;
        m_FramesInRowTooLate++;
        if ((m_FramesInRowTooLate > 3) ||
            m_bSeekPending) 
        {
            // Heuristic: If we've missed more than 3 frames in a row, we stop
            // advancing movie time until the decoder has caught up.
            double framerate = Player::get()->getEffectiveFramerate();
            if (framerate != 0) {
                m_PauseTime += (long long)(1000/framerate);
            }
        }
//        AVG_TRACE(Logger::PROFILE, "Missed video frame.");
    } else if (FrameAvailable == FA_USE_LAST_FRAME) {
        m_FramesInRowTooLate = 0;
        m_bSeekPending = false;
//        AVG_TRACE(Logger::PROFILE, "Video frame reused.");
    }
    if (m_pDecoder->isEOF()) {
        m_bEOFPending = true;
        if (m_bLoop) {
            seek(0);
        } else {
            changeVideoState(Paused);
        }
    }
    return (FrameAvailable == FA_NEW_FRAME);
}

void Video::onEOF()
{
    if (m_pEOFCallback) {
        PyObject * arglist = Py_BuildValue("()");
        PyObject * result = PyEval_CallObject(m_pEOFCallback, arglist);
        Py_DECREF(arglist);    
        if (!result) {
            throw error_already_set();
        }
        Py_DECREF(result);
    }
}

}
