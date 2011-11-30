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

#ifndef _PanoImageNode_H_
#define _PanoImageNode_H_

#include "../api.h"
#include "../avgconfigwrapper.h"

#include "AreaNode.h"
#include "../base/UTF8String.h"
#include "../graphics/Bitmap.h"

#include <string>
#include <vector>

namespace avg {
    
class AVG_API PanoImageNode: public AreaNode
{
    public:
        static NodeDefinition createDefinition();
        
        PanoImageNode(const ArgList& Args);
        virtual ~PanoImageNode();
        
        virtual void connectDisplay();
        virtual void disconnect(bool bKill);
        virtual void render(const FRect& Rect);

        float getScreenPosFromPanoPos(int PanoPos) const;
        float getScreenPosFromAngle(float Angle) const;
        const UTF8String& getHRef() const;
        void setHRef(const UTF8String& href);
        float getSensorWidth() const;
        void setSensorWidth(float sensorWidth);
        float getSensorHeight() const;
        void setSensorHeight(float sensorHeight);
        float getFocalLength() const;
        void setFocalLength(float focalLength);
        float getRotation() const;
        void setRotation(float rotation);
        float getMaxRotation() const;

    protected:        
        virtual glm::vec2 getPreferredMediaSize();

    private:
        void load();
        void calcProjection();
        void setupTextures();
        void clearTextures();

        UTF8String m_href;
        std::string m_Filename;
        float m_SensorWidth;
        float m_SensorHeight;
        float m_FocalLength;
        BitmapPtr m_pBmp;
        int m_TexHeight;
        std::vector<unsigned int> m_TileTextureIDs;

        // Derived values calculated in calcProjection
        float m_fovy;         // Vertical field of view 
        float m_aspect;       // Sensor aspect ratio
        float m_CylHeight;
        float m_CylAngle;     // Total angle covered by panorama
        float m_SliceAngle;   // Angle per slice
        float m_MaxRotation;  // Maximum rotation angle (=Total angle - visible area)

        float m_Rotation;
};

}

#endif //_PanoImageNode_H_

