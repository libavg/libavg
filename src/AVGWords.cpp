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
    : m_pSurface(0)
{
    NS_INIT_ISUPPORTS();
}

AVGWords::~AVGWords ()
{
    if (m_pSurface) {
        m_pSurface->Release(m_pSurface);
    }
}

NS_IMETHODIMP
AVGWords::GetStringAttr(const char *name, char **_retval)
{
    string Attr;
    if (!strcmp(name, "Font")) {
        Attr = m_FontName;
    } else if (!strcmp(name, "String")) {
        Attr = m_Str;
    } else if (!strcmp(name, "Color")) {
        Attr = m_ColorName;
    } else {
        return AVGNode::GetStringAttr(name, _retval);
    }
    *_retval = (char *) nsMemory::Clone(Attr.c_str(), 
            Attr.length()+1);
    return NS_OK;
}

NS_IMETHODIMP 
AVGWords::GetIntAttr(const char *name, PRInt32 *_retval)
{
    if (!strcmp(name, "Size")) {
        *_retval = m_Size;
    } else {
        return AVGNode::GetIntAttr(name, _retval);
    }
    return NS_OK;
}

NS_IMETHODIMP
AVGWords::SetStringAttr(const char *name, const char *value)
{
    if (!strcmp(name, "Font")) {
        m_FontName = value;
        changeFont();
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
        changeFont();
    } else {
        return AVGNode::SetIntAttr(name, value);
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
    AVGNode::init(id, pEngine, pParent);
    initVisible(x, y, z, width, height, opacity); 
    m_Size = size;
    m_Str = str;
    m_ColorName = color;
    m_Color = colorStringToColor(color);
    m_FontName = font;
    changeFont();
}

string AVGWords::getTypeStr ()
{
    return "AVGWords";
}

void AVGWords::setViewport (int x, int y, int width, int height)
{
    PLPoint oldExtents(getRelViewport().Width(), getRelViewport().Height());
    AVGNode::setViewport (x, y, width, height);
    if (oldExtents != PLPoint(width, height) && (width != -1 || height != -1)) {
        drawString();
    }
}

void AVGWords::changeFont()
{
    IDirectFB * pDFB = getEngine()->getDFB();
    m_pFont = getEngine()->getFontManager()->getFont(pDFB, m_FontName, m_Size);

    drawString();
}

void AVGWords::drawString()
{
    if (m_pSurface) {
        m_pSurface->Release(m_pSurface);
    }

    IDirectFB * pDFB = getEngine()->getDFB();

    DFBSurfaceDescription SurfDesc;
    SurfDesc.flags = DFBSurfaceDescriptionFlags
        (DSDESC_CAPS | DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_PIXELFORMAT);
    SurfDesc.caps = DSCAPS_NONE;
    SurfDesc.width = getRelViewport().Width();
    SurfDesc.height = getRelViewport().Height();
    SurfDesc.pixelformat = DSPF_A8;  
    DFBResult err = pDFB->CreateSurface(pDFB, &SurfDesc, &m_pSurface);
    getEngine()->DFBErrorCheck(AVG_ERR_FONT_INIT_FAILED, "AVGWords::drawString", err);
    m_pSurface->Clear(m_pSurface, 0x00,0x00,0x00,0x00);
    m_pSurface->SetColor(m_pSurface, 0xFF, 0xFF, 0xFF, 0xFF);

    m_pSurface->SetDrawingFlags(m_pSurface, DSDRAW_BLEND);
    m_pSurface->SetFont(m_pSurface, m_pFont);
    m_pSurface->DrawString(m_pSurface, m_Str.c_str(), -1, 0, 0, 
        DFBSurfaceTextFlags(DSTF_LEFT | DSTF_TOP));    
}

void AVGWords::render(const PLRect& Rect)
{
//    cerr << "render " << getID() << endl;
    if (getEffectiveOpacity() > 0.001) {
        bool bVisible = getEngine()->setClipRect(getAbsViewport());
        if (bVisible) {
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
            getEngine()->DFBErrorCheck(AVG_ERR_VIDEO_GENERAL, "AVGWords::render", err);
        }
    }
}

PLPixel32 AVGWords::colorStringToColor(const string & colorString)
{   
    int r,g,b;
    sscanf(colorString.c_str(), "%2x%2x%2x", &r, &g, &b);
    return PLPixel32(r,g,b);
}

