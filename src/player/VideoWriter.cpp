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
#ifdef WIN32
#define _open open
#define _O_RDWR O_RDWR
#endif

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
      m_bStopped(false)
{
    IntPoint size = m_pCanvas->getSize();
    int fd = open(m_sOutFileName.c_str(), O_RDWR);
    if (fd == -1) {
        throw Exception(AVG_ERR_VIDEO_INIT_FAILED, 
                string("Could not open output file '") + m_sOutFileName + "'. Reason: " +
                strerror(errno));
    }
    close(fd);
    VideoWriterThread writer(m_CmdQueue, m_sOutFileName, size, m_FrameRate, qMin, qMax);
    m_pThread = new boost::thread(writer);
    m_pCanvas->registerPlaybackEndListener(this);
    m_pCanvas->registerFrameEndListener(this);
    // TODO: Why yield here?
//    m_pVideoWriterThread->yield();
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
    m_CmdQueue.pushCmd(boost::bind(&VideoWriterThread::encodeFrame, _1, pBitmap));
}

void VideoWriter::handleAutoSynchronizedFrame()
{
    // TODO: Make this work with Player::getFrameTime().
    int fraction = static_cast<int>(Player::get()->getFramerate() + 0.5)/m_FrameRate;
    m_FrameIndex = m_FrameIndex % fraction;
    if (m_FrameIndex == 0) {
        handleFrame();
    }
    m_FrameIndex++;
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
