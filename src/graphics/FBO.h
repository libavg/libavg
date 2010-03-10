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

#ifndef _FBO_H_
#define _FBO_H_

#include "../api.h"

#include "../base/Point.h"
#include "../graphics/PBOImage.h"

#include <boost/shared_ptr.hpp>
#include <vector>

namespace avg {

class AVG_API FBO
{
public:
    FBO(const IntPoint& size, PixelFormat pf, unsigned texID, 
            unsigned multisampleSamples=1);
    FBO(const IntPoint& size, PixelFormat pf, std::vector<unsigned> texIDs);
    virtual ~FBO();

    void activate() const;
    void deactivate() const;

    BitmapPtr getImage(int i) const;
    unsigned getTexture() const;

    static bool isFBOSupported();
    static bool isMultisampleFBOSupported();

private:
    void init();
    void checkError() const;

    IntPoint m_Size;
    PixelFormat m_PF;
    unsigned m_MultisampleSamples;
    PBOImagePtr m_pOutputPBO;
    unsigned m_FBO;
    std::vector<unsigned> m_TexIDs;

    // Multisample support
    unsigned m_ColorBuffer;
    unsigned m_OutputFBO;
};

typedef boost::shared_ptr<FBO> FBOPtr;

}


#endif 

