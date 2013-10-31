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

#include "SyncVideoDecoder.h"
#include "FFMpegDemuxer.h"

#include "../base/Exception.h"
#include "../base/Logger.h"
#include "../base/ScopeTimer.h"
#include "../base/ObjectCounter.h"
#include "../base/ProfilingZoneID.h"
#include "../base/StringHelper.h"

#include "../graphics/BitmapLoader.h"

#include <iostream>
#include <sstream>
#ifndef _WIN32
#include <unistd.h>
#endif

using namespace std;

namespace avg {

SyncVideoDecoder::SyncVideoDecoder()
    : m_pDemuxer(0),
      m_bFirstPacket(false),
      m_bUseStreamFPS(true),
      m_FPS(0)
{
    ObjectCounter::get()->incRef(&typeid(*this));
}

SyncVideoDecoder::~SyncVideoDecoder()
{
    ObjectCounter::get()->decRef(&typeid(*this));
}

void SyncVideoDecoder::open(const string& sFilename, bool bUseHardwareAcceleration, 
        bool bEnableSound)
{
    m_bProcessingLastFrames = false;
    VideoDecoder::open(sFilename, false, false);

    if (getVStreamIndex() >= 0) {
        if (m_bUseStreamFPS) {
            m_FPS = getStreamFPS();
        }
        m_bFirstPacket = true;
        m_bVideoSeekDone = false;
    }
}

void SyncVideoDecoder::startDecoding(bool bDeliverYCbCr, const AudioParams* pAP)
{
    VideoDecoder::startDecoding(bDeliverYCbCr, 0);

    AVG_ASSERT(!m_pDemuxer);
    vector<int> streamIndexes;
    streamIndexes.push_back(getVStreamIndex());
    m_pDemuxer = new FFMpegDemuxer(getFormatContext(), streamIndexes);

    m_pFrameDecoder = FFMpegFrameDecoderPtr(new FFMpegFrameDecoder(getVideoStream()));
    m_pFrameDecoder->setFPS(m_FPS);
#if LIBAVCODEC_VERSION_INT > AV_VERSION_INT(54, 28, 0) 
    m_pFrame = avcodec_alloc_frame();
#else
    m_pFrame = new AVFrame;
#endif
}

void SyncVideoDecoder::close() 
{
    delete m_pDemuxer;
    m_pDemuxer = 0;

    m_pFrameDecoder = FFMpegFrameDecoderPtr();
    VideoDecoder::close();
#if LIBAVCODEC_VERSION_INT > AV_VERSION_INT(54, 28, 0) 
    avcodec_free_frame(&m_pFrame);
#else
    delete m_pFrame;
#endif
}

void SyncVideoDecoder::seek(float destTime) 
{
    AVG_ASSERT(getState() == DECODING);

    if (m_bFirstPacket) {
        readFrame(m_pFrame);
    }
    m_pDemuxer->seek(destTime);
    m_bVideoSeekDone = true;
    m_pFrameDecoder->handleSeek();
}

void SyncVideoDecoder::loop()
{
    seek(0);
}

int SyncVideoDecoder::getCurFrame() const
{
    return int(getCurTime()*getStreamFPS()+0.49);
}

int SyncVideoDecoder::getNumFramesQueued() const
{
    return 0;
}

float SyncVideoDecoder::getCurTime() const
{
    AVG_ASSERT(getState() != CLOSED);
    if (m_pFrameDecoder) {
        return m_pFrameDecoder->getCurTime();
    } else {
        return 0;
    }
}

float SyncVideoDecoder::getFPS() const
{
    AVG_ASSERT(getState() != CLOSED);
    return m_FPS;
}

void SyncVideoDecoder::setFPS(float fps)
{
    m_bUseStreamFPS = (fps == 0);
    if (fps == 0) {
        m_FPS = getStreamFPS();
    } else {
        m_FPS = fps;
    }
    if (m_pFrameDecoder) {
        m_pFrameDecoder->setFPS(m_FPS);
    }
}

static ProfilingZoneID RenderToBmpProfilingZone("FFMpeg: renderToBmp", true);
static ProfilingZoneID CopyImageProfilingZone("FFMpeg: copy image", true);

FrameAvailableCode SyncVideoDecoder::renderToBmps(vector<BitmapPtr>& pBmps, 
        float timeWanted)
{
    AVG_ASSERT(getState() == DECODING);
    ScopeTimer timer(RenderToBmpProfilingZone);
    FrameAvailableCode frameAvailable;
    if (timeWanted == -1) {
        readFrame(m_pFrame);
        frameAvailable = FA_NEW_FRAME;
    } else {
        frameAvailable = readFrameForTime(m_pFrame, timeWanted);
    }
    if (frameAvailable == FA_USE_LAST_FRAME || isEOF()) {
        return FA_USE_LAST_FRAME;
    } else {
        if (pixelFormatIsPlanar(getPixelFormat())) {
            ScopeTimer timer(CopyImageProfilingZone);
            for (unsigned i = 0; i < pBmps.size(); ++i) {
                m_pFrameDecoder->copyPlaneToBmp(pBmps[i], m_pFrame->data[i],
                        m_pFrame->linesize[i]);
            }
        } else {
            m_pFrameDecoder->convertFrameToBmp(m_pFrame, pBmps[0]);
        }
        return FA_NEW_FRAME;
    }
}

void SyncVideoDecoder::throwAwayFrame(float timeWanted)
{
    AVG_ASSERT(getState() == DECODING);
    readFrameForTime(m_pFrame, timeWanted);
}

bool SyncVideoDecoder::isEOF() const
{
    AVG_ASSERT(getState() == DECODING);
    return m_pFrameDecoder->isEOF() && !m_bProcessingLastFrames;
}

FrameAvailableCode SyncVideoDecoder::readFrameForTime(AVFrame* pFrame, float timeWanted)
{
    AVG_ASSERT(getState() == DECODING);
    float timePerFrame = 1.0f/m_FPS;
    if (!m_bVideoSeekDone && timeWanted-m_pFrameDecoder->getCurTime() < 0.5f*timePerFrame)
    {
        // The last frame is still current. Display it again.
        return FA_USE_LAST_FRAME;
    } else {
        bool bInvalidFrame = true;
        while (bInvalidFrame && !isEOF()) {
            readFrame(pFrame);
            bInvalidFrame = m_pFrameDecoder->getCurTime()-timeWanted < -0.5f*timePerFrame;
        }
    }
    if (m_bVideoSeekDone) {
        m_bVideoSeekDone = false;
    }
    return FA_NEW_FRAME;
}

static ProfilingZoneID DecodeProfilingZone("FFMpeg: decode", true);

void SyncVideoDecoder::readFrame(AVFrame* pFrame)
{
    AVG_ASSERT(getState() == DECODING);
    ScopeTimer timer(DecodeProfilingZone); 

    if (m_bProcessingLastFrames) {
        // EOF received, but last frames still need to be decoded.
        bool bGotPicture = m_pFrameDecoder->decodeLastFrame(pFrame);
        if (!bGotPicture) {
            m_bProcessingLastFrames = false;
        }
    } else {        
        bool bDone = false;
        while (!bDone) {
            AVPacket* pPacket = m_pDemuxer->getPacket(getVStreamIndex());
            m_bFirstPacket = false;
            bool bGotPicture;
            if (pPacket) {
                bGotPicture = m_pFrameDecoder->decodePacket(pPacket, pFrame, 
                        m_bVideoSeekDone);
            } else {
                bGotPicture = m_pFrameDecoder->decodeLastFrame(pFrame);
            }
            if (bGotPicture && m_pFrameDecoder->isEOF()) {
                m_bProcessingLastFrames = true;
            }
            if (bGotPicture || m_pFrameDecoder->isEOF()) {
                bDone = true;
            }
        }
    }
}

}

