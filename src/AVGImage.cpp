//
// $Id$
// 

#include "AVGImage.h"
#include "AVGSDLDisplayEngine.h"

#include <paintlib/plbitmap.h>
#include <paintlib/planydec.h>
#include <paintlib/Filter/plfilterresizebilinear.h>

using namespace std;

AVGImage::AVGImage (const string& id, int x, int y, int z, 
           int width, int height, double opacity, const string& filename, 
           AVGSDLDisplayEngine * pEngine, AVGContainer * pParent)
    : AVGVisibleNode(id, x, y, z, width, height, opacity, pEngine, pParent),
      m_Filename (filename)
{
    m_pBmp = getEngine()->createSurface();
    PLAnyPicDecoder decoder;
    decoder.MakeBmpFromFile(m_Filename.c_str(), m_pBmp);
    if (width == 0 || height == 0) {
        setViewport (x, y, x+m_pBmp->GetWidth(), y+m_pBmp->GetHeight());
    } else {
        if (m_pBmp->GetWidth() != width || m_pBmp->GetHeight() != height) {
            cerr << "Warning: size of image node with id " << id << 
                    " does not match bitmap size." << endl;
            m_pBmp->ApplyFilter(PLFilterResizeBilinear(width, height));   
        }
    }
}

AVGImage::~AVGImage ()
{
}

void AVGImage::render ()
{
    getEngine()->render(m_pBmp, getAbsViewport().tl);
}

void AVGImage::getDirtyRect ()
{

}

string AVGImage::getTypeStr ()
{
    return "AVGImage";
}

