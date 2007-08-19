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

#include "Video.h"
#include "DisplayEngine.h"
#include "Player.h"
#include "ISurface.h"

#include "../base/Exception.h"
#include "../base/Logger.h"
#include "../base/ScopeTimer.h"
#include "../base/XMLHelper.h"

#include "../graphics/Filterflipuv.h"

#include "../video/AsyncVideoDecoder.h"
#include "../video/FFMpegDecoder.h"

#include <boost/python.hpp>

#include <iostream>
#include <sstream>
#include <unistd.h>

using namespace boost::python;
using namespace std;

namespace avg {

bool Video::m_bInitialized = false;

Video::Video (const xmlNodePtr xmlNode, Player * pPlayer)
    : VideoBase(xmlNode, pPlayer),
      m_Filename(""),
      m_pEOFCallback(0),
      m_FramesTooLate(0),
      m_FramesPlayed(0),
      m_pDecoder(0)
{
    m_href = getDefaultedStringAttr (xmlNode, "href", "");
    m_bLoop = getDefaultedBoolAttr (xmlNode, "loop", false);
    m_bThreaded = getDefaultedBoolAttr (xmlNode, "threaded", false);
    m_FPS = getDefaultedDoubleAttr (xmlNode, "fps", false);
    m_Filename = m_href;
    if (m_Filename != "") {
        initFilename(getPlayer(), m_Filename);
    }
    if (m_bThreaded) {
        VideoDecoderPtr pSyncDecoder = VideoDecoderPtr(new FFMpegDecoder());
        m_pDecoder = new AsyncVideoDecoder(pSyncDecoder);
    } else {
        m_pDecoder = new FFMpegDecoder();
    }
}

Video::~Video ()
{
    if (m_pDecoder) {
        delete m_pDecoder;
        m_pDecoder = 0;
    }
    if (m_pEOFCallback) {
        Py_DECREF(m_pEOFCallback);
    }
}

int Video::getNumFrames() const
{
    if (getVideoState() != Unloaded) {
        return m_pDecoder->getNumFrames();
    } else {
        AVG_TRACE(Logger::WARNING,
               "Error in Video::getNumFrames: Video not loaded.");
        return -1;
    }
}

int Video::getCurFrame() const
{
    if (getVideoState() != Unloaded) {
        return m_CurFrame;
    } else {
        AVG_TRACE(Logger::WARNING, 
                "Error in Video::GetCurFrame: Video not loaded.");
        return -1;
    }
}

void Video::seekToFrame(int FrameNum)
{
    if (getVideoState() != Unloaded) {
        if (FrameNum != m_CurFrame) {
            seek(FrameNum);
        }
    } else {
        AVG_TRACE(Logger::WARNING, 
                "Error in Video::SeekToFrame: Video "+getID()+" not loaded.");
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

void Video::setDisplayEngine(DisplayEngine * pEngine)
{
    VideoBase::setDisplayEngine(pEngine);
}

void Video::disconnect()
{
    stop();
    VideoBase::disconnect();
}

const string& Video::getHRef() const
{
    return m_Filename;
}

void Video::setHRef(const string& href)
{
    string fileName (href);
    m_href = href;
    if (m_href != "") {
        initFilename(getPlayer(), fileName);
        if (fileName != m_Filename) {
            changeVideoState(Unloaded);
            m_Filename = fileName;
            changeVideoState(Paused);
        }
    } else {
        changeVideoState(Unloaded);
        m_Filename = "";
    }
}

string Video::getTypeStr ()
{
    return "Video";
}

void Video::changeVideoState(VideoState NewVideoState)
{
    long long CurTime = getPlayer()->getFrameTime(); 
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
    VideoBase::changeVideoState(NewVideoState);
}

void Video::seek(int DestFrame) 
{

    m_pDecoder->seek(DestFrame);
    m_StartTime = getPlayer()->getFrameTime()
            -(long long)((DestFrame*1000.0)/m_pDecoder->getFPS());
    m_PauseTime = 0;
    m_PauseStartTime = getPlayer()->getFrameTime();
    m_CurFrame = DestFrame;
    setFrameAvailable(false);
}

        
void Video::open(YCbCrMode ycbcrMode)
{
    m_CurFrame = 0;
    m_FramesTooLate = 0;
    m_FramesPlayed = 0;
    m_pDecoder->open(m_Filename, ycbcrMode, m_bThreaded);
    m_pDecoder->setFPS(m_FPS);
}

void Video::close()
{
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

IntPoint Video::getSize()
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

long long Video::getCurTime()
{
    switch (getVideoState()) {
        case Unloaded:
            return 0;
        case Paused:
            return m_PauseStartTime-m_StartTime;
        case Playing:
            return getPlayer()->getFrameTime()-m_StartTime-m_PauseTime;
        default:
            assert(false);
            return 0;
    }
}

static ProfilingZone RenderProfilingZone("Video::render");

bool Video::renderToSurface(ISurface * pSurface)
{
    ScopeTimer Timer(RenderProfilingZone);
    PixelFormat PF = m_pDecoder->getPixelFormat();
    FrameAvailableCode FrameAvailable;
    if (PF == YCbCr420p || PF == YCbCrJ420p) {
        BitmapPtr pBmp = pSurface->lockBmp(0);
        FrameAvailable = m_pDecoder->renderToYCbCr420p(pBmp,
                pSurface->lockBmp(1), pSurface->lockBmp(2), getCurTime());
    } else {
        BitmapPtr pBmp = pSurface->lockBmp();
        FrameAvailable = m_pDecoder->renderToBmp(pBmp, getCurTime());
//        DisplayEngine::YCbCrMode ycbcrMode = getEngine()->getYCbCrMode();
//        if (ycbcrMode == DisplayEngine::OGL_MESA && pBmp->getPixelFormat() == YCbCr422) {
//            FilterFlipUV().applyInPlace(pBmp);
//        }   
    }
    pSurface->unlockBmps();
    if (FrameAvailable == FA_NEW_FRAME) {
        m_FramesPlayed++;
        getEngine()->surfaceChanged(pSurface);
        m_CurFrame++;
    } else if (FrameAvailable == FA_STILL_DECODING) {
        m_FramesPlayed++;
        m_FramesTooLate++;
//        AVG_TRACE(Logger::PROFILE, "Missed video frame.");
//    } else if (FrameAvailable == FA_USE_LAST_FRAME) {
//        AVG_TRACE(Logger::PROFILE, "Video frame reused.");
    }
    if (m_pDecoder->isEOF()) {
        onEOF();
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
