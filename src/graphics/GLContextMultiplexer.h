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
#ifndef _GLContextMultiplexer_H_
#define _GLContextMultiplexer_H_

#include "../api.h"

#include "PixelFormat.h"
#include "GLContext.h"

#include <map>

namespace avg {

class GLTexture;
typedef boost::shared_ptr<GLTexture> GLTexturePtr;
class Bitmap;
typedef boost::shared_ptr<Bitmap> BitmapPtr;
class VertexArray;
typedef boost::shared_ptr<VertexArray> VertexArrayPtr;

class AVG_API GLContextMultiplexer
{
public:
    static GLContextMultiplexer* get();
    static bool exists();

    GLContextMultiplexer();
    virtual ~GLContextMultiplexer();

    GLTexturePtr createTexture(const IntPoint& size, PixelFormat pf, bool bMipmap=false,
            int potBorderColor=0, unsigned wrapSMode=GL_CLAMP_TO_EDGE,
            unsigned wrapTMode=GL_CLAMP_TO_EDGE, bool bForcePOT=false);
    void scheduleTexUpload(GLTexturePtr pTex, BitmapPtr pBmp);
    GLTexturePtr createTextureFromBmp(BitmapPtr pBmp, bool bMipmap=false,
            int potBorderColor=0, unsigned wrapSMode=GL_CLAMP_TO_EDGE,
            unsigned wrapTMode=GL_CLAMP_TO_EDGE, bool bForcePOT=false);
    void deleteTexture(unsigned texID);

    VertexArrayPtr createVertexArray(int reserveVerts = 0, int reserveIndexes = 0);
    typedef std::map<GLContext*, unsigned> BufferIDMap;
    void deleteBuffers(BufferIDMap& bufferIDs);

    void uploadData();
    void reset();

private:
    std::vector<GLTexturePtr> m_pPendingTexCreates;
    typedef std::map<GLTexturePtr, BitmapPtr> TexUploadMap;
    TexUploadMap m_pPendingTexUploads;
    std::vector<unsigned> m_PendingTexDeletes;

    std::vector<VertexArrayPtr> m_pPendingVACreates;
    std::vector<BufferIDMap> m_PendingBufferDeletes;

    static GLContextMultiplexer* s_pGLContextMultiplexer;
};

typedef boost::shared_ptr<GLContextMultiplexer> GLContextMultiplexerPtr;

}
#endif


