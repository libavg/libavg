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

#include <string>
#include <map>

namespace avg {

class Camera : public VideoBase
{
    public:
        Camera ();
        Camera (const xmlNodePtr xmlNode, Container * pParent);
        virtual ~Camera ();

        virtual void init (DisplayEngine * pEngine, Container * pParent,
            Player * pPlayer);
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
        
        int getWhiteBalance() const
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
        bool isYCbCrSupported();

    private:
        virtual bool renderToSurface(ISurface * pSurface);
        virtual bool canRenderToBackbuffer(int BitsPerPixel);
        virtual double getFPS();
        virtual void open(int* pWidth, int* pHeight);
        virtual void close();
        void setFeature(int FeatureID);

        std::string m_sDevice;
        double m_FrameRate;
        int m_FrameRateConstant;  // libdc1394 constant for framerate.
        std::string m_sMode;
        int m_Mode;               // libdc1394 constant for mode.
        std::map<int, int> m_Features;

#ifdef AVG_ENABLE_1394
        void YUV411toBGR32Line(unsigned char * pSrc, int y, Pixel32 * pDestLine);
        void YUV411toBGR32(unsigned char * pSrc, BitmapPtr pBmp);

        bool findCameraOnPort(int port, raw1394handle_t& FWHandle);

        void checkDC1394Error(int Code, const std::string & sMsg);
        void fatalError(const std::string & sMsg);
        void dumpCameraInfo();
        int getFeatureID(const std::string& sFeature) const;

        dc1394_cameracapture m_Camera;
        raw1394handle_t m_FWHandle;
        dc1394_feature_set m_FeatureSet;
        long long m_LastFrameTime;
        bool m_bCameraAvailable;

        static void initCameraSupport();
        static void initYUV2RGBConversionMatrix();
        static bool m_bInitialized;
#endif

};

}

#endif

