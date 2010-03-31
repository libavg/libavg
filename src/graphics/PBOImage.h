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

#ifndef _PBOImage_H_
#define _PBOImage_H_

#include "../api.h"
#include "Bitmap.h"
#include "OGLHelper.h"

#include "../base/Point.h"

#include <boost/shared_ptr.hpp>

namespace avg {

class VertexArray;

class AVG_API PBOImage {

public:
    PBOImage(const IntPoint& size, PixelFormat pfInternal, PixelFormat pfExternal,
            bool bUseInputPBO, bool bUseOutputPBO, bool bMipmap = false);
    virtual ~PBOImage();

    static bool isFloatFormatSupported();

    void setImage(BitmapPtr pBmp);
    void setImage(float * pData);
    virtual BitmapPtr getImage() const;
    void activateTex(int textureUnit);
    void draw();

    PixelFormat getIntPF() const;
    PixelFormat getExtPF() const;
    const IntPoint& getSize() const;
    unsigned getTexID() const;
    unsigned getOutputPBO() const;
    int getFormat(PixelFormat pf) const;
    int getType(PixelFormat pf) const;

private:
    int getInternalFormat() const;
    unsigned createInputPBO() const;
    unsigned createOutputPBO() const;
    void deletePBO(unsigned* pPBO);
    void getPixelStride();

    PixelFormat m_pfInt, m_pfExt;
    IntPoint m_Size;
    int m_Precision;
    bool m_bUseInputPBO;
    bool m_bUseOutputPBO;
    unsigned m_InputPBO;
    unsigned m_OutputPBO;
    unsigned m_TexID;
    VertexArray * m_pVertexes;
};

typedef boost::shared_ptr<PBOImage> PBOImagePtr;

}

#endif
 

