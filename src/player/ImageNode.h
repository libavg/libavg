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

#ifndef _ImageNode_H_
#define _ImageNode_H_

#include "../api.h"
#include "RasterNode.h"
#include "Image.h"

#include "../graphics/Bitmap.h"
#include "../base/UTF8String.h"

#include <string>

namespace avg {

class AVG_API ImageNode : public RasterNode
{
    public:
        static NodeDefinition createDefinition();
        
        ImageNode(const ArgList& Args);
        virtual ~ImageNode();
        virtual void setRenderingEngines(DisplayEngine * pDisplayEngine, 
                AudioEngine * pAudioEngine);
        virtual void connect();
        virtual void disconnect(bool bKill);
        virtual void checkReload();

        const UTF8String& getHRef() const;
        void setHRef(const UTF8String& href);
        void setBitmap(const Bitmap * pBmp);
        
        virtual void render(const DRect& Rect);
        
        virtual BitmapPtr getBitmap();
        virtual IntPoint getMediaSize();

    private:
        UTF8String m_href;
        ImagePtr m_pImage;
};

}

#endif

