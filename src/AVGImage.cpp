//
// $Id$
// 

#include "AVGImage.h"
#include "AVGDFBDisplayEngine.h"

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

void AVGImage::init (const std::string& id, int x, int y, int z, 
       int width, int height, double opacity, const std::string& filename, 
       AVGDFBDisplayEngine * pEngine, AVGContainer * pParent)
{
    AVGNode::init(id, pEngine, pParent);
    initVisible(x, y, z,  width, height, opacity);
    m_Filename = filename;
    cerr << "Loading " << m_Filename << endl;
    m_pBmp = getEngine()->createSurface();
    PLAnyPicDecoder decoder;

    decoder.MakeBmpFromFile(m_Filename.c_str(), m_pBmp);
    m_pBmp->ApplyFilter(PLFilterFlipRGB());
    if (width == 0 || height == 0) {
        setViewport (x, y, x+m_pBmp->GetWidth(), y+m_pBmp->GetHeight());
    } else {
        if (m_pBmp->GetWidth() != width || m_pBmp->GetHeight() != height) {
            cerr << "Warning: size of image node with id " << id << 
                    " does not match bitmap size." << endl;
            cerr << "  Resizing from " << m_pBmp->GetWidth() << ", " << 
                    m_pBmp->GetHeight() << " to " << width << ", " << height << 
                    "." << endl;
            m_pBmp->ApplyFilter (PLFilterResizeBilinear(width, height));
        }
    }
}

void AVGImage::render (const PLRect& Rect)
{
//    cerr << "render " << getID() << endl;
    getEngine()->render(m_pBmp, getAbsViewport().tl, getEffectiveOpacity());
}

bool AVGImage::obscures (const PLRect& Rect, int z) 
{
    return (getEffectiveOpacity() > 0.999 && !m_pBmp->HasAlpha() &&
            getZ() > z && getAbsViewport().Contains(Rect));
}

string AVGImage::getTypeStr ()
{
    return "AVGImage";
}

