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

#include "FBOImage.h"
#include "OGLHelper.h"

#include "../base/Logger.h"
#include "../base/Exception.h"

#include <cstdio>

namespace avg {

FBOImage::FBOImage(const IntPoint& size, PixelFormat pfInternal, PixelFormat pfExternal,
            bool bUseInputPBO, bool bUseOutputPBO)
    : PBOImage(size, pfInternal, pfExternal, bUseInputPBO, bUseOutputPBO)
{
    glproc::GenFramebuffers(1, &m_FBO);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "FBOImage: GenFramebuffers()");

    //bind the framebuffer, so operations will now occur on it
    glproc::BindFramebuffer(GL_FRAMEBUFFER_EXT, m_FBO);

    // bind this texture to the current framebuffer obj. as 
    // color_attachement_0 
    glproc::FramebufferTexture2D(GL_FRAMEBUFFER_EXT,
            GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_ARB, getTexID(), 0);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "FBOImage: glFramebufferTexture2D()");

    checkError();
    
    glproc::BindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
}

FBOImage::~FBOImage()
{
    glproc::DeleteFramebuffers(1, &m_FBO);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "FBOImage::~FBOImage: DeleteFramebuffers");
}
    
void FBOImage::activate() const
{
    glproc::BindFramebuffer(GL_FRAMEBUFFER_EXT, m_FBO);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "FBOImage::activate: BindFramebuffer()");
    
}

void FBOImage::deactivate() const
{
    glproc::BindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "FBOImage::deactivate: BindFramebuffer()");
    
}

BitmapPtr FBOImage::getImage() const
{
    activate();
    BitmapPtr pBmp(new Bitmap(getSize(), getExtPF()));
    glproc::BindBuffer(GL_PIXEL_PACK_BUFFER_EXT, getOutputPBO());
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "FBOImage::getImage BindBuffer()");
    glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "FBOImage::getImage ReadBuffer()");
    
    int memNeeded = getSize().x*getSize().y * Bitmap::getBytesPerPixel(getExtPF());
    glproc::BufferData(GL_PIXEL_PACK_BUFFER_EXT, memNeeded, 0, GL_STREAM_READ);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "FBOImage::getImage BufferData()");
    glReadPixels (0, 0, getSize().x, getSize().y, getFormat(getExtPF()), 
            getType(getExtPF()), 0);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "FBOImage::getImage ReadPixels()");
    void * pPBOPixels = glproc::MapBuffer(GL_PIXEL_PACK_BUFFER_EXT, GL_READ_ONLY);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "FBOImage::getImage MapBuffer()");
    Bitmap PBOBitmap(getSize(), getExtPF(), (unsigned char *)pPBOPixels, 
            getSize().x*Bitmap::getBytesPerPixel(getExtPF()), false);
    pBmp->copyPixels(PBOBitmap);
    glproc::UnmapBuffer(GL_PIXEL_PACK_BUFFER_EXT);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "FBOImage::getImage: UnmapBuffer()");
    glproc::BindBuffer(GL_PIXEL_PACK_BUFFER_EXT, 0);
    deactivate();
    return pBmp;
}


bool FBOImage::isFBOSupported()
{
    return queryOGLExtension("GL_EXT_framebuffer_object");
}
    
void FBOImage::checkError() const
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

}
