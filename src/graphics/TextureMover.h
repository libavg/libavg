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

#ifndef _TextureMover_H_
#define _TextureMover_H_

#include "../api.h"
#include "PixelFormat.h"
#include "OGLHelper.h"

#include "../base/GLMHelper.h"

#include <boost/shared_ptr.hpp>

namespace avg {

class Bitmap;
typedef boost::shared_ptr<Bitmap> BitmapPtr;
class GLTexture;
typedef boost::shared_ptr<GLTexture> GLTexturePtr;

class TextureMover;
typedef boost::shared_ptr<TextureMover> TextureMoverPtr;

class AVG_API TextureMover {
public:
    static TextureMoverPtr create(OGLMemoryMode memoryMode, IntPoint size, PixelFormat pf,
            unsigned usage);
    static TextureMoverPtr create(IntPoint size, PixelFormat pf, unsigned usage);

    TextureMover(const IntPoint& size, PixelFormat pf);
    virtual ~TextureMover();

    virtual void moveBmpToTexture(BitmapPtr pBmp, GLTexture& tex) = 0;

    virtual BitmapPtr lock() = 0;
    virtual void unlock() = 0;
    virtual void moveToTexture(GLTexture& tex) = 0;

    PixelFormat getPF() const;
    const IntPoint& getSize() const;

private:
    IntPoint m_Size;
    PixelFormat m_pf;
};

}

#endif
 

