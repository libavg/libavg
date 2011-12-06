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

#ifndef _FakeCamera_H_
#define _FakeCamera_H_

#include "../api.h"
#include "Camera.h"

#include <boost/shared_ptr.hpp>

#include <string>
#include <queue>

namespace avg {

typedef boost::shared_ptr<std::queue<BitmapPtr> > BitmapQueuePtr;

class AVG_API FakeCamera: public Camera
{
public:
    FakeCamera(PixelFormat camPF, PixelFormat destPF);
    FakeCamera(std::vector<std::string>& pictures);
    virtual ~FakeCamera();
    virtual void open();
    virtual void close();

    virtual BitmapPtr getImage(bool bWait);
    virtual bool isCameraAvailable();

    virtual const std::string& getDevice() const; 
    virtual const std::string& getDriverName() const; 
    virtual const std::string& getMode() const;

    virtual int getFeature(CameraFeature feature) const;
    virtual void setFeature(CameraFeature feature, int Value, bool bIgnoreOldValue=false);
    virtual void setFeatureOneShot(CameraFeature feature);
    virtual int getWhitebalanceU() const;
    virtual int getWhitebalanceV() const;
    virtual void setWhitebalance(int u, int v, bool bIgnoreOldValue=false);

private:
    BitmapQueuePtr m_pBmpQ;
    bool m_bIsOpen;
};

}

#endif


