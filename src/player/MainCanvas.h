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

#ifndef _MainCanvas_H_
#define _MainCanvas_H_

#include "../api.h"
#include "Canvas.h"

namespace avg {

class DisplayEngine;
typedef boost::shared_ptr<DisplayEngine> DisplayEnginePtr;
class SecondaryGLXContext;
typedef boost::shared_ptr<SecondaryGLXContext> SecondaryGLXContextPtr;
class GLContextMultiplexer;
typedef boost::shared_ptr<GLContextMultiplexer> GLContextMultiplexerPtr;

class AVG_API MainCanvas: public Canvas
{
    public:
        MainCanvas(Player * pPlayer, bool bSecondViewport);
        virtual ~MainCanvas();
        virtual void setRoot(NodePtr pRootNode);
        virtual void initPlayback(const DisplayEnginePtr& pDisplayEngine);
       
        virtual BitmapPtr screenshot() const;

    private:
        void renderTree();
        void createSecondWindow();
        void pollEvents();

        DisplayEnginePtr m_pDisplayEngine;
        GLContextMultiplexerPtr m_pMultiplexer;
        bool m_bSecondViewport;

        SecondaryGLXContextPtr m_pGLContext;
};

}
#endif

