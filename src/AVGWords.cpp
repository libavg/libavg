//
// $Id$
// 

#include "AVGWords.h"
#include "AVGDFBDisplayEngine.h"
#include "AVGException.h"

#include <nsMemory.h>
#include <xpcom/nsComponentManagerUtils.h>

#include <iostream>
#include <stdlib.h>

using namespace std;

NS_IMPL_ISUPPORTS1_CI(AVGWords, IAVGNode);


AVGWords * AVGWords::create()
{
    return createNode<AVGWords>("@c-base.org/avgtext;1");
}       

AVGWords::AVGWords ()
{
    NS_INIT_ISUPPORTS();
}

AVGWords::~AVGWords ()
{
    m_pFont->Release(m_pFont);
}

NS_IMETHODIMP 
AVGWords::GetType(PRInt32 *_retval)
{
    *_retval = NT_TEXT;
    return NS_OK;
}

void 
AVGWords::init (const string& id, int x, int y, int z, 
           int width, int height, double opacity, int size, const string& font, 
           const string& str, const string& color,
           AVGDFBDisplayEngine * pEngine, AVGContainer * pParent)
{
    AVGVisibleNode::init(id, x, y, z,  width, height, opacity, pEngine, pParent);
    m_Size = size;
    m_Str = str;
    m_Color = colorStringToColor(color);
    m_FontName = font;
    loadFont();

}

string AVGWords::getTypeStr ()
{
    return "AVGWords";
}

void AVGWords::loadFont()
{
    DFBFontDescription fontDesc;

    fontDesc.flags = DFBFontDescriptionFlags(DFDESC_HEIGHT | DFDESC_ATTRIBUTES);
    fontDesc.height = m_Size;
    fontDesc.attributes = (DFBFontAttributes)0;

    IDirectFB * pDFB = getEngine()->getDFB();

    string FontPath = getFontPath()+"/"+m_FontName+".ttf";
    DFBResult err = pDFB->CreateFont(pDFB, FontPath.c_str(), &fontDesc, &m_pFont);
    getEngine()->DFBErrorCheck(AVG_ERR_FONT_INIT_FAILED, err);

    DFBSurfaceDescription SurfDesc;
    SurfDesc.flags = DFBSurfaceDescriptionFlags
        (DSDESC_CAPS | DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_PIXELFORMAT);
    SurfDesc.caps = DSCAPS_NONE;
    SurfDesc.width = getRelViewport().Width();
    SurfDesc.height = getRelViewport().Height();
    SurfDesc.pixelformat =  DSPF_ARGB; // DSPF_A8;  // Alpha-only surface?
    err = pDFB->CreateSurface(pDFB, &SurfDesc, &m_pSurface);
    getEngine()->DFBErrorCheck(AVG_ERR_FONT_INIT_FAILED, err);
    m_pSurface->Clear(m_pSurface, 0xFF,0xFF,0xFF,0x00);
    m_pSurface->SetColor(m_pSurface, m_Color.GetR(), m_Color.GetG(), m_Color.GetB(), 0xFF);
    m_pSurface->SetFont(m_pSurface, m_pFont);
    m_pSurface->DrawString(m_pSurface, m_Str.c_str(), -1, 0, 0, 
        DFBSurfaceTextFlags(DSTF_LEFT | DSTF_TOP));
}

void AVGWords::render(const PLRect& Rect)
{
    getEngine()->setClipRect(getAbsViewport());
    IDirectFBSurface * pSurface = getEngine()->getPrimary();
    getEngine()->render(m_pSurface, getAbsViewport().tl, getEffectiveOpacity(), true);
}

PLPixel32 AVGWords::colorStringToColor(const string & colorString)
{   
    int r,g,b;
    sscanf(colorString.c_str(), "%2x%2x%2x", &r, &g, &b);
    return PLPixel32(r,g,b);
}

const string & AVGWords::getFontPath()
{
    static string FontPath;
    if (FontPath=="") {
        char * pFontPath = getenv("AVG_FONT_PATH");
        if (pFontPath) {
            FontPath = pFontPath;
        } else {
            cerr << "Warning: no font path specified." << endl;
        }
    }
    return FontPath;
}

