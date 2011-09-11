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
#ifndef _GLContext_H_
#define _GLContext_H_
#include "../api.h"

#include "OGLHelper.h"
#include "GLBufferCache.h"

#ifdef __APPLE__
#include <AGL/agl.h>
#undef check // Conflicts with boost
#elif defined linux
#include <GL/glx.h>
#elif defined _WIN32
#include <gl/gl.h>
#include <gl/glu.h>
#endif

#include <boost/shared_ptr.hpp>
#include <boost/thread/tss.hpp>

namespace avg {

class GLContext;
typedef boost::shared_ptr<GLContext> GLContextPtr;
class ShaderRegistry;
typedef boost::shared_ptr<ShaderRegistry> ShaderRegistryPtr;

class AVG_API GLContext {
public:
    GLContext(bool bUseCurrent=false);
    virtual ~GLContext();
    void init();

    void activate();
    ShaderRegistryPtr getShaderRegistry() const;

    // GL Object caching.
    GLBufferCache& getVertexBufferCache();
    GLBufferCache& getIndexBufferCache();
    GLBufferCache& getPBOCache();
    unsigned genFBO();
    void returnFBOToCache(unsigned fboID);

    // GL state cache.
    void enableTexture(bool bEnable);
    void enableGLColorArray(bool bEnable);
    enum BlendMode {BLEND_BLEND, BLEND_ADD, BLEND_MIN, BLEND_MAX, BLEND_COPY};
    void setBlendMode(BlendMode mode, bool bPremultipliedAlpha = false);

    static BlendMode stringToBlendMode(const std::string& s);

    static GLContext* getCurrent();

protected:
#ifdef __APPLE__
    AGLContext m_Context;
#elif defined linux
    Display* m_pDisplay;
    GLXDrawable m_Drawable;
    GLXContext m_Context;
#elif defined _WIN32
    HDC m_hDC;
    HGLRC m_Context;
#endif
    
private:
    ShaderRegistryPtr m_pShaderRegistry;

    GLBufferCache m_VertexBufferCache;
    GLBufferCache m_IndexBufferCache;
    GLBufferCache m_PBOCache;
    std::vector<unsigned int> m_FBOIDs;

    // OpenGL state
    bool m_bEnableTexture;
    bool m_bEnableGLColorArray;
    BlendMode m_BlendMode;
    bool m_bPremultipliedAlpha;

    static boost::thread_specific_ptr<GLContext*> s_pCurrentContext;

};

}
#endif


