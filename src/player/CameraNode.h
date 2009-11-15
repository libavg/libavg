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

#ifndef _CameraNode_H_
#define _CameraNode_H_

#include "../api.h"
#include "../avgconfigwrapper.h"

#include "RasterNode.h"

#include "../imaging/Camera.h"

#include <boost/thread/thread.hpp>

#include <string>
#include <map>

namespace avg {

class AVG_API CameraNode : public RasterNode
{
    public:
        static NodeDefinition createDefinition();
        
        CameraNode(const ArgList& Args);
        virtual ~CameraNode();

        virtual void setRenderingEngines(DisplayEngine * pDisplayEngine,
                AudioEngine * pAudioEngine);
                
        void play();
        void stop();
        bool isAvailable();

        const std::string& getDevice() const 
        {
            return m_pCamera->getDevice();
        }

        const std::string& getDriverName() const
        {
            return m_pCamera->getDriverName();
        }

        double getFrameRate() const
        {
            return m_pCamera->getFrameRate();
        }

        int getBrightness() const;
        void setBrightness(int Value);
        int getSharpness() const;
        void setSharpness(int Value);
        int getSaturation() const;
        void setSaturation(int Value);
        int getGamma() const;
        void setGamma(int Value);
        int getShutter() const;
        void setShutter(int Value);
        int getGain() const;
        void setGain(int Value);
        int getWhitebalanceU() const;
        int getWhitebalanceV() const;
        void setWhitebalance(int u, int v);
        int getStrobeDuration() const;
        void setStrobeDuration(int Value);
        
        virtual void preRender();
        virtual void render(const DRect& Rect);

        int getFrameNum() const;
        IntPoint getMediaSize();

        static void dumpCameras();
        static void resetFirewireBus();

    private:
        int getFeature (CameraFeature Feature) const;
        void setFeature (CameraFeature Feature, int Value);

        virtual double getFPS() const;
        virtual void open();
        virtual PixelFormat getPixelFormat();
        void setFeature(int FeatureID);

        bool m_bIsPlaying;
    
        CameraPtr m_pCamera;
        int m_FrameNum;
        BitmapPtr m_pCurBmp;
};

}

#endif

