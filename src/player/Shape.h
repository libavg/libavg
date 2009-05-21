//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2008 Ulrich von Zadow
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
#include "OGLTexture.h"

#include "../base/Point.h"
#include "../graphics/Bitmap.h"
#include "../graphics/VertexArray.h"

#include <boost/shared_ptr.hpp>
#include <string>

namespace avg {

class AVG_API Shape: public Image
{
    public:
        Shape(const std::string& sFilename, int texWrapSMode, int texWrapTMode);
        virtual ~Shape();

        void setBitmap(const Bitmap * pBmp);

        virtual void moveToGPU(SDLDisplayEngine* pEngine);
        virtual void moveToCPU();

        VertexArrayPtr getVertexArray();
        void draw();

    private:
        void downloadTexture();

        OGLTexturePtr m_pTexture;
        int m_TexWrapSMode;
        int m_TexWrapTMode;
        VertexArrayPtr m_pVertexArray;
};

typedef boost::shared_ptr<Shape> ShapePtr;

}

#endif

