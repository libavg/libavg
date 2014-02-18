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

#ifndef _FBOInfo_H_
#define _FBOInfo_H_

#include "../api.h"

#include "PixelFormat.h"

#include "../base/GLMHelper.h"

#include <boost/shared_ptr.hpp>

#include <vector>

namespace avg {

class AVG_API FBOInfo
{
public:
    FBOInfo(const IntPoint& size, PixelFormat pf, unsigned numTextures, 
            unsigned multisampleSamples, bool bUsePackedDepthStencil,
            bool bUseStencil, bool bMipmap);
    virtual ~FBOInfo();

    const IntPoint& getSize() const;

    static bool isFBOSupported();
    static bool isMultisampleFBOSupported();
    static bool isPackedDepthStencilSupported();

protected:
    PixelFormat getPF() const;
    unsigned getMultisampleSamples() const;
    bool getUsePackedDepthStencil() const;
    bool getUseStencil() const;
    bool getMipmap() const;

    void throwMultisampleError();

private:
    IntPoint m_Size;
    PixelFormat m_PF;
    unsigned m_MultisampleSamples;
    bool m_bUsePackedDepthStencil;
    bool m_bUseStencil;
    bool m_bMipmap;
};

}

#endif 
