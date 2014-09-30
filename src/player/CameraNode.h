//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2014 Ulrich von Zadow
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
#include "../imaging/CameraInfo.h"

#include <boost/thread/thread.hpp>

#include <string>
#include <map>

namespace avg {

class TextureMover;
typedef boost::shared_ptr<TextureMover> TextureMoverPtr;
typedef std::vector<CameraInfo> CamerasInfosVector;
class MCTexture;
typedef boost::shared_ptr<MCTexture> MCTexturePtr;

class AVG_API CameraNode : public RasterNode
{
    public:
        static void registerType();
        
        CameraNode(const ArgList& args);
        virtual ~CameraNode();

        virtual void connectDisplay();
        virtual void connect(CanvasPtr pCanvas);
        virtual void disconnect(bool bKill);
                
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

        float getFrameRate() const
        {
            return m_pCamera->getFrameRate();
        }

        int getBrightness() const;
        void setBrightness(int value);
        int getSharpness() const;
        void setSharpness(int value);
        int getSaturation() const;
        void setSaturation(int value);
        int getCamGamma() const;
        void setCamGamma(int value);
        int getShutter() const;
        void setShutter(int value);
        int getGain() const;
        void setGain(int value);
        int getWhitebalanceU() const;
        int getWhitebalanceV() const;
        void setWhitebalance(int u, int v);
        void doOneShotWhitebalance();
        int getStrobeDuration() const;
        void setStrobeDuration(int value);
        
        void updateCameraImage();
        void setAutoUpdateCameraImage(bool bVal);
        bool isImageAvailable() const;

        virtual void preRender(const VertexArrayPtr& pVA, bool bIsParentActive, 
                float parentEffectiveOpacity);
        virtual void render();

        int getFrameNum() const;
        IntPoint getMediaSize();
        virtual BitmapPtr getBitmap();

        static CamerasInfosVector getCamerasInfos();
        static void resetFirewireBus();

    private:
        int getFeature (CameraFeature feature) const;
        void setFeature (CameraFeature feature, int value);

        virtual float getFPS() const;
        virtual void open();
        virtual PixelFormat getPixelFormat();
        void setFeature(int FeatureID);

        void updateToLatestCameraImage();

        bool m_bIsPlaying;
    
        CameraPtr m_pCamera;
        int m_FrameNum;
        BitmapPtr m_pCurBmp;
        bool m_bAutoUpdateCameraImage;
        bool m_bNewBmp;
        bool m_bNewSurface;

        MCTexturePtr m_pTex;
};

}

#endif

