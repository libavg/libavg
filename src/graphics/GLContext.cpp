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

namespace avg {

using namespace boost;
thread_specific_ptr<GLContext*> GLContext::s_pCurrentContext;

GLContext::GLContext(bool bUseCurrent)
    : m_Context(0)
{
    if (s_pCurrentContext.get() == 0) {
        s_pCurrentContext.reset(new (GLContext*));
    }
    if (bUseCurrent) {
#if defined(__APPLE__)
        m_Context = CGLGetCurrentContext();
#elif defined(__linux__)
        m_pDisplay = glXGetCurrentDisplay();
        m_Drawable = glXGetCurrentDrawable();
        m_Context = glXGetCurrentContext();
#elif defined(_WIN32)
        m_hDC = wglGetCurrentDC();
        m_Context = wglGetCurrentContext();
#endif
        *s_pCurrentContext = this;
    }
    m_pShaderRegistry = ShaderRegistryPtr(new ShaderRegistry());
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

GLContext* GLContext::getCurrent()
{
    return *s_pCurrentContext;
}

}
