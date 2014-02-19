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

#ifndef _MCFBO_H_
#define _MCFBO_H_

#include "../api.h"

#include "FBOInfo.h"
#include "FBO.h"
#include "MCTexture.h"

#include "../base/GLMHelper.h"

#include <boost/shared_ptr.hpp>

#include <vector>
#include <map>

namespace avg {

class AVG_API MCFBO: public FBOInfo
{
public:
    MCFBO(const IntPoint& size, PixelFormat pf, unsigned numTextures=1, 
            unsigned multisampleSamples=1, bool bUsePackedDepthStencil=false,
            bool bUseStencil=false, bool bMipmap=false);
    virtual ~MCFBO();
    void initForGLContext();

    void activate() const;
    FBOPtr getCurFBO() const;

    void copyToDestTexture() const;
    BitmapPtr getImage(int i=0) const;
    void moveToPBO(int i=0) const;
    BitmapPtr getImageFromPBO() const;
    MCTexturePtr getTex(int i=0) const;

private:

    typedef std::map<GLContext*, FBOPtr> FBOMap;
    FBOMap m_pFBOs;
    std::vector<MCTexturePtr> m_pTextures;
    
};

typedef boost::shared_ptr<MCFBO> MCFBOPtr;

}

#endif 
