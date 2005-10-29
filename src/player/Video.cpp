//
// $Id$
// 

#include "Video.h"
#include "IDisplayEngine.h"
#include "Player.h"
#include "FFMpegDecoder.h"
#include "ISurface.h"

#include "../base/Exception.h"
#include "../base/Logger.h"
#include "../base/ScopeTimer.h"
#include "../base/XMLHelper.h"

#include <iostream>
#include <sstream>
#include <unistd.h>

using namespace std;

namespace avg {

bool Video::m_bInitialized = false;

Video::Video ()
    : m_Filename(""),
      m_bLoop(false),
      m_pDecoder(0)
{
}

Video::Video (const xmlNodePtr xmlNode, Container * pParent)
    : VideoBase(xmlNode, pParent),
      m_pDecoder(0)
{
    m_Filename = getDefaultedStringAttr (xmlNode, "href", "");
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
    if (getState() != Unloaded) {
        return m_pDecoder->getNumFrames();
    } else {
        AVG_TRACE(Logger::WARNING,
               "Error in Video::getNumFrames: Video not loaded.");
        return -1;
    }
}

int Video::getCurFrame() const
{
    if (getState() != Unloaded) {
        return m_CurFrame;
    } else {
        AVG_TRACE(Logger::WARNING, 
                "Error in Video::GetCurFrame: Video not loaded.");
        return -1;
    }
}

void Video::seekToFrame(int num)
{
    if (getState() != Unloaded) {
        seek(num);
    } else {
        AVG_TRACE(Logger::WARNING, 
                "Error in Video::SeekToFrame: Video not loaded.");
    }
}

bool Video::getLoop() const
{
    return m_bLoop;
}

bool Video::isYCbCrSupported() 
{
    return m_pDecoder->isYCbCrSupported();
}

void Video::init (IDisplayEngine * pEngine, Container * pParent, 
        Player * pPlayer)
{
    m_pDecoder = new FFMpegDecoder();
    initFilename(pPlayer, m_Filename);
    VideoBase::init(pEngine, pParent, pPlayer);
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
}

void Video::close()
{
    m_pDecoder->close();
}

double Video::getFPS()
{
    return m_pDecoder->getFPS();
}

static ProfilingZone RenderProfilingZone("    Video::render");

bool Video::renderToSurface(ISurface * pSurface)
{
    ScopeTimer Timer(RenderProfilingZone);
    m_bEOF = m_pDecoder->renderToBmp(pSurface->getBmp());
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
            changeState(Paused);
        }
    }
}

}
