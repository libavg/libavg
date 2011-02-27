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

#include "VideoWriter.h"
#include "Canvas.h"
#include "Player.h"

#include <boost/bind.hpp>

#include <fcntl.h>
#include <stdio.h>

using namespace std;

namespace avg {

VideoWriter::VideoWriter(Canvas* pCanvas, const string& sOutFileName, int frameRate,
        int qMin, int qMax, bool bSyncToPlayback)
    : m_pCanvas(pCanvas),
      m_sOutFileName(sOutFileName),
      m_FrameRate(frameRate),
      m_QMin(qMin),
      m_QMax(qMax),
      m_bSyncToPlayback(bSyncToPlayback),
      m_bStopped(false),
      m_CurFrame(0)
{
    IntPoint size = m_pCanvas->getSize();
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
    VideoWriterThread writer(m_CmdQueue, m_sOutFileName, size, m_FrameRate, qMin, qMax);
    m_pThread = new boost::thread(writer);
    m_pCanvas->registerPlaybackEndListener(this);
    m_pCanvas->registerFrameEndListener(this);

    m_StartTime = Player::get()->getFrameTime();
}

VideoWriter::~VideoWriter()
{
    stop();
    m_pThread->join();
}

void VideoWriter::stop()
{
    if (!m_bStopped) {
        if (!m_bHasValidData) {
            writeDummyFrame();
        }

        m_bStopped = true;
        m_CmdQueue.pushCmd(boost::bind(&VideoWriterThread::stop, _1));
        
        m_pCanvas->unregisterFrameEndListener(this);
        m_pCanvas->unregisterPlaybackEndListener(this);
    }
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
    if (m_bSyncToPlayback) {
        handleFrame();
    } else {
        handleAutoSynchronizedFrame();
    }
}

void VideoWriter::handleFrame()
{
    BitmapPtr pBmp = m_pCanvas->screenshot();
    addFrame(pBmp);
    m_bHasValidData = true;
}

void VideoWriter::addFrame(BitmapPtr pBitmap)
{
    m_CurFrame++;
    m_CmdQueue.pushCmd(boost::bind(&VideoWriterThread::encodeFrame, _1, pBitmap));
}

void VideoWriter::handleAutoSynchronizedFrame()
{
    long long movieTime = Player::get()->getFrameTime() - m_StartTime;
    double timePerFrame = 1000./m_FrameRate;
    int wantedFrame = int(movieTime/timePerFrame+0.1);
    if (wantedFrame > m_CurFrame) {
        handleFrame();
    }
}


void VideoWriter::onPlaybackEnd()
{
    stop();
}

void VideoWriter::writeDummyFrame()
{
    BitmapPtr pBmp = BitmapPtr(new Bitmap(m_FrameSize, B8G8R8X8));
    addFrame(pBmp);
}

}
