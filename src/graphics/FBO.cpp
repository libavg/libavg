//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2008 Ulrich von Zadow
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  Current versions can be found at www.libavg.de
//

#include "FBO.h"

#include "../base/Exception.h"
#include "../graphics/PBOImage.h"
#include "../graphics/OGLHelper.h"

using namespace std;

namespace avg {

FBO::FBO(const IntPoint& size, PixelFormat pf, unsigned texID)
    : m_Size(size),
      m_PF(pf)
{
    m_TexIDs.push_back(texID);
    init();
}

FBO::FBO(const IntPoint& size, PixelFormat pf, vector<unsigned> texIDs)
    : m_Size(size),
      m_PF(pf),
      m_TexIDs(texIDs)
{
    m_TexIDs = texIDs;
    init();
}

FBO::~FBO()
{
    glproc::DeleteFramebuffers(1, &m_FBO);
}

BitmapPtr FBO::getImage(int i) const
{
    activate();
    PixelFormat pf = m_pOutputPBO->getExtPF();
    IntPoint size = m_pOutputPBO->getSize();
    BitmapPtr pBmp(new Bitmap(m_pOutputPBO->getSize(), pf));
    glproc::BindBuffer(GL_PIXEL_PACK_BUFFER_EXT, m_pOutputPBO->getOutputPBO());
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "FBO::getImage BindBuffer()");
    glReadBuffer(GL_COLOR_ATTACHMENT0_EXT+i);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "FBO::getImage ReadBuffer()");
    
    glReadPixels (0, 0, size.x, size.y, m_pOutputPBO->getFormat(pf), 
            m_pOutputPBO->getType(pf), 0);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "FBO::getImage ReadPixels()");
    void * pPBOPixels = glproc::MapBuffer(GL_PIXEL_PACK_BUFFER_EXT, GL_READ_ONLY);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "FBO::getImage MapBuffer()");
    Bitmap PBOBitmap(size, pf, (unsigned char *)pPBOPixels, 
            size.x*Bitmap::getBytesPerPixel(pf), false);
    pBmp->copyPixels(PBOBitmap);
    glproc::UnmapBuffer(GL_PIXEL_PACK_BUFFER_EXT);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "FBO::getImage: UnmapBuffer()");
    glproc::BindBuffer(GL_PIXEL_PACK_BUFFER_EXT, 0);
    deactivate();
    return pBmp;
}
 
void FBO::init()
{
    m_pOutputPBO = PBOImagePtr(new PBOImage(m_Size, m_PF, m_PF, false, true));

    glproc::GenFramebuffers(1, &m_FBO);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "FBO: GenFramebuffers()");

    glproc::BindFramebuffer(GL_FRAMEBUFFER_EXT, m_FBO);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "FBO::activate: BindFramebuffer()");

    for (unsigned i=0; i<m_TexIDs.size(); ++i) {
        glproc::FramebufferTexture2D(GL_FRAMEBUFFER_EXT,
                GL_COLOR_ATTACHMENT0_EXT+i, GL_TEXTURE_RECTANGLE_ARB, 
                m_TexIDs[i], 0);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "FBO: glFramebufferTexture2D()");
    }

    checkError();
    glproc::BindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
}

void FBO::activate() const
{
    glproc::BindFramebuffer(GL_FRAMEBUFFER_EXT, m_FBO);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "FBO::activate: BindFramebuffer()");
    checkError();
}

void FBO::deactivate() const
{
    glproc::BindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "FBO::deactivate: BindFramebuffer()");
}

bool FBO::isFBOSupported()
{
    return queryOGLExtension("GL_EXT_framebuffer_object");
}
    
void FBO::checkError() const
{
    GLenum status;
    status = glproc::CheckFramebufferStatus(GL_FRAMEBUFFER_EXT);
    if (status == GL_FRAMEBUFFER_COMPLETE_EXT) {
        return;
    } else {
        fprintf(stderr, "Framebuffer error: %i\n", status);
        switch(status) {
            case GL_FRAMEBUFFER_COMPLETE_EXT:
                fprintf(stderr,"framebuffer complete!\n");
                break;
            case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
                fprintf(stderr,"framebuffer GL_FRAMEBUFFER_UNSUPPORTED_EXT\n");
                /* you gotta choose different formats */ \
                assert(0);
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
                fprintf(stderr,"framebuffer INCOMPLETE_ATTACHMENT\n");
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
                fprintf(stderr,"framebuffer FRAMEBUFFER_MISSING_ATTACHMENT\n");
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
                fprintf(stderr,"framebuffer FRAMEBUFFER_DIMENSIONS\n");
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT_EXT:
                fprintf(stderr,"framebuffer INCOMPLETE_DUPLICATE_ATTACHMENT\n");
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
                fprintf(stderr,"framebuffer INCOMPLETE_FORMATS\n");
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
                fprintf(stderr,"framebuffer INCOMPLETE_DRAW_BUFFER\n");
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
                fprintf(stderr,"framebuffer INCOMPLETE_READ_BUFFER\n");
                break;
            case GL_FRAMEBUFFER_BINDING_EXT:
                fprintf(stderr,"framebuffer BINDING_EXT\n");
                break;
            default:
                /* programming error; will fail on all hardware */
                assert(false);
        }
        assert(false);
    }
}

} // namespace avg


