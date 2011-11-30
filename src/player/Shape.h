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

#ifndef _Shape_H_
#define _Shape_H_

#include "../api.h"
#include "Image.h"
#include "MaterialInfo.h"

#include "../base/GLMHelper.h"
#include "../graphics/Bitmap.h"
#include "../graphics/VertexArray.h"

#include <boost/shared_ptr.hpp>
#include <string>

namespace avg {

class AVG_API Shape
{
    public:
        Shape(const MaterialInfo& material);
        virtual ~Shape();

        void setBitmap(BitmapPtr pBmp);

        virtual void moveToGPU();
        virtual void moveToCPU();

        ImagePtr getImage();
        VertexArrayPtr getVertexArray();
        void draw();

        void discard();

    private:
        bool isTextured() const;

        VertexArrayPtr m_pVertexArray;
        OGLSurface * m_pSurface;
        ImagePtr m_pImage;
};

typedef boost::shared_ptr<Shape> ShapePtr;

}

#endif

