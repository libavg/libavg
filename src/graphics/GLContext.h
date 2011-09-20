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
#include "GLConfig.h"

#include "../base/Point.h"

#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
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
    GLContext(bool bUseCurrent=false, 
            const GLConfig& GLConfig=GLConfig(false, true, true, 1), 
            GLContext* pSharedContext=0);
    virtual ~GLContext();
    void init();

    void activate();
    ShaderRegistryPtr getShaderRegistry() const;

    virtual void pushTransform(const DPoint& translate, double angle, 
            const DPoint& pivot);
    virtual void popTransform();

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

    const GLConfig& getConfig();
    void logConfig();
    int getMaxTexSize();
    bool usePOTTextures();
    OGLMemoryMode getMemoryModeSupported();
    bool isUsingShaders() const;
    
    static BlendMode stringToBlendMode(const std::string& s);

    static GLContext* getCurrent();

private:
    void checkShaderSupport();

#ifdef __APPLE__
    CGLContextObj m_Context;
#elif defined linux
    Display* m_pDisplay;
    GLXDrawable m_Drawable;
    GLXContext m_Context;
#elif defined _WIN32
    HWND m_hwnd;
    HDC m_hDC;
    HGLRC m_Context;
#endif
 
    bool m_bOwnsContext;

    ShaderRegistryPtr m_pShaderRegistry;

    GLBufferCache m_VertexBufferCache;
    GLBufferCache m_IndexBufferCache;
    GLBufferCache m_PBOCache;
    std::vector<unsigned int> m_FBOIDs;

    int m_MaxTexSize;
    GLConfig m_GLConfig;
    bool m_bCheckedMemoryMode;
    OGLMemoryMode m_MemoryMode;

    // OpenGL state
    bool m_bEnableTexture;
    bool m_bEnableGLColorArray;
    BlendMode m_BlendMode;
    bool m_bPremultipliedAlpha;

    static boost::thread_specific_ptr<GLContext*> s_pCurrentContext;

};

}
#endif


