//
// $Id$
// 

#include "AVGImage.h"
#include "IAVGDisplayEngine.h"
#include "AVGPlayer.h"
#include "AVGLogger.h"

#include <paintlib/plbitmap.h>
#include <paintlib/planybmp.h>
#include <paintlib/plpngenc.h>
#include <paintlib/planydec.h>
#include <paintlib/Filter/plfilterresizebilinear.h>
#include <paintlib/Filter/plfilterfliprgb.h>
#include <paintlib/Filter/plfilterfill.h>

#include <nsMemory.h>
#include <xpcom/nsComponentManagerUtils.h>

#include <iostream>
#include <sstream>

using namespace std;

NS_IMPL_ISUPPORTS1_CI(AVGImage, IAVGNode);


AVGImage * AVGImage::create()
{
    return createNode<AVGImage>("@c-base.org/avgimage;1");
}       

AVGImage::AVGImage ()
    : m_pBmp(0),
      m_pOrigBmp(0)
{
    NS_INIT_ISUPPORTS();
}

AVGImage::~AVGImage ()
{
    if (m_pBmp) {
        delete m_pBmp;
    }
    if (m_pOrigBmp) {
        delete m_pOrigBmp;
    }
}

NS_IMETHODIMP 
AVGImage::GetType(PRInt32 *_retval)
{
    *_retval = NT_IMAGE;
    return NS_OK;
}

void AVGImage::init (const std::string& id, int x, int y, int z, 
       int width, int height, double opacity, const std::string& filename, 
       IAVGDisplayEngine * pEngine, AVGContainer * pParent, AVGPlayer * pPlayer)
{
    AVGNode::init(id, pEngine, pParent, pPlayer);

    m_Filename = filename;
    AVG_TRACE(AVGPlayer::DEBUG_MEMORY, "Loading " << m_Filename << endl);
    m_pBmp = getEngine()->createSurface();

    PLAnyPicDecoder decoder;
    decoder.MakeBmpFromFile(m_Filename.c_str(), m_pBmp);
    m_pBmp->ApplyFilter(PLFilterFlipRGB());
   
    initVisible(x, y, z,  width, height, opacity);

    PLPoint size = PLPoint(getRelViewport().Width(), getRelViewport().Height());
    if (m_pBmp->GetWidth() != size.x || m_pBmp->GetHeight() != size.y) {
        m_pOrigBmp = m_pBmp;
        m_pBmp = getEngine()->createSurface();
        m_pBmp->CreateFilteredCopy(*m_pOrigBmp, PLFilterResizeBilinear(size.x, size.y));
    }
    getEngine()->surfaceChanged(m_pBmp);
}

void AVGImage::render (const PLRect& Rect)
{
//    cerr << "render " << getID() << endl;
    
/*
    PLRect SrcRect(0, 0, getRelViewport().Width(), getRelViewport().Height());
    if (getRelViewport().tl.x < 0) {
        SrcRect.tl.x = -getRelViewport().tl.x;
    }
    if (getRelViewport().tl.y < 0) {
        SrcRect.tl.y = -getRelViewport().tl.y;
    }
*/
    getEngine()->blt32(m_pBmp, &getAbsViewport(), getEffectiveOpacity());
}

bool AVGImage::obscures (const PLRect& Rect, int z) 
{
    return (getEffectiveOpacity() > 0.999 && !m_pBmp->HasAlpha() &&
            getZ() > z && getVisibleRect().Contains(Rect));
}

void AVGImage::setViewport (int x, int y, int width, int height)
{
    if (width != -32767 || height != -32767) {
        if (!m_pOrigBmp) {
            m_pOrigBmp = m_pBmp;
            m_pBmp = getEngine()->createSurface();
        }
        if (width == -32767) {
            width = getRelViewport().Width();
        }
        if (height == -32767) {
            height = getRelViewport().Height();
        }
        m_pBmp->CreateFilteredCopy(*m_pOrigBmp, PLFilterResizeBilinear(width, height));
    }
    
    AVGNode::setViewport(x, y, width, height);
}

string AVGImage::getTypeStr ()
{
    return "AVGImage";
}

PLPoint AVGImage::getPreferredMediaSize()
{
    return m_pBmp->GetSize();
}

