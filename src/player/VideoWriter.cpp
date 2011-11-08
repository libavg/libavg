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

#include "VideoWriter.h"
#include "OffscreenCanvas.h"
#include "Player.h"
#include "SDLDisplayEngine.h"

#include "../graphics/FBO.h"
#include "../graphics/GPURGB2YUVFilter.h"
#include "../graphics/Filterfill.h"
#include "../base/StringHelper.h"

#include <boost/bind.hpp>

#include <fcntl.h>
#include <stdio.h>

using namespace std;
using namespace boost;

namespace avg {

VideoWriter::VideoWriter(CanvasPtr pCanvas, const string& sOutFileName, int frameRate,
        int qMin, int qMax, bool bSyncToPlayback)
    : m_pCanvas(pCanvas),
      m_sOutFileName(sOutFileName),
      m_FrameRate(frameRate),
      m_QMin(qMin),
      m_QMax(qMax),
      m_bHasValidData(false),
      m_bSyncToPlayback(bSyncToPlayback),
      m_bPaused(false),
      m_PauseTime(0),
      m_bStopped(false),
      m_CurFrame(0),
      m_StartTime(-1),
      m_bFramePending(false)
{
    m_FrameSize = m_pCanvas->getSize();
#ifdef WIN32
    int fd = _open(m_sOutFileName.c_str(), O_RDWR | O_CREAT, _S_IREAD | _S_IWRITE);

#else
    int fd = open(m_sOutFileName.c_str(), O_RDWR | O_CREAT, S_IRWXU);
#endif
    if (fd == -1) {
        throw Exception(AVG_ERR_VIDEO_INIT_FAILED, 
                string("Could not open output file '") + m_sOutFileName + "'. Reason: " +
                strerror(errno));
    }
#ifdef WIN32
    _close(fd);
#else
    close(fd);
#endif
    remove(m_sOutFileName.c_str());
    CanvasPtr pMainCanvas = Player::get()->getMainCanvas();
    if (pMainCanvas != m_pCanvas) {
        m_pFBO = dynamic_pointer_cast<OffscreenCanvas>(m_pCanvas)->getFBO();
        if (GLContext::getCurrent()->isUsingShaders()) {
            m_pFilter = GPURGB2YUVFilterPtr(new GPURGB2YUVFilter(m_FrameSize));
        }
    }
    VideoWriterThread writer(m_CmdQueue, m_sOutFileName, m_FrameSize, m_FrameRate, 
            qMin, qMax);
    m_pThread = new boost::thread(writer);
    m_pCanvas->registerPlaybackEndListener(this);
    m_pCanvas->registerFrameEndListener(this);
}

VideoWriter::~VideoWriter()
{
    stop();
    m_pThread->join();
    delete m_pThread;
}

void VideoWriter::stop()
{
    if (!m_bStopped) {
        getFrameFromPBO();
        if (!m_bHasValidData) {
            writeDummyFrame();
        }

        m_bStopped = true;
        m_CmdQueue.pushCmd(boost::bind(&VideoWriterThread::stop, _1));
        
        m_pCanvas->unregisterFrameEndListener(this);
        m_pCanvas->unregisterPlaybackEndListener(this);
    }
}

void VideoWriter::pause()
{
    if (m_bPaused) {
        throw Exception(AVG_ERR_UNSUPPORTED, "VideoWriter::pause() called when paused.");
    }
    if (m_bStopped) {
        throw Exception(AVG_ERR_UNSUPPORTED, "VideoWriter::pause() called when stopped.");
    }
    m_bPaused = true;
    m_PauseStartTime = Player::get()->getFrameTime();
}

void VideoWriter::play()
{
    if (!m_bPaused) {
        throw Exception(AVG_ERR_UNSUPPORTED, 
                "VideoWriter::play() called when not paused.");
    }
    m_bPaused = false;
    m_PauseTime += (Player::get()->getFrameTime() - m_PauseStartTime);
}

std::string VideoWriter::getFileName() const
{
    return m_sOutFileName;
}

int VideoWriter::getFramerate() const
{
    return m_FrameRate;
}

int VideoWriter::getQMin() const
{
    return m_QMin;
}

int VideoWriter::getQMax() const
{
    return m_QMax;
}

void VideoWriter::onFrameEnd()
{
    // The VideoWriter handles OffscreenCanvas and MainCanvas differently:
    // For MainCanvas, it simply does a screenshot onFrameEnd and sends that to the 
    // VideoWriterThread immediately.
    // For OffscreenCanvas, an asynchronous PBO readback is started in onFrameEnd.
    // In the next frame's onFrameEnd, the data is read into a bitmap and sent to 
    // the VideoWriterThread.
    if (m_pFBO) {
        // Read last frame's bitmap.
        getFrameFromPBO();
    }
    if (m_StartTime == -1) {
        m_StartTime = Player::get()->getFrameTime();
    }
    if (!m_bPaused) {
        if (m_bSyncToPlayback) {
            getFrameFromFBO();
        } else {
            long long movieTime = Player::get()->getFrameTime() - m_StartTime
                    - m_PauseTime;
            double timePerFrame = 1000./m_FrameRate;
            int wantedFrame = int(movieTime/timePerFrame+0.1);
            if (wantedFrame > m_CurFrame) {
                getFrameFromFBO();
                if (wantedFrame > m_CurFrame + 1) {
                    m_CurFrame = wantedFrame - 1;
                }
            }
        }
    }
    
    if (!m_pFBO) {
        getFrameFromPBO();
    }
}

void VideoWriter::getFrameFromFBO()
{
    if (m_pFBO) {
        if (m_pFilter) {
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            m_pFilter->apply(m_pFBO->getTex());
            FBOPtr pYUVFBO = m_pFilter->getFBO();
            pYUVFBO->moveToPBO();
            glPopMatrix();
        } else {
            m_pFBO->moveToPBO();
        }
        m_bFramePending = true;
    } else {
        BitmapPtr pBmp = Player::get()->getDisplayEngine()->screenshot(GL_BACK);
        sendFrameToEncoder(pBmp);
    }
}

void VideoWriter::getFrameFromPBO()
{
    if (m_bFramePending) {
        BitmapPtr pBmp;
        if (m_pFilter) {
            pBmp = m_pFilter->getFBO()->getImageFromPBO();
        } else {
            pBmp = m_pFBO->getImageFromPBO();
        }
        sendFrameToEncoder(pBmp);
        m_bFramePending = false;
    }
}

void VideoWriter::sendFrameToEncoder(BitmapPtr pBitmap)
{
    m_CurFrame++;
    m_bHasValidData = true;
    if (m_pFilter) {
        m_CmdQueue.pushCmd(boost::bind(&VideoWriterThread::encodeYUVFrame, _1, pBitmap));
    } else {
        m_CmdQueue.pushCmd(boost::bind(&VideoWriterThread::encodeFrame, _1, pBitmap));
    }
}

void VideoWriter::onPlaybackEnd()
{
    stop();
}

void VideoWriter::writeDummyFrame()
{
    BitmapPtr pBmp = BitmapPtr(new Bitmap(m_FrameSize, B8G8R8X8));
    FilterFill<Pixel32>(Pixel32(0,0,0,255)).applyInPlace(pBmp);
    sendFrameToEncoder(pBmp);
}

}
