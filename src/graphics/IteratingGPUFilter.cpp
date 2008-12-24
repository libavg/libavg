// Copyright (C) 2008 Archimedes Solutions GmbH,
// Saarbrücker Str. 24b, Berlin, Germany
//
// This file contains proprietary source code and confidential
// information. Its contents may not be disclosed or distributed to
// third parties unless prior specific permission by Archimedes
// Solutions GmbH, Berlin, Germany is obtained in writing. This applies
// to copies made in any form and using any medium. It applies to
// partial as well as complete copies.

#include "IteratingGPUFilter.h"

#include <base/Exception.h>
#include <graphics/PBOImage.h>
#include <graphics/GPUFilter.h>
#include <graphics/OGLShader.h>
#include <graphics/OGLHelper.h>
#include <graphics/OGLImagingContext.h>

using namespace std;

namespace avg {

IteratingGPUFilter::IteratingGPUFilter(const IntPoint& size, int numIterations)
    : m_Size(size),
      m_NumIterations(numIterations)
{
    m_pSrcPBO = PBOImagePtr(new PBOImage(m_Size, R32G32B32A32F, R32G32B32A32F, 
            true, false));
    m_pDestPBO = PBOImagePtr(new PBOImage(m_Size, R32G32B32A32F, R32G32B32A32F, 
            false, true));
    createFBO();
}

IteratingGPUFilter::~IteratingGPUFilter()
{
}

BitmapPtr IteratingGPUFilter::apply(BitmapPtr pImage)
{
    m_pSrcPBO->setImage(pImage);
    applyOnGPU();
    BitmapPtr pDestImage;
    return getImage();
}

void IteratingGPUFilter::applyOnGPU()
{
    for(int i=0;i<2;i++) {
        glproc::ActiveTexture(GL_TEXTURE0+i);        
        glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    activate();
    checkError();
    for(int k=0; k<m_NumIterations; k++) {
        glDrawBuffer(GL_COLOR_ATTACHMENT1_EXT);
        applyOnce(m_pSrcPBO);
        glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
        applyOnce(m_pDestPBO);
    }
    glDrawBuffer(GL_COLOR_ATTACHMENT1_EXT);
    applyOnce(m_pSrcPBO);
    
    deactivate();
}

BitmapPtr IteratingGPUFilter::getImage() const
{
    activate();
    PixelFormat pf = m_pDestPBO->getExtPF();
    IntPoint size = m_pDestPBO->getSize();
    BitmapPtr pBmp(new Bitmap(m_pDestPBO->getSize(), pf));
    glproc::BindBuffer(GL_PIXEL_PACK_BUFFER_EXT, m_pDestPBO->getOutputPBO());
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "FBOImage::getImage BindBuffer()");
    glReadBuffer(GL_COLOR_ATTACHMENT1_EXT);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "FBOImage::getImage ReadBuffer()");
    
    glReadPixels (0, 0, size.x, size.y, m_pDestPBO->getFormat(pf), 
            m_pDestPBO->getType(pf), 0);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "FBOImage::getImage ReadPixels()");
    void * pPBOPixels = glproc::MapBuffer(GL_PIXEL_PACK_BUFFER_EXT, GL_READ_ONLY);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "FBOImage::getImage MapBuffer()");
    Bitmap PBOBitmap(size, pf, (unsigned char *)pPBOPixels, 
            size.x*Bitmap::getBytesPerPixel(pf), false);
    pBmp->copyPixels(PBOBitmap);
    glproc::UnmapBuffer(GL_PIXEL_PACK_BUFFER_EXT);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "FBOImage::getImage: UnmapBuffer()");
    glproc::BindBuffer(GL_PIXEL_PACK_BUFFER_EXT, 0);
    deactivate();
    return pBmp;
}
 
void IteratingGPUFilter::createFBO()
{
    glproc::GenFramebuffers(1, &m_FBO);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "IteratingGPUFilter: GenFramebuffers()");

    activate();

    glproc::FramebufferTexture2D(GL_FRAMEBUFFER_EXT,
            GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_ARB, 
            m_pSrcPBO->getTexID(), 0);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "IteratingGPUFilter: glFramebufferTexture2D()");
    glproc::FramebufferTexture2D(GL_FRAMEBUFFER_EXT,
            GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_RECTANGLE_ARB, 
            m_pDestPBO->getTexID(), 0);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "IteratingGPUFilter: glFramebufferTexture2D()");

    checkError();
    deactivate();
}

void IteratingGPUFilter::activate() const
{
    glproc::BindFramebuffer(GL_FRAMEBUFFER_EXT, m_FBO);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "IteratingGPUFilter::activate: BindFramebuffer()");
}

void IteratingGPUFilter::deactivate() const
{
    glproc::BindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "IteratingGPUFilter::deactivate: BindFramebuffer()");
}

void IteratingGPUFilter::checkError() const
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

