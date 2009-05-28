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

#ifndef _VideoBase_H_
#define _VideoBase_H_

#include "../api.h"
#include "RasterNode.h"

#include "../base/Rect.h"

#include <string>


namespace avg {

class OGLTiledSurface;

class AVG_API VideoBase : public RasterNode
{
    public:
        static NodeDefinition createDefinition();
        
        virtual ~VideoBase ();
        void setRenderingEngines(DisplayEngine * pDisplayEngine, 
                AudioEngine * pAudioEngine);
        void disconnect();
        
        void play();
        void stop();
        void pause();
        virtual double getFPS() = 0;
        
        virtual void render (const DRect& Rect);
        virtual std::string dump (int indent = 0);
        
    protected:        
        VideoBase();
        enum VideoState {Unloaded, Paused, Playing};
        virtual VideoState getVideoState() const;
        void setFrameAvailable(bool bAvailable);
        virtual void changeVideoState(VideoState NewVideoState);
   
    private:
        void open();

        virtual bool renderToSurface(OGLTiledSurface * pSurface) = 0;
        virtual void open(bool bUseYCbCrShaders) = 0;
        virtual void close() = 0;
        virtual PixelFormat getPixelFormat() = 0;

        VideoState m_VideoState;

        bool m_bFrameAvailable;
        bool m_bFirstFrameDecoded;
};

}

#endif 

