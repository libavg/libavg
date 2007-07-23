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

#ifndef _V4LCamera_H_
#define _V4LCamera_H_

#include "../avgconfig.h"
#undef PACKAGE_BUGREPORT
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION

#include "ICamera.h"
#include <string>
#include <vector>

namespace avg {

typedef int V4LCID_t;

class V4LCamera: public ICamera {

    struct Buffer {
        void * start;
        size_t length;
    };
    
    public:
        V4LCamera(std::string sDevice, int Channel, IntPoint Size,
            const std::string &PixelFormat, bool bColor);
        virtual ~V4LCamera();
        virtual void open();
        virtual void close();

        virtual IntPoint getImgSize();
        virtual BitmapPtr getImage(bool bWait);
        virtual bool isCameraAvailable();

        virtual const std::string& getDevice() const; 
        virtual double getFrameRate() const;
        virtual const std::string& getMode() const;
        
        virtual unsigned int getFeature(const std::string& sFeature) const;
        virtual void setFeature(const std::string& sFeature, int Value);
        
    private:
        int m_Fd;
        int m_Channel;
        std::string m_sDevice;
        std::string m_sMode;
        std::vector<Buffer> m_vBuffers;
        bool m_bCameraAvailable;
        int m_CamPF;
        bool m_bColor;
        IntPoint m_ImgSize;
        
        int getCamPF(const std::string& sPF);
        
        void initDevice();
        void startCapture();
        void initMMap();
        
        void setFeature(V4LCID_t V4LFeature, int Value);
        V4LCID_t getFeatureID(const std::string& sFeature) const;
        std::string getFeatureName(V4LCID_t V4LFeature);
        bool isFeatureSupported(V4LCID_t V4LFeature) const;
        typedef std::map<V4LCID_t, unsigned int> FeatureMap;
        typedef std::map<int, std::string> FeatureNamesMap;
        FeatureMap m_Features;
        FeatureNamesMap m_FeaturesNames;
};

}

#endif
