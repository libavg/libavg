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

#ifndef _Shape_H_
#define _Shape_H_

#include "../api.h"

#include "../base/GLMHelper.h"
#include "../graphics/SubVertexArray.h"
#include "../graphics/WrapMode.h"

#include <boost/shared_ptr.hpp>

namespace avg {

class Bitmap;
typedef boost::shared_ptr<Bitmap> BitmapPtr;
class GPUImage;
typedef boost::shared_ptr<GPUImage> GPUImagePtr;
class OGLSurface;

class AVG_API Shape
{
    public:
        Shape(const WrapMode& wrapMode, bool bUseMipmaps);
        virtual ~Shape();

        void setBitmap(BitmapPtr pBmp);

        virtual void moveToGPU();
        virtual void moveToCPU();

        GPUImagePtr getGPUImage();
        void setVertexData(VertexDataPtr pVertexData);
        void setVertexArray(const VertexArrayPtr& pVA);
        void draw(GLContext* pContext, const glm::mat4& transform, float opacity);
        bool isPtInside(const glm::vec2& pos);

        void discard();

    private:
        VertexDataPtr m_pVertexData;
        SubVertexArray m_SubVA;
        OGLSurface * m_pSurface;
        GPUImagePtr m_pGPUImage;
        FRect m_Bounds;
};

typedef boost::shared_ptr<Shape> ShapePtr;

}

#endif

