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

#ifndef _ImageNode_H_
#define _ImageNode_H_

#include "../api.h"
#include "RasterNode.h"
#include "GPUImage.h"
#include "../graphics/TexInfo.h"

#include "../base/UTF8String.h"

#include <string>

namespace avg {

class Bitmap;
typedef boost::shared_ptr<Bitmap> BitmapPtr;

class AVG_API ImageNode : public RasterNode
{
    public:
        static void registerType();
        
        ImageNode(const ArgList& args, const std::string& sPublisherName="Node");
        virtual ~ImageNode();
        virtual void connectDisplay();
        virtual void connect(CanvasPtr pCanvas);
        virtual void disconnect(bool bKill);
        virtual void checkReload();

        const UTF8String& getHRef() const;
        void setHRef(const UTF8String& href);
        const std::string getCompression() const;
        void setBitmap(BitmapPtr pBmp);
        
        virtual void preRender(const VertexArrayPtr& pVA, bool bIsParentActive, 
                float parentEffectiveOpacity);
        virtual void render(GLContext* pContext, const glm::mat4& transform);
        
        void getElementsByPos(const glm::vec2& pos, NodeChainPtr& pElements);
        glm::vec2 toCanvasPos(const glm::vec2& pos);

        virtual BitmapPtr getBitmap();
        virtual IntPoint getMediaSize();
        GPUImage::Source getSource() const;

        virtual std::string dump(int indent = 0);

    private:
        bool isCanvasURL(const std::string& sURL);
        void checkCanvasValid(const CanvasPtr& pCanvas);

        UTF8String m_href;
        TexCompression m_Compression;
        GPUImagePtr m_pGPUImage;
};

typedef boost::shared_ptr<ImageNode> ImageNodePtr;

}

#endif

