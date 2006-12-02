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

#ifndef _ICamera_H_
#define _ICamera_H_

#include "../graphics/Bitmap.h"

#include <boost/shared_ptr.hpp>

#include <string>

namespace avg {

class ICamera
{
    public:
        virtual ~ICamera() {};
        virtual void open() = 0;
        virtual void close() = 0;

        virtual IntPoint getImgSize() = 0;
        virtual BitmapPtr getImage(bool bWait) = 0;
        virtual bool isCameraAvailable() = 0;

        virtual const std::string& getDevice() const = 0; 
        virtual double getFrameRate() const = 0;
        virtual const std::string& getMode() const = 0;
        
        virtual unsigned int getFeature(const std::string& sFeature) const = 0;
        virtual void setFeature(const std::string& sFeature, int Value) = 0;
};

typedef boost::shared_ptr<ICamera> CameraPtr;

}

#endif

