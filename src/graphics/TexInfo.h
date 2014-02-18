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

#ifndef _TexInfo_H_
#define _TexInfo_H_

#include "../api.h"
#include "Bitmap.h"
#include "OGLHelper.h"

#include <boost/shared_ptr.hpp>

namespace avg {

class AVG_API TexInfo {

public:
    TexInfo(const IntPoint& size, PixelFormat pf, bool bMipmap,
            unsigned wrapSMode, unsigned wrapTMode, bool bUsePOT, int potBorderColor);
    virtual ~TexInfo();

    virtual void setWrapMode(unsigned wrapSMode, unsigned wrapTMode);

    const IntPoint& getSize() const;
    const IntPoint& getGLSize() const;
    PixelFormat getPF() const;

    IntPoint getMipmapSize(int level) const;

    static bool isFloatFormatSupported();
    static int getGLFormat(PixelFormat pf);
    static int getGLType(PixelFormat pf);
    int getGLInternalFormat() const;
    
    void dump(unsigned wrapSMode=-1, unsigned wrapTMode=-1) const;

protected:
    unsigned getWrapSMode() const;
    unsigned getWrapTMode() const;
    bool getUseMipmap() const;
    bool getUsePOT() const;
    int getPOTBorderColor() const;

    static bool usePOT(bool bForcePOT, bool bMipmap);

private:
    IntPoint m_Size;
    IntPoint m_GLSize;
    PixelFormat m_pf;
    bool m_bMipmap;

    unsigned m_WrapSMode;
    unsigned m_WrapTMode;
    
    bool m_bUsePOT;
    int m_POTBorderColor;
};

typedef boost::shared_ptr<TexInfo> TexInfoPtr;

}

#endif
 


