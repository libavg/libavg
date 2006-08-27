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
#include "FFMpegDecoder.h"
#include "ISurface.h"

#include "../base/Exception.h"
#include "../base/Logger.h"
#include "../base/ScopeTimer.h"
#include "../base/XMLHelper.h"
#include "../graphics/Filterflipuv.h"

#include <iostream>
#include <sstream>
#include <unistd.h>

using namespace std;

namespace avg {

bool Video::m_bInitialized = false;

Video::Video ()
    : m_href(""),
      m_Filename(""),
      m_bLoop(false),
      m_bEOF(false),
      m_pDecoder(0)
{
}

Video::Video (const xmlNodePtr xmlNode, Player * pPlayer)
    : VideoBase(xmlNode, pPlayer),
      m_Filename(""),
      m_bEOF(false),
      m_pDecoder(0)
{
    m_href = getDefaultedStringAttr (xmlNode, "href", "");
    m_bLoop = getDefaultedBoolAttr (xmlNode, "loop", false);
}

Video::~Video ()
{
    if (m_pDecoder) {
        delete m_pDecoder;
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

void Video::seekToFrame(int num)
{
    if (getVideoState() != Unloaded) {
        seek(num);
    } else {
        AVG_TRACE(Logger::WARNING, 
                "Error in Video::SeekToFrame: Video "+getID()+" not loaded.");
    }
}

bool Video::getLoop() const
{
    return m_bLoop;
}

void Video::connect(DisplayEngine * pEngine)
{
    m_pDecoder = new FFMpegDecoder();
    m_Filename = m_href;
    initFilename(getPlayer(), m_Filename);
    VideoBase::connect(pEngine);
}

void Video::disconnect()
{
    stop();
    VideoBase::disconnect();
    delete m_pDecoder;
    m_pDecoder = 0;
}

const string& Video::getHRef() const
{
    return m_Filename;
}

void Video::setHRef(const string& href)
{
    string fileName (href);
    initFilename(getPlayer(), fileName);
    if (fileName != m_Filename) {
        changeVideoState(Unloaded);
        m_Filename = fileName;
        changeVideoState(Paused);
    }
}

string Video::getTypeStr ()
{
    return "Video";
}

void Video::seek(int DestFrame) {
    m_pDecoder->seek(DestFrame, m_CurFrame);
    m_CurFrame = DestFrame;
    setFrameAvailable(false);
}

void Video::open(int* pWidth, int* pHeight)
{
    m_CurFrame = 0;
    m_pDecoder->open(m_Filename, pWidth, pHeight);
    m_bEOF = false;
}

void Video::close()
{
    m_pDecoder->close();
}

PixelFormat Video::getDesiredPixelFormat() 
{
    return m_pDecoder->getDesiredPixelFormat();
}

double Video::getFPS()
{
    return m_pDecoder->getFPS();
}

static ProfilingZone RenderProfilingZone("    Video::render");

bool Video::renderToSurface(ISurface * pSurface)
{
    ScopeTimer Timer(RenderProfilingZone);
    if (getYCbCrMode() == DisplayEngine::OGL_SHADER) {
        m_bEOF = m_pDecoder->renderToYCbCr420p(pSurface->lockBmp(0),
                pSurface->lockBmp(1), pSurface->lockBmp(2));
    } else {
        BitmapPtr pBmp = pSurface->lockBmp();
        m_bEOF = m_pDecoder->renderToBmp(pBmp);
        if (getYCbCrMode() == DisplayEngine::OGL_MESA) {
            FilterFlipUV().applyInPlace(pBmp);
        }   
    }
    pSurface->unlockBmps();
    if (!m_bEOF) {
        getEngine()->surfaceChanged(pSurface);
    }
    advancePlayback();
    return !m_bEOF;
}

bool Video::canRenderToBackbuffer(int BPP) 
{
    return m_pDecoder->canRenderToBuffer(BPP);
}

void Video::advancePlayback()
{
    m_CurFrame++;
    if (m_bEOF) {
        if (m_bLoop) {
            seek(0);
        } else {
            changeVideoState(Paused);
        }
    }
}

}
