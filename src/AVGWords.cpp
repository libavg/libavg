//
// $Id$
// 

#include "AVGWords.h"
#include "AVGDFBDisplayEngine.h"
#include "AVGException.h"

#include <paintlib/pldirectfbbmp.h>
#include <paintlib/Filter/plfilterfill.h>
#include <paintlib/plpixel8.h>

#include <nsMemory.h>
#include <xpcom/nsComponentManagerUtils.h>

#include <iostream>
#include <stdlib.h>

using namespace std;

NS_IMPL_ISUPPORTS2_CI(AVGWords, IAVGNode, IAVGWords);

AVGWords * AVGWords::create()
{
    return createNode<AVGWords>("@c-base.org/avgwords;1");
}       

AVGWords::AVGWords ()
    : m_pBmp(0)
{
    NS_INIT_ISUPPORTS();
}

AVGWords::~AVGWords ()
{
    if (m_pBmp) {
        delete m_pBmp;
    }
}

/* attribute string font; */
NS_IMETHODIMP AVGWords::GetFont(char * *aFont)
{
    *aFont = (char *) nsMemory::Clone(m_FontName.c_str(), 
            m_FontName.length()+1);
    return NS_OK;
}

NS_IMETHODIMP AVGWords::SetFont(const char * aFont)
{
    if (m_FontName != aFont) {
        invalidate();
        m_FontName = aFont;
        changeFont();
        invalidate();
    }
    return NS_OK;
}

/* attribute string text; */
NS_IMETHODIMP AVGWords::GetText(char * *aText)
{
    *aText = (char *) nsMemory::Clone(m_Str.c_str(), 
            m_Str.length()+1);
    return NS_OK;
}

NS_IMETHODIMP AVGWords::SetText(const char * aText)
{
    if (m_Str != aText) {
        invalidate();
        m_Str = aText;
        drawString();
        invalidate();
    }
    return NS_OK;
}

/* attribute string color; */
NS_IMETHODIMP AVGWords::GetColor(char * *aColor)
{
    *aColor = (char *) nsMemory::Clone(m_ColorName.c_str(), 
            m_ColorName.length()+1);
    return NS_OK;
}

NS_IMETHODIMP AVGWords::SetColor(const char * aColor)
{
    if (m_ColorName != aColor) {
        invalidate();
        m_ColorName = aColor;
        m_Color = colorStringToColor(m_ColorName);
        invalidate();
    }
    return NS_OK;
}

/* attribute long size; */
NS_IMETHODIMP AVGWords::GetSize(PRInt32 *aSize)
{
    *aSize = m_Size;
    return NS_OK;
}

NS_IMETHODIMP AVGWords::SetSize(PRInt32 aSize)
{
    if (m_Size != aSize) {
        invalidate();
        m_Size = aSize;
        changeFont();
        invalidate();
    }
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
           double opacity, int size, const string& font, 
           const string& str, const string& color,
           AVGDFBDisplayEngine * pEngine, AVGContainer * pParent, AVGPlayer * pPlayer)
{
    AVGNode::init(id, pEngine, pParent, pPlayer);
    m_Size = size;
    m_Str = str;
    m_ColorName = color;
    m_Color = colorStringToColor(color);
    m_pBmp = getEngine()->createSurface();
    m_FontName = font;
    changeFont();
    initVisible(x, y, z, 0, 0, opacity); 
}

string AVGWords::getTypeStr ()
{
    return "AVGWords";
}

void AVGWords::changeFont()
{
    IDirectFB * pDFB = getEngine()->getDFB();
    m_pFont = getEngine()->getFontManager()->getFont(pDFB, m_FontName, m_Size);
    drawString();
}

void AVGWords::drawString()
{
    IDirectFB * pDFB = getEngine()->getDFB();

    DFBRectangle DFBExtents; 
    // TODO: This gets the logical extent of the string, not the ink rect.
    //       Change that and adjust the blit accordingly.
    m_pFont->GetStringExtents(m_pFont, m_Str.c_str(), -1, &DFBExtents, 0);
    m_StringExtents=PLPoint(DFBExtents.w, DFBExtents.h);
    if (m_StringExtents.x == 0) {
        m_StringExtents = PLPoint(1,1);
    }
    m_pBmp->Create(m_StringExtents.x, m_StringExtents.y, 8, false, false);
    m_pBmp->ApplyFilter(PLFilterFill<PLPixel8>(PLPixel8(0x0)));
    IDirectFBSurface * pSurface = dynamic_cast<PLDirectFBBmp*>(m_pBmp)->GetSurface();
    getEngine()->dumpSurface(pSurface, "Text field.");
    pSurface->SetColor(pSurface, 0xFF, 0xFF, 0xFF, 0xFF);

    pSurface->SetDrawingFlags(pSurface, DSDRAW_BLEND);
    pSurface->SetFont(pSurface, m_pFont);
    DFBResult err = pSurface->DrawString(pSurface, m_Str.c_str(), -1, 0, 0, 
            DFBSurfaceTextFlags(DSTF_LEFT | DSTF_TOP));
    getEngine()->DFBErrorCheck(AVG_ERR_FONT_INIT_FAILED, "AVGWords::drawString", err);    
    setViewport(-32767, -32767, m_StringExtents.x, m_StringExtents.y);
}

void AVGWords::render(const PLRect& Rect)
{
    if (getEffectiveOpacity() > 0.001) {
        bool bVisible = getEngine()->setClipRect(getAbsViewport());
        if (bVisible) {
            PLRect SrcRect(0, 0, getRelViewport().Width(), getRelViewport().Height());
            if (getRelViewport().tl.x < 0) {
                SrcRect.tl.x = -getRelViewport().tl.x;
            }
            if (getRelViewport().tl.y < 0) {
                SrcRect.tl.y = -getRelViewport().tl.y;
            }
            getEngine()->blta8(m_pBmp, &SrcRect, getAbsViewport().tl, getEffectiveOpacity(),
                    m_Color);
        }
    }
}

PLPixel32 AVGWords::colorStringToColor(const string & colorString)
{   
    int r,g,b;
    sscanf(colorString.c_str(), "%2x%2x%2x", &r, &g, &b);
    return PLPixel32(r,g,b);
}

PLPoint AVGWords::getPreferredMediaSize()
{
    return m_StringExtents;
}

