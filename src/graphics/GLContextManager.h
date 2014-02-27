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
#ifndef _GLContextManager_H_
#define _GLContextManager_H_

#include "../api.h"

#include "PixelFormat.h"
#include "GLContext.h"

#include <map>

namespace avg {

class GLTexture;
typedef boost::shared_ptr<GLTexture> GLTexturePtr;
class MCTexture;
typedef boost::shared_ptr<MCTexture> MCTexturePtr;
class Bitmap;
typedef boost::shared_ptr<Bitmap> BitmapPtr;
class VertexArray;
typedef boost::shared_ptr<VertexArray> VertexArrayPtr;
class MCFBO;
typedef boost::shared_ptr<MCFBO> MCFBOPtr;

class AVG_API GLContextManager
{
public:
    static GLContextManager* get();
    static bool exists();

    GLContextManager();
    virtual ~GLContextManager();

    GLContext* createContext(const GLConfig& glConfig, 
            const IntPoint& windowSize=IntPoint(0,0), const SDL_SysWMinfo* pSDLWMInfo=0);
    void registerContext(GLContext* pContext);
    void unregisterContext(GLContext* pContext);

    MCTexturePtr createTexture(const IntPoint& size, PixelFormat pf, bool bMipmap=false,
            unsigned wrapSMode=GL_CLAMP_TO_EDGE, unsigned wrapTMode=GL_CLAMP_TO_EDGE,
            bool bForcePOT=false, int potBorderColor=0);
    GLTexturePtr createGLTexture(const IntPoint& size, PixelFormat pf, 
            bool bMipmap, unsigned wrapSMode, unsigned wrapTMode, 
            bool bForcePOT, int potBorderColor);
    MCFBOPtr createFBO(const IntPoint& size, PixelFormat pf, unsigned numTextures=1, 
            unsigned multisampleSamples=1, bool bUsePackedDepthStencil=false,
            bool bUseStencil=false, bool bMipmap=false,
            unsigned wrapSMode=GL_CLAMP_TO_EDGE, unsigned wrapTMode=GL_CLAMP_TO_EDGE);
    void createShader(const std::string& sID);

    void scheduleTexUpload(MCTexturePtr pTex, BitmapPtr pBmp);
    MCTexturePtr createTextureFromBmp(BitmapPtr pBmp, bool bMipmap=false, 
            unsigned wrapSMode=GL_CLAMP_TO_EDGE, unsigned wrapTMode=GL_CLAMP_TO_EDGE,
            bool bForcePOT=false, int potBorderColor=0);
    GLTexturePtr createGLTextureFromBmp(BitmapPtr pBmp, bool bMipmap=false,
            unsigned wrapSMode=GL_CLAMP_TO_EDGE, unsigned wrapTMode=GL_CLAMP_TO_EDGE, 
            bool bForcePOT=false, int potBorderColor=0);
    void deleteTexture(unsigned texID);

    VertexArrayPtr createVertexArray(int reserveVerts = 0, int reserveIndexes = 0);
    typedef std::map<GLContext*, unsigned> BufferIDMap;
    void deleteBuffers(BufferIDMap& bufferIDs);

    void uploadData();
    void uploadDataForContext();
    void reset();

    static bool isGLESSupported();

private:
    std::vector<GLContext*> m_pContexts;

    std::vector<MCTexturePtr> m_pPendingTexCreates;
    typedef std::map<MCTexturePtr, BitmapPtr> TexUploadMap;
    TexUploadMap m_pPendingTexUploads;
    std::vector<unsigned> m_PendingTexDeletes;

    std::vector<MCFBOPtr> m_pPendingFBOCreates;

    std::vector<VertexArrayPtr> m_pPendingVACreates;
    std::vector<BufferIDMap> m_PendingBufferDeletes;

    static GLContextManager* s_pGLContextManager;
};

}
#endif


