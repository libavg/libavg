//
// $Id$
// 

#include "AVGVideo.h"
#include "AVGDFBDisplayEngine.h"
#include "AVGException.h"
#include "AVGPlayer.h"
#include "AVGLogger.h"
#include "AVGContainer.h"
#include "AVGFFMpegDecoder.h"
#include "IAVGSurface.h"

#include <paintlib/plbitmap.h>
#include <paintlib/plpngenc.h>
#include <paintlib/planybmp.h>
#include <paintlib/Filter/plfilterfill.h>

#include <nsMemory.h>
#include <xpcom/nsComponentManagerUtils.h>

#include <iostream>
#include <sstream>
#include <unistd.h>

using namespace std;

NS_IMPL_ISUPPORTS3_CI(AVGVideo, IAVGNode, IAVGVideoBase, IAVGVideo);

bool AVGVideo::m_bInitialized = false;

AVGVideo * AVGVideo::create()
{
    return createNode<AVGVideo>("@c-base.org/avgvideo;1");
}       

AVGVideo::AVGVideo ()
    : m_pDecoder(0)
{
    NS_INIT_ISUPPORTS();
}

AVGVideo::~AVGVideo ()
{
    if (m_pDecoder) {
        delete m_pDecoder;
    }
}

NS_IMETHODIMP 
AVGVideo::GetType(PRInt32 *_retval)
{
    *_retval = NT_VIDEO;
    return NS_OK;
}

NS_IMETHODIMP AVGVideo::GetNumFrames(int *_retval)
{
    if (getState() != Unloaded) {
        *_retval = m_pDecoder->getNumFrames();
    } else {
        AVG_TRACE(IAVGPlayer::DEBUG_WARNING,
               "Error in AVGVideo::GetNumFrames: Video not loaded.");
        *_retval = -1;
    }
    return NS_OK;
}

NS_IMETHODIMP AVGVideo::GetCurFrame(int *_retval)
{
    if (getState() != Unloaded) {
        *_retval = m_CurFrame;
    } else {
        AVG_TRACE(IAVGPlayer::DEBUG_WARNING, 
                "Error in AVGVideo::GetCurFrame: Video not loaded.");
        *_retval = -1;
    }
    return NS_OK;
}

NS_IMETHODIMP AVGVideo::SeekToFrame(int num)
{
    if (getState() != Unloaded) {
        seek(num);
    } else {
        AVG_TRACE(IAVGPlayer::DEBUG_WARNING, 
                "Error in AVGVideo::SeekToFrame: Video not loaded.");
    }
    return NS_OK;
}

void AVGVideo::init (const std::string& id, const std::string& filename,
       bool bLoop, bool bOverlay, 
       IAVGDisplayEngine * pEngine, AVGContainer * pParent, AVGPlayer * pPlayer)
{
    m_Filename = filename;
    m_pDecoder = new AVGFFMpegDecoder();
    m_bLoop = bLoop;
    
    AVGVideoBase::init(id, bOverlay, pEngine, pParent, pPlayer);
}

string AVGVideo::getTypeStr ()
{
    return "AVGVideo";
}

void AVGVideo::seek(int DestFrame) {
    m_pDecoder->seek(DestFrame, m_CurFrame);
    m_CurFrame = DestFrame;
    setFrameAvailable(false);
}

void AVGVideo::open(int* pWidth, int* pHeight)
{
    m_CurFrame = 0;
    m_pDecoder->open(m_Filename, pWidth, pHeight);
}

void AVGVideo::close()
{
    m_pDecoder->close();
}

double AVGVideo::getFPS()
{
    return m_pDecoder->getFPS();
}

bool AVGVideo::renderToSurface(IAVGSurface * pSurface)
{
    m_bEOF = m_pDecoder->renderToBmp(pSurface->getBmp(), 
            getEngine()->hasRGBOrdering());
    if (!m_bEOF) {
        getEngine()->surfaceChanged(pSurface);
    }
    advancePlayback();
    return !m_bEOF;
}

bool AVGVideo::canRenderToBackbuffer(int BPP) 
{
    return m_pDecoder->canRenderToBuffer(BPP);
}

void AVGVideo::advancePlayback()
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
