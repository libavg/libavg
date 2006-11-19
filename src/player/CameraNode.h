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

#ifndef _CameraNode_H_
#define _CameraNode_H_

#include "../avgconfig.h"
#undef PACKAGE_BUGREPORT
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION

#include "VideoBase.h"

#include "../imaging/Camera.h"

#include <boost/thread/thread.hpp>

#include <string>
#include <map>

namespace avg {

class CameraNode : public VideoBase
{
    public:
        CameraNode();
        CameraNode(const xmlNodePtr xmlNode, Player * pPlayer);
        virtual ~CameraNode();

        virtual void setDisplayEngine(DisplayEngine * pEngine);
        virtual std::string getTypeStr();

        const std::string& getDevice() const 
        {
            return m_pCamera->getDevice();
        }

        double getFrameRate() const
        {
            return m_pCamera->getFrameRate();
        }

        const std::string& getMode() const
        {
            return m_pCamera->getMode();
        }

        int getBrightness() const
        {
            return getFeature ("brightness");
        }

        void setBrightness(int Value)
        {
            setFeature ("brightness", Value);
        }
        
        int getExposure() const
        {
            return getFeature ("exposure");
        }

        void setExposure(int Value)
        {
            setFeature ("exposure", Value);
        }
        
        int getSharpness() const
        {
            return getFeature ("sharpness");
        }
        
        unsigned int getWhiteBalance() const
        {
            return getFeature ("whitebalance");
        }

        void setSharpness(int Value)
        {
            setFeature ("sharpness", Value);
        }
        
        int getSaturation() const
        {
            return getFeature ("saturation");
        }

        void setSaturation(int Value)
        {
            setFeature ("saturation", Value);
        }
        
        int getGamma() const
        {
            return getFeature ("gamma");
        }

        void setGamma(int Value)
        {
            setFeature ("gamma", Value);
        }
        
        int getShutter() const
        {
            return getFeature ("shutter");
        }

        void setShutter(int Value)
        {
            setFeature ("shutter", Value);
        }
        
        int getGain() const
        {
            return getFeature ("gain");
        }

        void setGain(int Value)
        {
            setFeature ("gain", Value);
        }
        
        void setWhiteBalance(int Value)
        {
            setFeature ("whitebalance", Value);
        }
            

        unsigned int getFeature (const std::string& sFeature) const;
        void setFeature (const std::string& sFeature, int Value);

    private:
        virtual bool renderToSurface(ISurface * pSurface);
        virtual bool canRenderToBackbuffer(int BitsPerPixel);
        virtual double getFPS();
        virtual void open(int* pWidth, int* pHeight);
        virtual void close();
        virtual PixelFormat getDesiredPixelFormat();
        void setFeature(int FeatureID);
        IntPoint getNativeSize();

        CameraPtr m_pCamera;
};

}

#endif

