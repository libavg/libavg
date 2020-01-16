//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2014 Ulrich von Zadow
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

#ifndef _FXNode_H_
#define _FXNode_H_

#include "../api.h"

#include "../base/GLMHelper.h"
#include "../base/Rect.h"

#include <boost/shared_ptr.hpp>

namespace avg {

class Bitmap;
typedef boost::shared_ptr<Bitmap> BitmapPtr;
class FBO;
typedef boost::shared_ptr<FBO> FBOPtr;
class GPUFilter;
typedef boost::shared_ptr<GPUFilter> GPUFilterPtr;
class GLTexture;
typedef boost::shared_ptr<GLTexture> GLTexturePtr;
class GLContext;

class AVG_API FXNode {
public:
    FXNode(bool bSupportsGLES=true);
    virtual ~FXNode();

    virtual void connect();
    virtual void disconnect();
    virtual void setSize(const IntPoint& newSize);

    virtual void apply(GLContext* pContext, GLTexturePtr pSrcTex);

    GLTexturePtr getTex(GLContext* pContext);
    BitmapPtr getImage(GLContext* pContext);
    FRect getRelDestRect() const;

    bool isDirty() const;
    void resetDirty();

protected:
    FBOPtr getFBO(GLContext* pContext);
    void setDirty();

private:
    virtual GPUFilterPtr createFilter(const IntPoint& size) = 0;
    void checkGLES() const;

    IntPoint m_Size;
    GPUFilterPtr m_pFilter;
    
    bool m_bSupportsGLES;
    bool m_bDirty;
};

typedef boost::shared_ptr<FXNode> FXNodePtr;

}

#endif

