//
// $Id$
// 

#include "Image.h"
#include "IDisplayEngine.h"
#include "Player.h"
#include "ISurface.h"

#include "../base/Logger.h"
#include "../base/ScopeTimer.h"

#include <paintlib/plbitmap.h>
#include <paintlib/planybmp.h>
#include <paintlib/plpngenc.h>
#include <paintlib/planydec.h>
#include <paintlib/Filter/plfilterresizebilinear.h>
#include <paintlib/Filter/plfilterfliprgb.h>
#include <paintlib/Filter/plfilterfill.h>
#include <paintlib/Filter/plfiltercolorize.h>

#include <iostream>
#include <sstream>

using namespace std;

namespace avg {

Image::Image ()
    : m_Hue(-1),
      m_Saturation(-1)
{
}

Image::~Image ()
{
}

void Image::init (IDisplayEngine * pEngine, Container * pParent,
        Player * pPlayer)
{
    Node::init(pEngine, pParent, pPlayer);
    m_Filename = m_href;
    initFilename(pPlayer, m_Filename);
    AVG_TRACE(Logger::PROFILE, "Loading " << m_Filename);

    PLAnyPicDecoder decoder;
    PLAnyBmp TempBmp;
    decoder.MakeBmpFromFile(m_Filename.c_str(), &TempBmp);
    PLPixelFormat pf = PLPixelFormat::R8G8B8;
    if (TempBmp.HasAlpha()) {
        pf = PLPixelFormat::A8R8G8B8;
    }

    getSurface()->create(TempBmp.GetWidth(), TempBmp.GetHeight(), pf);
    getSurface()->getBmp()->CopyPixels(TempBmp);
    if (m_Saturation != -1) {
        PLFilterColorize(m_Hue, m_Saturation).ApplyInPlace(
                getSurface()->getBmp());
    }
    if (pEngine->hasRGBOrdering()) {
        PLFilterFlipRGB().ApplyInPlace(getSurface()->getBmp());
    }
}

static ProfilingZone RenderProfilingZone("  Image::render");

void Image::render (const DRect& Rect)
{
    ScopeTimer Timer(RenderProfilingZone);
    getEngine()->blt32(getSurface(), &getAbsViewport(), getEffectiveOpacity(), 
            getAngle(), getPivot(), getBlendMode());
}

bool Image::obscures (const DRect& Rect, int z) 
{
    return (isActive() && getEffectiveOpacity() > 0.999
            && !getSurface()->getBmp()->HasAlpha() 
            && getZ() > z && getVisibleRect().Contains(Rect));
}

string Image::getTypeStr ()
{
    return "Image";
}

DPoint Image::getPreferredMediaSize()
{
    return DPoint(getSurface()->getBmp()->GetSize());
}

}
