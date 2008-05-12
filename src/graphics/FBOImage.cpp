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

FBOImage::FBOImage(const IntPoint& size, PixelFormat pf, int precision, 
            bool bUseInputPBO, bool bUseOutputPBO)
    : PBOImage(size, pf, precision, bUseInputPBO, bUseOutputPBO)
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
}
    
void FBOImage::activate()
{
    glproc::BindFramebuffer(GL_FRAMEBUFFER_EXT, m_FBO);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "FBOImage::activate: BindFramebuffer()");
    
}

void FBOImage::deactivate()
{
    glproc::BindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "FBOImage::deactivate: BindFramebuffer()");
    
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
