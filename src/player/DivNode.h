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

#ifndef _DivNode_H_
#define _DivNode_H_

#include "../api.h"
#include "AreaNode.h"

#include "../base/UTF8String.h"
#include "../graphics/VertexArray.h"

#include <string>

namespace avg {
    
class AVG_API DivNode : public AreaNode
{
    public:
        static NodeDefinition createDefinition();
        
        DivNode(const ArgList& Args);
        virtual ~DivNode();
        virtual void setRenderingEngines(DisplayEngine * pDisplayEngine, 
                AudioEngine * pAudioEngine);
        virtual void connect(Scene * pScene);
        virtual void disconnect(bool bKill);

        VisibleNodePtr getVChild(unsigned i);
        virtual void insertChild(NodePtr pNewNode, unsigned i);
        void removeChild(NodePtr pNode);
        void removeChild(unsigned i);
        void removeChild(NodePtr pNode, bool bKill);
        void removeChild(unsigned i, bool bKill);

        bool getCrop() const;
        void setCrop(bool bCrop);

        const std::string& getElementOutlineColor() const;
        void setElementOutlineColor(const std::string& sColor);

        const UTF8String& getMediaDir() const;
        void setMediaDir(const UTF8String& mediaDir);

        virtual VisibleNodePtr getElementByPos(const DPoint & pos);
        virtual void preRender();
        virtual void render(const DRect& rect);
        virtual void renderOutlines(VertexArrayPtr pVA, Pixel32 color);

        virtual std::string getEffectiveMediaDir();
        virtual void checkReload();

        virtual std::string dump(int indent = 0);
        IntPoint getMediaSize();
   
    private:
        UTF8String m_sMediaDir;
        bool m_bCrop;
        std::string m_sElementOutlineColor;
        Pixel32 m_ElementOutlineColor;
};

}

#endif //_DivNode_H_
