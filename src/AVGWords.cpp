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
    return createNode<AVGWords>("@c-base.org/avgwords;1");
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
AVGWords::GetStringAttr(const char *name, char **_retval)
{
    if (!strcmp(name, "Font")) {
        strcpy(*_retval, m_FontName.c_str());
    } else if (!strcmp(name, "String")) {
        strcpy(*_retval, m_Str.c_str());
    } else if (!strcmp(name, "Color")) {
        strcpy (*_retval, m_ColorName.c_str());
    } else {
        return AVGNode::GetStringAttr(name, _retval);
    }
    return NS_OK;
}

NS_IMETHODIMP 
AVGWords::GetIntAttr(const char *name, PRInt32 *_retval)
{
    if (!strcmp(name, "Size")) {
        *_retval = m_Size;
    } else {
        return AVGVisibleNode::GetIntAttr(name, _retval);
    }
    return NS_OK;
}

NS_IMETHODIMP
AVGWords::SetStringAttr(const char *name, const char *value)
{
    if (!strcmp(name, "Font")) {
        m_FontName = value;
        loadFont();
    } else if (!strcmp(name, "String")) {
        if (m_Str != value) {
            m_Str = value;
            drawString();
        }
    } else if (!strcmp(name, "Color")) {
        m_ColorName = value;        
        m_Color = colorStringToColor(m_ColorName);
    } else {
        return AVGNode::SetStringAttr(name, value);
    }
    invalidate();
    return NS_OK;
}

NS_IMETHODIMP
AVGWords::SetIntAttr(const char *name, PRInt32 value)
{
    if (!strcmp(name, "Size")) {
        m_Size = value;
        loadFont();
    } else {
        return AVGVisibleNode::SetIntAttr(name, value);
    }
    invalidate();
    return NS_OK;
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
    m_ColorName = color;
    m_Color = colorStringToColor(color);
    m_FontName = font;
    loadFont();

}

string AVGWords::getTypeStr ()
{
    return "AVGWords";
}

void AVGWords::setViewport (int x, int y, int width, int height)
{
    PLPoint oldExtents(getRelViewport().Width(), getRelViewport().Height());
    AVGVisibleNode::setViewport (x, y, width, height);
    if (oldExtents != PLPoint(width, height) && (width != -1 || height != -1)) {
        drawString();
    }
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

    drawString();
}

void AVGWords::drawString()
{
    IDirectFB * pDFB = getEngine()->getDFB();

    DFBSurfaceDescription SurfDesc;
    SurfDesc.flags = DFBSurfaceDescriptionFlags
        (DSDESC_CAPS | DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_PIXELFORMAT);
    SurfDesc.caps = DSCAPS_NONE;
    SurfDesc.width = getRelViewport().Width();
    SurfDesc.height = getRelViewport().Height();
    SurfDesc.pixelformat = DSPF_A8;  
    DFBResult err = pDFB->CreateSurface(pDFB, &SurfDesc, &m_pSurface);
    getEngine()->DFBErrorCheck(AVG_ERR_FONT_INIT_FAILED, err);
    m_pSurface->Clear(m_pSurface, 0x00,0x00,0x00,0x00);
    m_pSurface->SetColor(m_pSurface, 0xFF, 0xFF, 0xFF, 0xFF);

    m_pSurface->SetDrawingFlags(m_pSurface, DSDRAW_BLEND);
    m_pSurface->SetFont(m_pSurface, m_pFont);
    m_pSurface->DrawString(m_pSurface, m_Str.c_str(), -1, 0, 0, 
        DFBSurfaceTextFlags(DSTF_LEFT | DSTF_TOP));    
}

void AVGWords::render(const PLRect& Rect)
{
    getEngine()->setClipRect(getAbsViewport());
    IDirectFBSurface * pSurface = getEngine()->getPrimary();
    pSurface->SetColor(pSurface, m_Color.GetR(), m_Color.GetG(), m_Color.GetB(),
            __u8(getEffectiveOpacity()*256));
//    getEngine()->render(m_pSurface, getAbsViewport().tl, getEffectiveOpacity(), true);
   
    DFBSurfaceBlittingFlags BltFlags;
    BltFlags = DFBSurfaceBlittingFlags(DSBLIT_BLEND_ALPHACHANNEL | DSBLIT_COLORIZE);
    pSurface->SetBlittingFlags(pSurface, BltFlags);
    
    PLPoint pos = getAbsViewport().tl;
    DFBResult err = pSurface->Blit(pSurface, m_pSurface, 0, 
            pos.x, pos.y);
    getEngine()->DFBErrorCheck(AVG_ERR_VIDEO_GENERAL, err);

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

