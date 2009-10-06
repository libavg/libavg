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

#include "../base/Exception.h"
#include "../base/Logger.h"
#include "../base/ScopeTimer.h"
#include "../base/XMLHelper.h"

#include "../graphics/Filterflipuv.h"

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
        .extendDefinition(VideoBase::createDefinition())
        .addArg(Arg<string>("href", "", false, offsetof(Video, m_href)))
        .addArg(Arg<bool>("loop", false, false, offsetof(Video, m_bLoop)))
        .addArg(Arg<bool>("threaded", true, false, offsetof(Video, m_bThreaded)))
        .addArg(Arg<double>("fps", 0.0, false, offsetof(Video, m_FPS)))
        ;
}

Video::Video(const ArgList& Args)
    : m_Filename(""),
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
    Player::get()->registerFrameEndListener(this);
    ObjectCounter::get()->incRef(&typeid(*this));
}

Video::~Video()
{
    Player::get()->unregisterFrameEndListener(this);
    if (m_pDecoder) {
        delete m_pDecoder;
        m_pDecoder = 0;
    }
    if (m_pEOFCallback) {
        Py_DECREF(m_pEOFCallback);
    }
    ObjectCounter::get()->decRef(&typeid(*this));
}

int Video::getNumFrames() const
{
    return m_pDecoder->getNumFrames();
}

int Video::getCurFrame() const
{
    if (getVideoState() != Unloaded) {
        return m_pDecoder->getCurFrame();
    } else {
        AVG_TRACE(Logger::WARNING, 
                "Error in Video::GetCurFrame: Video not loaded.");
        return -1;
    }
}

int Video::getNumFramesQueued() const
{
    return m_pDecoder->getNumFramesQueued();
}

void Video::seekToFrame(int FrameNum)
{
    if (getVideoState() != Unloaded) {
        if (getCurFrame() != FrameNum) {
            long long DestTime = (long long)(FrameNum*1000.0/m_pDecoder->getNominalFPS());
            seek(DestTime);
        }
    } else {
        AVG_TRACE(Logger::WARNING, 
                "Error in Video::SeekToTime: Video "+getID()+" not loaded.");
    }
}

long long Video::getDuration() const
{
    if (getVideoState() != Unloaded) {
        return m_pDecoder->getDuration();
    } else {
        AVG_TRACE(Logger::WARNING,
               "Error in Video::getDuration: Video not loaded.");
        return -1;
    }
}

long long Video::getCurTime() const
{
    if (getVideoState() != Unloaded) {
        return m_pDecoder->getCurTime();
    } else {
        AVG_TRACE(Logger::WARNING, 
                "Error in Video::GetCurTime: Video not loaded.");
        return -1;
    }
}

void Video::seekToTime(long long Time)
{
    if (getVideoState() != Unloaded) {
        seek(Time);
        m_bSeekPending = true;
    } else {
        AVG_TRACE(Logger::WARNING, 
                "Error in Video::SeekToTime: Video "+getID()+" not loaded.");
    }
}

bool Video::getLoop() const
{
    return m_bLoop;
}

bool Video::isThreaded() const
{
    return m_bThreaded;
}

void Video::setEOFCallback(PyObject * pEOFCallback)
{
    if (m_pEOFCallback) {
        Py_DECREF(m_pEOFCallback);
    }
    Py_INCREF(pEOFCallback);
    m_pEOFCallback = pEOFCallback;
}

void Video::setRenderingEngines(DisplayEngine * pDisplayEngine, AudioEngine * pAudioEngine)
{
    checkReload();
    VideoBase::setRenderingEngines(pDisplayEngine, pAudioEngine);
}

const string& Video::getHRef() const
{
    return m_Filename;
}

void Video::setHRef(const string& href)
{
    m_href = href;
    checkReload();
}

void Video::setVolume(double Volume)
{
    if (Volume < 0) {
        Volume = 0;
    }
    m_Volume = Volume;
    if (m_pDecoder) {
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
    if (getVideoState() == Playing) {
        return m_pDecoder->fillAudioBuffer(pBuffer);
    } else {
        return 0;
    }
}

void Video::changeVideoState(VideoState NewVideoState)
{
    if (getState() == NS_CANRENDER) {
        long long CurTime = Player::get()->getFrameTime(); 
        if (NewVideoState != getVideoState()) {
            if (getVideoState() == Unloaded) {
                m_StartTime = CurTime;
                m_PauseTime = 0;
            }
            if (NewVideoState == Paused) {
                m_PauseStartTime = CurTime;
            } else if (NewVideoState == Playing && getVideoState() == Paused) {
                m_PauseTime += (CurTime-m_PauseStartTime
                        - (long long)(1000.0/m_pDecoder->getFPS()));
            }
        }
    }
    VideoBase::changeVideoState(NewVideoState);
}

void Video::seek(long long DestTime) 
{
    m_pDecoder->seek(DestTime);
    m_StartTime = Player::get()->getFrameTime() - DestTime;
    m_PauseTime = 0;
    m_PauseStartTime = Player::get()->getFrameTime();
    setFrameAvailable(false);
    m_bSeekPending = true;
}

void Video::open(bool bUseYCbCrShaders)
{
    m_FramesTooLate = 0;
    m_FramesInRowTooLate = 0;
    m_FramesPlayed = 0;
    const AudioParams * pAP = 0;
    if (getAudioEngine()) {
        pAP = getAudioEngine()->getParams();
    }
    m_pDecoder->open(m_Filename, pAP, bUseYCbCrShaders, m_bThreaded);
    m_pDecoder->setVolume(m_Volume);
    if (m_FPS != 0.0) {
        if (m_pDecoder->hasAudio()) {
            AVG_TRACE(Logger::WARNING, 
                    getID() + ": Can't set FPS if video contains audio. Ignored.");
        } else {
            m_pDecoder->setFPS(m_FPS);
        }
    }
    if (m_pDecoder->hasAudio()) {
        getAudioEngine()->addSource(this);
    }
    m_bSeekPending = true;
}

void Video::close()
{
    if (m_pDecoder->hasAudio()) {
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
    if (m_pDecoder)  {
        return m_pDecoder->getSize();
    } else {
        return IntPoint(0,0);
    }
}

double Video::getFPS()
{
    return m_pDecoder->getFPS();
}

long long Video::getNextFrameTime()
{
    switch (getVideoState()) {
        case Unloaded:
            return 0;
        case Paused:
            return m_PauseStartTime-m_StartTime;
        case Playing:
            if (m_pDecoder->getMasterStream() == SS_AUDIO) {
                // Sync to audio if possible.
                return m_pDecoder->getCurTime(SS_AUDIO);
            } else {
                return Player::get()->getFrameTime()-m_StartTime-m_PauseTime;
            }
        default:
            assert(false);
            return 0;
    }
}

void Video::preRender()
{
    Node::preRender();
    if (getEffectiveOpacity() <= 0.01 && getVideoState() == Playing) {
        // Throw away frames that are not visible to make sure the video keeps in sync.
        m_pDecoder->throwAwayFrame(getNextFrameTime());
    }
}

static ProfilingZone RenderProfilingZone("Video::render");

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
    } else if (FrameAvailable == FA_STILL_DECODING) {
        m_FramesPlayed++;
        m_FramesTooLate++;
        m_FramesInRowTooLate++;
        if ((m_FramesInRowTooLate > 3 && m_pDecoder->getMasterStream() != SS_AUDIO) ||
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
