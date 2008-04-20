//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2006 Ulrich von Zadow
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

#include "Node.h"

#include "../base/Point.h"

#include <string>

namespace avg {
    
class DivNode : public Node
{
    public:
        static NodeDefinition getNodeDefinition();
        
        DivNode (const ArgList& Args, Player * pPlayer);
        virtual ~DivNode ();
        virtual void setRenderingEngines(DisplayEngine * pDisplayEngine, AudioEngine * pAudioEngine);
        virtual void disconnect();

        const std::string& getMediaDir() const;
        void setMediaDir(const std::string& mediaDir);

        int getNumChildren();
        NodePtr getChild(unsigned i);
        void appendChild(NodePtr pNewNode);
        void insertChild(NodePtr pNewNode, unsigned i);
        void removeChild(unsigned i);
        void reorderChild(unsigned i, unsigned j);
        int indexOf(NodePtr pChild);

        virtual NodePtr getElementByPos (const DPoint & pos);
        virtual void render (const DRect& rect);
        virtual std::string getTypeStr ();
        std::string getEffectiveMediaDir();
        virtual void checkReload();

        virtual std::string dump (int indent = 0);

    protected:
        virtual Point<double> getPreferredMediaSize();
    
    private:
        std::string m_sMediaDir;
        std::vector<NodePtr> m_Children;
};

}

#endif //_DivNode_H_
