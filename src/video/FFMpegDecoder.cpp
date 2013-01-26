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

#include "FFMpegDecoder.h"
#include "AsyncDemuxer.h"
#ifdef AVG_ENABLE_VDPAU
#include "VDPAUDecoder.h"
#endif

#include "../base/Exception.h"
#include "../base/Logger.h"
#include "../base/ScopeTimer.h"
#include "../base/ObjectCounter.h"
#include "../base/ProfilingZoneID.h"
#include "../base/StringHelper.h"

#include <iostream>
#include <sstream>
#ifndef _WIN32
#include <unistd.h>
#endif

using namespace std;
using namespace boost;

namespace avg {

FFMpegDecoder::FFMpegDecoder(AsyncDemuxer* pDemuxer, AVStream* pStream, int streamIndex, 
        PixelFormat pf, bool bUseVDPAU)
    : m_bVideoSeekDone(false),
      m_pDemuxer(pDemuxer),
      m_pStream(pStream),
      m_StreamIndex(streamIndex),
      m_PF(pf),
      m_bUseVDPAU(bUseVDPAU),
      m_bEOFPending(false)
{
    m_pFrameDecoder = FFMpegFrameDecoderPtr(new FFMpegFrameDecoder(pStream));
    ObjectCounter::get()->incRef(&typeid(*this));
}

FFMpegDecoder::~FFMpegDecoder()
{
    ObjectCounter::get()->decRef(&typeid(*this));
}

float FFMpegDecoder::getCurTime() const
{
    return m_pFrameDecoder->getCurTime();
}

void FFMpegDecoder::setFPS(float fps)
{
    m_pFrameDecoder->setFPS(fps);
}

static ProfilingZoneID RenderToBmpProfilingZone("FFMpeg: renderToBmp", true);
static ProfilingZoneID CopyImageProfilingZone("FFMpeg: copy image", true);
static ProfilingZoneID VDPAUCopyProfilingZone("FFMpeg: VDPAU copy", true);

FrameAvailableCode FFMpegDecoder::renderToBmps(vector<BitmapPtr>& pBmps)
{
    ScopeTimer timer(RenderToBmpProfilingZone);
    AVFrame frame;
    readFrame(frame);

    if (m_pDemuxer->isClosed(m_StreamIndex)) {
        return FA_CLOSED;
    } else {
        if (m_pFrameDecoder->isEOF() && !m_bEOFPending) {
            return FA_USE_LAST_FRAME;
        } else {
            if (pixelFormatIsPlanar(m_PF)) {
#ifdef AVG_ENABLE_VDPAU
                if (m_bUseVDPAU) {
                    ScopeTimer timer(VDPAUCopyProfilingZone);
                    vdpau_render_state* pRenderState = (vdpau_render_state *)frame.data[0];
                    getPlanesFromVDPAU(pRenderState, pBmps[0], pBmps[1], pBmps[2]);
                } else {
                    ScopeTimer timer(CopyImageProfilingZone);
                    for (unsigned i = 0; i < pBmps.size(); ++i) {
                        m_pFrameDecoder->copyPlaneToBmp(pBmps[i],
                                frame.data[i], frame.linesize[i]);
                    }
                }
#else 
                ScopeTimer timer(CopyImageProfilingZone);
                for (unsigned i = 0; i < pBmps.size(); ++i) {
                    m_pFrameDecoder->copyPlaneToBmp(pBmps[i],
                            frame.data[i], frame.linesize[i]);
                }
#endif
            } else {
#ifdef AVG_ENABLE_VDPAU
                if (m_bUseVDPAU) {
                    ScopeTimer timer(VDPAUCopyProfilingZone);
                    vdpau_render_state* pRenderState = (vdpau_render_state *)frame.data[0];
                    getBitmapFromVDPAU(pRenderState, pBmps[0]);
                } else {
                    m_pFrameDecoder->convertFrameToBmp(frame, pBmps[0]);
                }
#else 
                m_pFrameDecoder->convertFrameToBmp(frame, pBmps[0]);
#endif
            }
            return FA_NEW_FRAME;
        }
    }
}


#ifdef AVG_ENABLE_VDPAU
FrameAvailableCode FFMpegDecoder::renderToVDPAU(vdpau_render_state** ppRenderState)
{
    AVG_ASSERT(m_bUseVDPAU);
    ScopeTimer timer(RenderToBmpProfilingZone);
    AVFrame frame;
    readFrame(frame);
    if (m_pDemuxer->isClosed(m_StreamIndex)) {
        return FA_CLOSED;
    } else {
        if (m_pFrameDecoder->isEOF() && !m_bEOFPending) {
            return FA_USE_LAST_FRAME;
        } else {
            ScopeTimer timer(VDPAUCopyProfilingZone);
            vdpau_render_state *pRenderState = (vdpau_render_state *)frame.data[0];
            *ppRenderState = pRenderState;
            return FA_NEW_FRAME;
        }
    }
}
#endif

bool FFMpegDecoder::isVideoSeekDone()
{
    bool bSeekDone = m_bVideoSeekDone;
    m_bVideoSeekDone = false;
    return bSeekDone;
}

int FFMpegDecoder::getSeekSeqNum()
{
    return m_SeekSeqNum;
}

bool FFMpegDecoder::isEOF() const
{
    return m_pFrameDecoder->isEOF() && !m_bEOFPending;
}

static ProfilingZoneID DecodeProfilingZone("FFMpeg: decode", true);

void FFMpegDecoder::readFrame(AVFrame& frame)
{
    ScopeTimer timer(DecodeProfilingZone); 

    if (m_bEOFPending) {
        m_bEOFPending = false;
        return;
    }
    bool bDone = false;
    while (!bDone) {
        int seqNum;
        float seekTime = m_pDemuxer->isSeekDone(m_StreamIndex, seqNum);
        if (seekTime != -1) {
            m_pFrameDecoder->handleSeek();
            m_SeekSeqNum = seqNum;
            m_bVideoSeekDone = true;
        }
        AVPacket* pPacket = m_pDemuxer->getPacket(m_StreamIndex);
        bool bGotPicture = m_pFrameDecoder->decodePacket(pPacket, frame, m_bVideoSeekDone);
        if (bGotPicture && m_pFrameDecoder->isEOF()) {
            m_bEOFPending = true;
        }
        if (bGotPicture || m_pFrameDecoder->isEOF()) {
            bDone = true;
        }
    }
}

}

