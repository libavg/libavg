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

#ifndef _FBO_H_
#define _FBO_H_

#include "../api.h"

#include "GLTexture.h"
#include "PBO.h"
#include "VertexArray.h"

#include "../base/Point.h"

#include <boost/shared_ptr.hpp>

#include <vector>

namespace avg {

class AVG_API FBO
{
public:
    FBO(const IntPoint& size, PixelFormat pf, unsigned numTextures=1, 
            unsigned multisampleSamples=1, bool bUsePackedDepthStencil=false, 
            bool bMipmap=false);
    virtual ~FBO();

    void activate() const;
    PixelFormat getPF() const;
    unsigned getNumTextures() const;

    void copyToDestTexture() const;
    BitmapPtr getImage(int i=0, int mipmapLevel=0) const;
    void moveToPBO(int i=0, int mipmapLevel=0) const;
    BitmapPtr getImageFromPBO() const;
    GLTexturePtr getTex(int i=0) const;
    const IntPoint& getSize() const;

    static bool isFBOSupported();
    static bool isMultisampleFBOSupported();
    static bool isPackedDepthStencilSupported();

    void initCache();
    static void deleteCache();

private:
    void init();
    void checkError(const std::string& sContext) const;

    IntPoint m_Size;
    PixelFormat m_PF;
    unsigned m_MultisampleSamples;
    bool m_bUsePackedDepthStencil;
    bool m_bMipmap;

    PBOPtr m_pOutputPBO;
    unsigned m_FBO;
    std::vector<GLTexturePtr> m_pTextures;
    unsigned m_StencilBuffer;

    // Multisample support
    unsigned m_ColorBuffer;
    unsigned m_OutputFBO;

};

typedef boost::shared_ptr<FBO> FBOPtr;

}

#endif 
