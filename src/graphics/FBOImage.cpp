//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2006 Ulrich von Zadow
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

namespace avg {

FBOImage::FBOImage(IntPoint size, PixelFormat pf)
    : m_pf(pf),
      m_Size(size)
{
    // Create a Pixel Buffer Object for data transfer and allocate its memory.
    glproc::GenBuffers(1, &m_PBO);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "FBOImage: GenBuffers()");
    glproc::BindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, m_PBO);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "FBOImage: BindBuffer()");
    int MemNeeded = m_Size.x*m_Size.y*Bitmap::getBytesPerPixel(m_pf);
    glproc::BufferData(GL_PIXEL_UNPACK_BUFFER_EXT, MemNeeded, 0,
            GL_STREAM_DRAW);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "FBOImage: BufferData()");

    glGenTextures(1, &m_TexID);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "FBOImage: glGenTextures()");

    glproc::GenFramebuffers(1, &m_FBO);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "FBOImage: GenFramebuffers()");

    //bind the framebuffer, so operations will now occur on it
    glproc::BindFramebuffer(GL_FRAMEBUFFER_EXT, m_FBO);

    // initialize texture that will store the framebuffer image
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, m_TexID);

    int OGLMode = getOGLMode(pf);
    glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, OGLMode, size.x, size.y, 0,
            OGLMode, getOGLPixelType(pf), 0);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "FBOImage: glTexImage2D()");

    // bind this texture to the current framebuffer obj. as 
    // color_attachement_0 
    glproc::FramebufferTexture2D(GL_FRAMEBUFFER_EXT,
            GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_ARB, m_TexID, 0);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "FBOImage: glFramebufferTexture2D()");

    checkError();
    
    glproc::BindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
}

FBOImage::~FBOImage()
{
    glproc::DeleteFramebuffers(1, &m_FBO);
    glDeleteTextures(1, &m_TexID);
    glproc::DeleteBuffers(1, &m_PBO);
}

void FBOImage::setImage(BitmapPtr pBmp)
{
    assert (pBmp->getSize() == m_Size);
    glproc::BindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, m_PBO);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "FBOImage::setImage BindBuffer()");
    void * pPBOPixels = glproc::MapBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, GL_WRITE_ONLY);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "FBOImage::setImage MapBuffer()");
    memcpy(pPBOPixels, pBmp->getPixels(), pBmp->getMemNeeded());
    
    if (!glproc::UnmapBuffer(GL_PIXEL_UNPACK_BUFFER_EXT)) {
        std::cerr << "Couldn't unmap pixel buffer. Exiting\n";
        assert(false);
    }

    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, m_TexID);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "FBOImage::setImage: glBindTexture()");
    glPixelStorei(GL_UNPACK_ROW_LENGTH, pBmp->getStride()/pBmp->getBytesPerPixel());
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "FBOImage::setImage: glPixelStorei(GL_UNPACK_ROW_LENGTH)");
    int OGLMode = getOGLMode(m_pf);
    glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, OGLMode, m_Size.x, m_Size.y, 0,
            OGLMode, getOGLPixelType(m_pf), 0);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "FBOImage::setImage: glTexImage2D()");
    glproc::BindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, 0);
}

BitmapPtr FBOImage::getImage() const
{
    BitmapPtr pBmp(new Bitmap(m_Size, m_pf));

    glproc::BindBuffer(GL_PIXEL_PACK_BUFFER_EXT, m_PBO);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "FBOImage::getImage BindBuffer()");

    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, m_TexID);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "FBOImage::getImage: glBindTexture()");
    glPixelStorei(GL_PACK_ROW_LENGTH, pBmp->getStride()/pBmp->getBytesPerPixel());
    int OGLMode = getOGLMode(m_pf);
    glGetTexImage(GL_TEXTURE_RECTANGLE_ARB, 0, OGLMode, getOGLPixelType(m_pf), 0);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "FBOImage::getImage: glGetTexImage()");

    void * pPBOPixels = glproc::MapBuffer(GL_PIXEL_PACK_BUFFER_EXT, GL_READ_ONLY);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "FBOImage::getImage MapBuffer()");
    memcpy(pBmp->getPixels(), pPBOPixels, pBmp->getMemNeeded());

    glproc::BindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, 0);
    return pBmp;
}

PixelFormat FBOImage::getPF() const
{
    return m_pf;
}

const IntPoint& FBOImage::getSize() const
{
    return m_Size;
}

bool FBOImage::isFBOSupported()
{
    return queryOGLExtension("GL_EXT_framebuffer_object");
}
    
int FBOImage::getOGLMode(PixelFormat pf) const
{
    switch (pf) {
        case B8G8R8X8:
        case B8G8R8A8:
            return GL_RGBA;
        default:
            AVG_TRACE(Logger::ERROR, "Unsupported pixel format " << 
                    Bitmap::getPixelFormatString(pf) <<
                    " in FBOImage::getOGLMode()");
            assert(false);
    }
    return 0;
}

int FBOImage::getOGLPixelType(PixelFormat pf) const
{
    if (pf == I16) {
        return GL_UNSIGNED_SHORT;
    } else {
        return GL_UNSIGNED_BYTE;
    }
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
