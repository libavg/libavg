//
// $Id$
// 

#include "AVGImage.h"
#include "IAVGDisplayEngine.h"
#include "AVGPlayer.h"
#include "AVGLogger.h"
#include "IAVGSurface.h"

#include <paintlib/plbitmap.h>
#include <paintlib/planybmp.h>
#include <paintlib/plpngenc.h>
#include <paintlib/planydec.h>
#include <paintlib/Filter/plfilterresizebilinear.h>
#include <paintlib/Filter/plfilterfliprgb.h>
#include <paintlib/Filter/plfilterfill.h>
#include <paintlib/Filter/plfiltercolorize.h>

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
    : m_pSurface(0)
{
    NS_INIT_ISUPPORTS();
}

AVGImage::~AVGImage ()
{
    if (m_pSurface) {
        delete m_pSurface;
    }
}

NS_IMETHODIMP 
AVGImage::GetType(PRInt32 *_retval)
{
    *_retval = NT_IMAGE;
    return NS_OK;
}

void AVGImage::init (const std::string& id, const std::string& filename, 
        int bpp, int hue, int saturation, IAVGDisplayEngine * pEngine, 
        AVGContainer * pParent, AVGPlayer * pPlayer)
{
    AVGNode::init(id, pEngine, pParent, pPlayer);

    m_Filename = filename;
    AVG_TRACE(AVGPlayer::DEBUG_PROFILE, "Loading " << m_Filename);
    m_pSurface = getEngine()->createSurface();

    PLAnyPicDecoder decoder;
    PLAnyBmp TempBmp;
    decoder.MakeBmpFromFile(m_Filename.c_str(), &TempBmp, 32);
    m_Hue = hue;
    m_Saturation = saturation;
    if (m_Saturation != -1) {
        cerr << "hue: " << m_Hue << ", saturation: " << m_Saturation << endl;
        TempBmp.ApplyFilter(PLFilterColorize(m_Hue, m_Saturation));
    }
    
    if (!pEngine->hasRGBOrdering()) {
        TempBmp.ApplyFilter(PLFilterFlipRGB());
    }
    int DestBPP = bpp;
    if (!pEngine->supportsBpp(bpp)) {
        DestBPP = 32;
    }

    m_pSurface->create(TempBmp.GetWidth(), TempBmp.GetHeight(), 
            DestBPP, TempBmp.HasAlpha());
    m_pSurface->getBmp()->CopyPixels(TempBmp);
}

void AVGImage::render (const AVGDRect& Rect)
{
    getEngine()->blt32(m_pSurface, &getAbsViewport(), getEffectiveOpacity(), 
            getAngle(), getPivot());
}

bool AVGImage::obscures (const AVGDRect& Rect, int z) 
{
    return (getEffectiveOpacity() > 0.999 && !m_pSurface->getBmp()->HasAlpha() 
            && getZ() > z && getVisibleRect().Contains(Rect));
}

string AVGImage::getTypeStr ()
{
    return "AVGImage";
}

AVGDPoint AVGImage::getPreferredMediaSize()
{
    return AVGDPoint(m_pSurface->getBmp()->GetSize());
}

