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

#ifndef _Camera_H_
#define _Camera_H_

#include "../avgconfig.h"
#undef PACKAGE_BUGREPORT
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION

#include "VideoBase.h"
#include "../graphics/Rect.h"
#include "../graphics/Pixel24.h"

#ifdef AVG_ENABLE_1394
#include <libraw1394/raw1394.h>
#include <libdc1394/dc1394_control.h>
#endif
#ifdef AVG_ENABLE_1394_2
#include <dc1394/control.h>
#endif

#include <string>
#include <map>

namespace avg {

class Camera : public VideoBase
{
    public:
        Camera ();
        Camera (const xmlNodePtr xmlNode, Player * pPlayer);
        virtual ~Camera ();

        virtual void setDisplayEngine(DisplayEngine * pEngine);
        virtual std::string getTypeStr ();

        const std::string& getDevice() const 
        {
            return m_sDevice;
        }

        double getFrameRate() const
        {
            return m_FrameRate;
        }

        const std::string& getMode() const
        {
            return m_sMode;
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

        std::string m_sDevice;
        double m_FrameRate;
        std::string m_sMode;
        std::map<int, int> m_Features;

#ifdef AVG_ENABLE_1394
        bool findCameraOnPort(int port, raw1394handle_t& FWHandle);
        void checkDC1394Error(int Code, const std::string & sMsg);

        dc1394_cameracapture m_Camera;
        raw1394handle_t m_FWHandle;
        dc1394_feature_set m_FeatureSet;
        int m_FrameRateConstant;  // libdc1394 constant for framerate.
        int m_Mode;               // libdc1394 constant for mode.
#endif
#ifdef AVG_ENABLE_1394_2
        dc1394camera_t * m_pCamera;
        dc1394featureset_t m_FeatureSet;
        dc1394framerate_t m_FrameRateConstant; 
        dc1394video_mode_t m_Mode;            
#endif
#if defined(AVG_ENABLE_1394) || defined(AVG_ENABLE_1394_2)
        void fatalError(const std::string & sMsg);
        void dumpCameraInfo();
        int getFeatureID(const std::string& sFeature) const;

        long long m_LastFrameTime;
        bool m_bCameraAvailable;
#endif


};

}

#endif

