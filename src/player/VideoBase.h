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

#ifndef _VideoBase_H_
#define _VideoBase_H_

#include "RasterNode.h"
#include "../graphics/Rect.h"

#include <string>

class ISurface;

namespace avg {

class VideoBase : public RasterNode
{
    public:
        virtual ~VideoBase ();
        
        virtual void init (DisplayEngine * pEngine, DivNode * pParent, 
                Player * pPlayer);

        void play();
        void stop();
        void pause();
        virtual double getFPS() = 0;
        
        virtual void prepareRender (int time, const DRect& parent);
        virtual void render (const DRect& Rect);
        bool obscures (const DRect& Rect, int Child);
        virtual std::string dump (int indent = 0);
        
    protected:        
        VideoBase ();
        VideoBase (const xmlNodePtr xmlNode, DivNode * pParent);
        virtual DPoint getPreferredMediaSize();
        typedef enum VideoState {Unloaded, Paused, Playing};
        virtual VideoState getState() const;
        void setFrameAvailable(bool bAvailable);
        void changeState(VideoState NewState);
        int getMediaWidth();
        int getMediaHeight();
        DisplayEngine::YCbCrMode getYCbCrMode();
   
    private:
        void renderToBackbuffer();
        void open();

        virtual bool renderToSurface(ISurface * pSurface) = 0;
        virtual bool canRenderToBackbuffer(int BitsPerPixel) = 0;
        virtual void open(int* pWidth, int* pHeight) = 0;
        virtual void close() = 0;
        virtual PixelFormat getDesiredPixelFormat() = 0;

        VideoState m_State;
       
        int m_Width;
        int m_Height;
        DisplayEngine::YCbCrMode m_YCbCrMode;

        bool m_bFrameAvailable;
};

}

#endif 

