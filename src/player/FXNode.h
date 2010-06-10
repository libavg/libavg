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

#ifndef _FXNode_H_
#define _FXNode_H_

#include "../api.h"

#include "../graphics/FBO.h"
#include "../base/IPlaybackEndListener.h"

#include <boost/shared_ptr.hpp>

namespace avg {

class SDLDisplayEngine;

class AVG_API FXNode: public IPlaybackEndListener {
public:
    FXNode();
    virtual ~FXNode();

    virtual void connect(SDLDisplayEngine* pEngine);
    virtual void disconnect();
    virtual void setSize(const IntPoint& newSize);

    virtual void apply(GLTexturePtr pSrcTex)=0;

    GLTexturePtr getTex();
    BitmapPtr getImage();

    virtual void onPlaybackEnd();

protected:
    SDLDisplayEngine* getEngine() const;
    FBOPtr getFBO();
    const std::string& getStdShaderCode() const;

private:
    virtual void destroyShader()=0;

    SDLDisplayEngine* m_pEngine;
    IntPoint m_Size;
    FBOPtr m_pFBO;
};

typedef boost::shared_ptr<FXNode> FXNodePtr;

}

#endif

