//
// $Id$
// 


#include "AVGDFBFont.h"
#include "AVGException.h"
#include "IAVGSurface.h"
#include "AVGDFBSurface.h"

#include <paintlib/plbitmap.h>
#include <paintlib/Filter/plfilterfill.h>
#include <paintlib/plpixel8.h>

using namespace std;

AVGDFBFont::AVGDFBFont(IDirectFB * pDFB, const std::string & Filename, int Size)
    : m_pDFB(pDFB)
{
    DFBFontDescription fontDesc;

    fontDesc.flags = DFBFontDescriptionFlags(DFDESC_HEIGHT | DFDESC_ATTRIBUTES);
    fontDesc.height = Size;
    fontDesc.attributes = (DFBFontAttributes)0;

    DFBResult err = m_pDFB->CreateFont(pDFB, Filename.c_str(), &fontDesc, &m_pFont);
    if (err) {
        throw AVGException(AVG_ERR_FONT_INIT_FAILED, 
                string("Font init failed: ") + DirectFBErrorString(err));
    }
    
}

AVGDFBFont::~AVGDFBFont()
{
    m_pFont->Release(m_pFont);
}
 
void AVGDFBFont::render(IAVGSurface& Surface, const std::string & Text)
{
    DFBRectangle DFBExtents; 
    // TODO: This gets the logical extent of the string, not the ink rect.
    //       Change that and adjust the blit accordingly.
    m_pFont->GetStringExtents(m_pFont, Text.c_str(), -1, &DFBExtents, 0);
    PLPoint StringExtents=PLPoint(DFBExtents.w, DFBExtents.h);
    if (StringExtents.x == 0) {
        StringExtents = PLPoint(1,1);
    }
    Surface.create(StringExtents.x, StringExtents.y, 8, false);    

    IDirectFBSurface * pSurface = 
            dynamic_cast<AVGDFBSurface&>(Surface).getSurface();
//    Bmp.ApplyFilter(PLFilterFill<PLPixel8>(PLPixel8(0x0)));
    pSurface->SetColor(pSurface, 0x00, 0x00, 0x00, 0x00);
    pSurface->FillRectangle(pSurface, 0, 0, StringExtents.x, StringExtents.y);
    
    pSurface->SetColor(pSurface, 0xFF, 0xFF, 0xFF, 0xFF);
    pSurface->SetDrawingFlags(pSurface, DSDRAW_BLEND);
    pSurface->SetFont(pSurface, m_pFont);
    DFBResult err = pSurface->DrawString(pSurface, Text.c_str(), -1, 0, 0, 
            DFBSurfaceTextFlags(DSTF_LEFT | DSTF_TOP));
    //TODO: Make display engine accessible here.
//    dynamic_cast<AVGDFBDisplayEngine*>(getEngine())->DFBErrorCheck(AVG_ERR_FONT_INIT_FAILED, "AVGWords::drawString", err);    
}


