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

#ifndef _MCTexture_H_
#define _MCTexture_H_

#include "../api.h"
#include "TexInfo.h"
#include "OGLHelper.h"

#include <boost/shared_ptr.hpp>
#ifdef _WIN32 
#include <unordered_map>
#elif defined __APPLE__
#include <boost/unordered_map.hpp>
#else
#include <tr1/unordered_map>
#endif

namespace avg {

class GLContext;
class GLTexture;
typedef boost::shared_ptr<GLTexture> GLTexturePtr;
class Bitmap;
typedef boost::shared_ptr<Bitmap> BitmapPtr;

class AVG_API MCTexture: public TexInfo {

public:
    MCTexture(const IntPoint& size, PixelFormat pf, bool bMipmap=false,
            bool bForcePOT=false, int potBorderColor=0);
    virtual ~MCTexture();

    void initForGLContext(GLContext* pContext);

    void moveBmpToTexture(GLContext* pContext, BitmapPtr pBmp);

    const GLTexturePtr& getTex(GLContext* pContext) const;

    void setDirty();
    bool isDirty() const;
    void resetDirty();

private:
#ifdef __APPLE__
    typedef boost::unordered_map<GLContext*, GLTexturePtr> TexMap;
#else
    typedef std::tr1::unordered_map<GLContext*, GLTexturePtr> TexMap;
#endif
    TexMap m_pTextures;

    bool m_bIsDirty;
};

typedef boost::shared_ptr<MCTexture> MCTexturePtr;

}

#endif

