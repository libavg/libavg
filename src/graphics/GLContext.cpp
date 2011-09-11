//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2011 Ulrich von Zadow
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

#include "GLContext.h"

#include "ShaderRegistry.h"

#include "../base/Exception.h"
#include "../base/Logger.h"
#include "../base/MathHelper.h"

namespace avg {

using namespace std;
using namespace boost;
thread_specific_ptr<GLContext*> GLContext::s_pCurrentContext;

GLContext::GLContext(bool bUseCurrent)
    : m_Context(0),
      m_bEnableTexture(false),
      m_bEnableGLColorArray(true),
      m_BlendMode(BLEND_ADD),
      m_MaxTexSize(0)
{
    if (s_pCurrentContext.get() == 0) {
        s_pCurrentContext.reset(new (GLContext*));
    }
    if (bUseCurrent) {
#if defined(__APPLE__)
        m_Context = aglGetCurrentContext();
#elif defined(__linux__)
        m_pDisplay = glXGetCurrentDisplay();
        m_Drawable = glXGetCurrentDrawable();
        m_Context = glXGetCurrentContext();
#elif defined(_WIN32)
        m_hDC = wglGetCurrentDC();
        m_Context = wglGetCurrentContext();
#endif
        *s_pCurrentContext = this;
        init();
    }
}

GLContext::~GLContext()
{
    for (unsigned i=0; i<m_FBOIDs.size(); ++i) {
        glproc::DeleteFramebuffers(1, &(m_FBOIDs[i]));
    }
    m_FBOIDs.clear();
}

void GLContext::init()
{
    glproc::init();
    m_pShaderRegistry = ShaderRegistryPtr(new ShaderRegistry());
    enableGLColorArray(false);
    setBlendMode(BLEND_BLEND, false);
}

void GLContext::activate()
{
#ifdef __APPLE__
    bool bOk = aglSetCurrentContext(m_Context);
    AVG_ASSERT(bOk);
#elif defined linux
    glXMakeCurrent(m_pDisplay, m_Drawable, m_Context);
#elif defined _WIN32
    BOOL bOk = wglMakeCurrent(m_hDC, m_Context);
    winOGLErrorCheck(bOK, "wglMakeCurrent");
#endif
    *s_pCurrentContext = this;
}

ShaderRegistryPtr GLContext::getShaderRegistry() const
{
    return m_pShaderRegistry;
}

void GLContext::pushTransform(const DPoint& translate, double angle, const DPoint& pivot)
{
    glPushMatrix();
    glTranslated(translate.x, translate.y, 0);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "pushTransform: glTranslated");
    glTranslated(pivot.x, pivot.y, 0);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "pushTransform: glTranslated");
    glRotated(angle*180.0/PI, 0, 0, 1);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "pushTransform: glRotated");
    glTranslated(-pivot.x, -pivot.y, 0);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "pushTransform: glTranslated");
}

void GLContext::popTransform()
{
    glPopMatrix();
}

GLBufferCache& GLContext::getVertexBufferCache()
{
    return m_VertexBufferCache;
}

GLBufferCache& GLContext::getIndexBufferCache()
{
    return m_IndexBufferCache;
}

GLBufferCache& GLContext::getPBOCache()
{
    return m_PBOCache;
}

unsigned GLContext::genFBO()
{
    unsigned fboID;
    if (m_FBOIDs.empty()) {
        glproc::GenFramebuffers(1, &fboID);
    } else {
        fboID = m_FBOIDs.back();
        m_FBOIDs.pop_back();
    }
    return fboID;
}

void GLContext::returnFBOToCache(unsigned fboID) 
{
    m_FBOIDs.push_back(fboID);
}

void GLContext::enableTexture(bool bEnable)
{
    if (bEnable != m_bEnableTexture) {
        if (bEnable) {
            glEnable(GL_TEXTURE_2D);
        } else {
            glDisable(GL_TEXTURE_2D);
        }
        m_bEnableTexture = bEnable;
    }
}

void GLContext::enableGLColorArray(bool bEnable)
{
    if (bEnable != m_bEnableGLColorArray) {
        if (bEnable) {
            glEnableClientState(GL_COLOR_ARRAY);
        } else {
            glDisableClientState(GL_COLOR_ARRAY);
        }
        m_bEnableGLColorArray = bEnable;
    }
}

void checkBlendModeError(const char * sMode) 
{    
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        static bool bErrorReported = false;
        if (!bErrorReported) {
            AVG_TRACE(Logger::WARNING, "Blendmode "<< sMode <<
                    " not supported by OpenGL implementation.");
            bErrorReported = true;
        }
    }
}

void GLContext::setBlendMode(BlendMode mode, bool bPremultipliedAlpha)
{
    GLenum srcFunc;
    if (bPremultipliedAlpha) {
        srcFunc = GL_CONSTANT_ALPHA;
    } else {
        srcFunc = GL_SRC_ALPHA;
    }
    if (mode != m_BlendMode || m_bPremultipliedAlpha != bPremultipliedAlpha) {
        switch (mode) {
            case BLEND_BLEND:
                glproc::BlendEquation(GL_FUNC_ADD);
                glproc::BlendFuncSeparate(srcFunc, GL_ONE_MINUS_SRC_ALPHA, 
                        GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
                checkBlendModeError("blend");
                break;
            case BLEND_ADD:
                glproc::BlendEquation(GL_FUNC_ADD);
                glproc::BlendFuncSeparate(srcFunc, GL_ONE, GL_ONE, GL_ONE);
                checkBlendModeError("add");
                break;
            case BLEND_MIN:
                glproc::BlendEquation(GL_MIN);
                glproc::BlendFuncSeparate(srcFunc, GL_ONE_MINUS_SRC_ALPHA, 
                        GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
                checkBlendModeError("min");
                break;
            case BLEND_MAX:
                glproc::BlendEquation(GL_MAX);
                glproc::BlendFuncSeparate(srcFunc, GL_ONE_MINUS_SRC_ALPHA, 
                        GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
                checkBlendModeError("max");
                break;
            case BLEND_COPY:
                glproc::BlendEquation(GL_FUNC_ADD);
                glBlendFunc(GL_ONE, GL_ZERO);
                break;
            default:
                AVG_ASSERT(false);
        }

        m_BlendMode = mode;
        m_bPremultipliedAlpha = bPremultipliedAlpha;
    }
}

int GLContext::getMaxTexSize() 
{
    if (m_MaxTexSize == 0) {
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &m_MaxTexSize);
    }
    return m_MaxTexSize;
}

GLContext::BlendMode GLContext::stringToBlendMode(const string& s)
{
    if (s == "blend") {
        return GLContext::BLEND_BLEND;
    } else if (s == "add") {
        return GLContext::BLEND_ADD;
    } else if (s == "min") {
        return GLContext::BLEND_MIN;
    } else if (s == "max") {
        return GLContext::BLEND_MAX;
    } else {
        throw(Exception(AVG_ERR_UNSUPPORTED, "Blend mode "+s+" not supported."));
    }
}

GLContext* GLContext::getCurrent()
{
    return *s_pCurrentContext;
}

}
