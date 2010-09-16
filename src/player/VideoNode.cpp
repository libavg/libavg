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
#include "VideoNode.h"
#include "DisplayEngine.h"
#include "Player.h"
#include "OGLSurface.h"
#include "NodeDefinition.h"
#include "SDLDisplayEngine.h"
#include "Canvas.h"

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

NodeDefinition VideoNode::createDefinition()
{
    return NodeDefinition("video", VisibleNode::buildNode<VideoNode>)
        .extendDefinition(RasterNode::createDefinition())
        .addArg(Arg<UTF8String>("href", "", false, offsetof(VideoNode, m_href)))
        .addArg(Arg<bool>("loop", false, false, offsetof(VideoNode, m_bLoop)))
        .addArg(Arg<bool>("threaded", true, false, offsetof(VideoNode, m_bThreaded)))
        .addArg(Arg<double>("fps", 0.0, false, offsetof(VideoNode, m_FPS)))
        .addArg(Arg<double>("volume", 1.0, false, offsetof(VideoNode, m_Volume)))
        ;
}

VideoNode::VideoNode(const ArgList& Args)
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

VideoNode::~VideoNode()
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

void VideoNode::setRenderingEngines(DisplayEngine * pDisplayEngine, 
        AudioEngine * pAudioEngine)
{
    checkReload();
    RasterNode::setRenderingEngines(pDisplayEngine, pAudioEngine);
    long long CurTime = Player::get()->getFrameTime(); 
    if (m_VideoState != Unloaded) {
        startDecoding();
    }
    if (m_VideoState == Paused) {
        m_PauseStartTime = CurTime;
    } 
}

void VideoNode::connect(CanvasPtr pCanvas)
{
    pCanvas->registerFrameEndListener(this);
    RasterNode::connect(pCanvas);
}

void VideoNode::disconnect(bool bKill)
{
    getCanvas()->unregisterFrameEndListener(this);
    if (bKill) {
        setEOFCallback(Py_None);
    }
    changeVideoState(Unloaded);
    RasterNode::disconnect(bKill);
}

void VideoNode::play()
{
    changeVideoState(Playing);
}

void VideoNode::stop()
{
    changeVideoState(Unloaded);
}

void VideoNode::pause()
{
    changeVideoState(Paused);
}

int VideoNode::getNumFrames() const
{
    exceptionIfUnloaded("getNumFrames");
    return m_pDecoder->getVideoInfo().m_NumFrames;
}

int VideoNode::getCurFrame() const
{
    exceptionIfUnloaded("getCurFrame");
    int curFrame = m_pDecoder->getCurFrame();
    if (curFrame > 0) {
        return curFrame;
    } else {
        return 0;
    }
}

int VideoNode::getNumFramesQueued() const
{
    exceptionIfUnloaded("getNumFramesQueued");
    return m_pDecoder->getNumFramesQueued();
}

void VideoNode::seekToFrame(int FrameNum)
{
    if (FrameNum < 0) {
        throw Exception(AVG_ERR_OUT_OF_RANGE,
                "Can't seek to a negative frame in a video.");
    }
    exceptionIfUnloaded("seekToFrame");
    if (getCurFrame() != FrameNum) {
        long long DestTime = (long long)(FrameNum*1000.0/m_pDecoder->getNominalFPS());
        seek(DestTime);
    }
}

std::string VideoNode::getStreamPixelFormat() const
{
    exceptionIfUnloaded("getStreamPixelFormat");
    return m_pDecoder->getVideoInfo().m_sPixelFormat;
}

long long VideoNode::getDuration() const
{
    exceptionIfUnloaded("getDuration");
    return m_pDecoder->getVideoInfo().m_Duration;
}

int VideoNode::getBitrate() const
{
    exceptionIfUnloaded("getBitrate");
    return m_pDecoder->getVideoInfo().m_Bitrate;
}

string VideoNode::getVideoCodec() const
{
    exceptionIfUnloaded("getVideoCodec");
    return m_pDecoder->getVideoInfo().m_sVCodec;
}

std::string VideoNode::getAudioCodec() const
{
    exceptionIfNoAudio("getAudioCodec");
    return m_pDecoder->getVideoInfo().m_sACodec;
}

int VideoNode::getAudioSampleRate() const
{
    exceptionIfNoAudio("getAudioSampleRate");
    return m_pDecoder->getVideoInfo().m_SampleRate;
}

int VideoNode::getNumAudioChannels() const
{
    exceptionIfNoAudio("getNumAudioChannels");
    return m_pDecoder->getVideoInfo().m_NumAudioChannels;
}

long long VideoNode::getCurTime() const
{
    exceptionIfUnloaded("getCurTime");
    long long curTime = m_pDecoder->getCurTime();
    if (curTime > 0) {
        return curTime;
    } else {
        return 0;
    }
}

void VideoNode::seekToTime(long long Time)
{
    if (Time < 0) {
        throw Exception(AVG_ERR_OUT_OF_RANGE,
                "Can't seek to a negative time in a video.");
    }
    exceptionIfUnloaded("seekToTime");
    seek(Time);
    m_bSeekPending = true;
}

bool VideoNode::getLoop() const
{
    return m_bLoop;
}

bool VideoNode::isThreaded() const
{
    return m_bThreaded;
}

bool VideoNode::hasAudio() const
{
    exceptionIfUnloaded("hasAudio");
    return m_pDecoder->getVideoInfo().m_bHasAudio;
}

bool VideoNode::hasAlpha() const
{
    exceptionIfUnloaded("hasAlpha");
    PixelFormat pf = getPixelFormat();
    return Bitmap::pixelFormatHasAlpha(pf); 
}

void VideoNode::setEOFCallback(PyObject * pEOFCallback)
{
    if (m_pEOFCallback) {
        Py_DECREF(m_pEOFCallback);
    }
    if (pEOFCallback == Py_None) {
        m_pEOFCallback = 0;
    } else {
        Py_INCREF(pEOFCallback);
        m_pEOFCallback = pEOFCallback;
    }
}

const UTF8String& VideoNode::getHRef() const
{
    return m_href;
}

void VideoNode::setHRef(const UTF8String& href)
{
    m_href = href;
    checkReload();
}

double VideoNode::getVolume()
{
    return m_Volume;
}

void VideoNode::setVolume(double Volume)
{
    if (Volume < 0) {
        Volume = 0;
    }
    m_Volume = Volume;
    if (m_VideoState != Unloaded && hasAudio()) {
        m_pDecoder->setVolume(Volume);
    }
}

void VideoNode::checkReload()
{
    string fileName (m_href);
    if (m_href != "") {
        initFilename(fileName);
        if (fileName != m_Filename && m_VideoState != Unloaded) {
            changeVideoState(Unloaded);
            m_Filename = fileName;
            changeVideoState(Paused);
        } else {
            m_Filename = fileName;
        }
    } else {
        changeVideoState(Unloaded);
        m_Filename = "";
    }
    RasterNode::checkReload();
}

void VideoNode::onFrameEnd()
{
    if (m_bEOFPending) {
        // If the VideoNode is unlinked by python in onEOF, the following line prevents
        // the object from being deleted until we return from this function.
        NodePtr pTempThis = getThis();
        m_bEOFPending = false;
        onEOF();
    }
}

int VideoNode::fillAudioBuffer(AudioBufferPtr pBuffer)
{
    AVG_ASSERT(m_bThreaded);
    if (m_VideoState == Playing) {
        return m_pDecoder->fillAudioBuffer(pBuffer);
    } else {
        return 0;
    }
}

void VideoNode::changeVideoState(VideoState NewVideoState)
{
    long long CurTime = Player::get()->getFrameTime(); 
    if (m_VideoState == NewVideoState) {
        return;
    }
    if (m_VideoState == Unloaded) {
        m_PauseStartTime = CurTime;
        open();
    }
    if (NewVideoState == Unloaded) {
        close();
    }
    if (getState() == NS_CANRENDER) {
        long long CurTime = Player::get()->getFrameTime(); 
        if (m_VideoState == Unloaded) {
            startDecoding();
        }
        if (NewVideoState == Paused) {
            m_PauseStartTime = CurTime;
        } else if (NewVideoState == Playing && m_VideoState == Paused) {
/*            
            cerr << "Play after pause:" << endl;
            cerr << "  getFrameTime()=" << CurTime << endl;
            cerr << "  m_PauseStartTime=" << m_PauseStartTime << endl;
            cerr << "  offset=" << (1000.0/m_pDecoder->getFPS()) << endl;
*/
            m_PauseTime += (CurTime-m_PauseStartTime
                    - (long long)(1000.0/m_pDecoder->getFPS()));
        }
    }
    m_VideoState = NewVideoState;
}

void VideoNode::seek(long long DestTime) 
{
    m_pDecoder->seek(DestTime);
    m_StartTime = Player::get()->getFrameTime() - DestTime;
    m_PauseTime = 0;
    m_PauseStartTime = Player::get()->getFrameTime();
    m_bFrameAvailable = false;
    m_bSeekPending = true;
}

void VideoNode::open() 
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
    m_StartTime = Player::get()->getFrameTime();
    m_PauseTime = 0;

    m_bFirstFrameDecoded = false;
    m_bFrameAvailable = false;
}

void VideoNode::startDecoding()
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

void VideoNode::close()
{
    if (hasAudio() && getAudioEngine()) {
        getAudioEngine()->removeSource(this);
    }
    m_pDecoder->close();
    if (m_FramesTooLate > 0) {
        string sID;
        if (getID() == "") {
            sID = m_href; 
        } else {
            sID = getID();
        }
        AVG_TRACE(Logger::PROFILE_VIDEO, "Missed video frames for '" << sID << "': " 
                << m_FramesTooLate << " of " << m_FramesPlayed);
        m_FramesTooLate = 0;
    }
}

PixelFormat VideoNode::getPixelFormat() const 
{
    return m_pDecoder->getPixelFormat();
}

IntPoint VideoNode::getMediaSize()
{
    if (m_pDecoder && m_pDecoder->getState() != IVideoDecoder::CLOSED) {
        return m_pDecoder->getSize();
    } else {
        return IntPoint(0,0);
    }
}

double VideoNode::getFPS() const
{
    return m_pDecoder->getFPS();
}

long long VideoNode::getNextFrameTime() const
{
    switch (m_VideoState) {
        case Unloaded:
            return 0;
        case Paused:
            return m_PauseStartTime-m_StartTime;
        case Playing:
            if (Player::get()->getFrameTime()-m_StartTime-m_PauseTime < 0) {
                cerr << "getNextFrameTime < 0" << endl;
                cerr << "getFrameTime(): " << Player::get()->getFrameTime() << endl;
                cerr << "m_StartTime: " << m_StartTime << endl;
                cerr << "m_PauseTime: " << m_PauseTime << endl;
            }
            return Player::get()->getFrameTime()-m_StartTime-m_PauseTime;
        default:
            AVG_ASSERT(false);
            return 0;
    }
}

void VideoNode::exceptionIfNoAudio(const std::string& sFuncName) const
{
    exceptionIfUnloaded(sFuncName);
    if (!hasAudio()) {
        throw Exception(AVG_ERR_VIDEO_GENERAL, 
                string("VideoNode.")+sFuncName+" failed: no audio stream.");
    }
}

void VideoNode::exceptionIfUnloaded(const std::string& sFuncName) const
{
    if (m_VideoState == Unloaded) {
        throw Exception(AVG_ERR_VIDEO_GENERAL, 
                string("VideoNode.")+sFuncName+" failed: video not loaded.");
    }
}

void VideoNode::preRender()
{
    VisibleNode::preRender();
    if (getEffectiveOpacity() <= 0.01 && m_VideoState == Playing) {
        // Throw away frames that are not visible to make sure the video keeps in sync.
        m_pDecoder->throwAwayFrame(getNextFrameTime());

       if (m_pDecoder->isEOF()) {
           updateStatusDueToDecoderEOF();
       }
    }
}

static ProfilingZone RenderProfilingZone("VideoNode::render");

void VideoNode::render(const DRect& Rect)
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
                    blt32(getSize(), getEffectiveOpacity(), getBlendMode());
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
                blt32(getSize(), getEffectiveOpacity(), getBlendMode());
            }
            break;
        case Unloaded:
            break;
    }
}

bool VideoNode::renderToSurface(OGLSurface * pSurface)
{
    ScopeTimer Timer(RenderProfilingZone);
    PixelFormat PF = m_pDecoder->getPixelFormat();
    FrameAvailableCode frameAvailable;
    if (PF == YCbCr420p || PF == YCbCrJ420p) {
        BitmapPtr pBmp = pSurface->lockBmp(0);
        frameAvailable = m_pDecoder->renderToYCbCr420p(pBmp,
                pSurface->lockBmp(1), pSurface->lockBmp(2), getNextFrameTime());
    } else {
        BitmapPtr pBmp = pSurface->lockBmp();
        frameAvailable = m_pDecoder->renderToBmp(pBmp, getNextFrameTime());
    }
    pSurface->unlockBmps();
    switch (frameAvailable) {
        case FA_NEW_FRAME:
            m_FramesPlayed++;
            m_FramesInRowTooLate = 0;
            bind();
            m_bSeekPending = false;
            setMaskCoords();
            break;
        case FA_STILL_DECODING:
            {
                m_FramesPlayed++;
                m_FramesTooLate++;
                m_FramesInRowTooLate++;
                double framerate = Player::get()->getEffectiveFramerate();
                long long frameTime = Player::get()->getFrameTime();
                if (m_VideoState == Playing) {
                    if (m_FramesInRowTooLate > 3 && framerate != 0) {
                        // Heuristic: If we've missed more than 3 frames in a row, we stop
                        // advancing movie time until the decoder has caught up.
                        m_PauseTime += (long long)(1000/framerate);
                    }
                    if (m_bSeekPending) {
                        // The movie time also stays still while waiting for a seek to 
                        // complete.
                        m_PauseTime = frameTime-m_PauseStartTime;
                    }
                    long long curMovieTime = 
                            Player::get()->getFrameTime()-m_StartTime-m_PauseTime;
                    if (curMovieTime < 0) {
                        cerr << "----------- curTime < 0 -------------" << endl;
                        cerr << "FramesPlayed=" << m_FramesPlayed << endl;
                        cerr << "getFrameTime()=" << Player::get()->getFrameTime()
                                << endl;
                        cerr << "m_StartTime=" << m_StartTime << endl;
                        cerr << "m_PauseTime=" << m_PauseTime << endl;
                        m_PauseTime = Player::get()->getFrameTime()-m_StartTime;
                    }
                }
            }
//            AVG_TRACE(Logger::PROFILE, "Missed video frame.");
            break;
        case FA_USE_LAST_FRAME:
            m_FramesInRowTooLate = 0;
            m_bSeekPending = false;
//            AVG_TRACE(Logger::PROFILE, "Video frame reused.");
            break;
        default:
            AVG_ASSERT(false);
    }

    if (m_pDecoder->isEOF()) {
        updateStatusDueToDecoderEOF();
    }

    return (frameAvailable == FA_NEW_FRAME);
}

void VideoNode::onEOF()
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


void VideoNode::updateStatusDueToDecoderEOF()
{
    m_bEOFPending = true;
    if (m_bLoop) {
        seek(0);
    } else {
        changeVideoState(Paused);
    }
}


}

