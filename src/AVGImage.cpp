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
    : m_pBmp(0)
{
    NS_INIT_ISUPPORTS();
}

AVGImage::~AVGImage ()
{
    if (m_pBmp) {
        delete m_pBmp;
    }
}

NS_IMETHODIMP 
AVGImage::GetType(PRInt32 *_retval)
{
    *_retval = NT_IMAGE;
    return NS_OK;
}

void AVGImage::init (const std::string& id, const std::string& filename, int bpp, 
       IAVGDisplayEngine * pEngine, AVGContainer * pParent, AVGPlayer * pPlayer)
{
    AVGNode::init(id, pEngine, pParent, pPlayer);

    m_Filename = filename;
    AVG_TRACE(AVGPlayer::DEBUG_PROFILE, "Loading " << m_Filename);
    m_pBmp = getEngine()->createSurface();

    PLAnyPicDecoder decoder;
    PLAnyBmp TempBmp;
    PLBmp * pBmp;
    if (bpp == 32 || !pEngine->supportsBpp(bpp)) {
        pBmp = m_pBmp;
    } else {
        pBmp = &TempBmp;
    }
    decoder.MakeBmpFromFile(m_Filename.c_str(), pBmp, 32);
    if (!pEngine->hasRGBOrdering()) {
        pBmp->ApplyFilter(PLFilterFlipRGB());
    }
    if (bpp != 32 && pEngine->supportsBpp(bpp)) {
        m_pBmp->CreateCopy(*pBmp, bpp);
        
    }
    getEngine()->surfaceChanged(m_pBmp);
}

void AVGImage::render (const AVGDRect& Rect)
{
    getEngine()->blt32(m_pBmp, &getAbsViewport(), getEffectiveOpacity(), 
            getAngle(), getPivot());
}

bool AVGImage::obscures (const AVGDRect& Rect, int z) 
{
    return (getEffectiveOpacity() > 0.999 && !m_pBmp->HasAlpha() &&
            getZ() > z && getVisibleRect().Contains(Rect));
}

string AVGImage::getTypeStr ()
{
    return "AVGImage";
}

AVGDPoint AVGImage::getPreferredMediaSize()
{
    return AVGDPoint(m_pBmp->GetSize());
}

