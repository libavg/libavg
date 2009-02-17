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

#ifndef _PanoImage_H_
#define _PanoImage_H_

#include "../api.h"
#include "../avgconfigwrapper.h"

#include "AreaNode.h"
#include "../graphics/Bitmap.h"

#include <string>
#include <vector>

namespace avg {
    
class SDLDisplayEngine;

class AVG_API PanoImage: public AreaNode
{
    public:
        static NodeDefinition createDefinition();
        
        PanoImage(const ArgList& Args, bool bFromXML);
        virtual ~PanoImage();
        
        virtual void setRenderingEngines(DisplayEngine * pDisplayEngine, 
                AudioEngine * pAudioEngine);
        virtual void disconnect();
        virtual void render(const DRect& Rect);

        double getScreenPosFromPanoPos(int PanoPos) const;
        double getScreenPosFromAngle(double Angle) const;
        const std::string& getHRef() const;
        void setHRef(const std::string& href);
        double getSensorWidth() const;
        void setSensorWidth(double sensorWidth);
        double getSensorHeight() const;
        void setSensorHeight(double sensorHeight);
        double getFocalLength() const;
        void setFocalLength(double focalLength);
        double getRotation() const;
        void setRotation(double rotation);
        double getMaxRotation() const;

    protected:        
        virtual DPoint getPreferredMediaSize();

    private:
        void load();
        void calcProjection();
        void setupTextures();
        void clearTextures();

        std::string m_href;
        std::string m_Filename;
        double m_SensorWidth;
        double m_SensorHeight;
        double m_FocalLength;
        BitmapPtr m_pBmp;
        int m_TexHeight;
        std::vector<unsigned int> m_TileTextureIDs;

        // Derived values calculated in calcProjection
        double m_fovy;         // Vertical field of view 
        double m_aspect;       // Sensor aspect ratio
        double m_CylHeight;
        double m_CylAngle;     // Total angle covered by panorama
        double m_SliceAngle;   // Angle per slice
        double m_MaxRotation;  // Maximum rotation angle (=Total angle - visible area)

        double m_Rotation;
};

}

#endif //_PanoImage_H_

