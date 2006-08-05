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

#ifndef _PanoImage_H_
#define _PanoImage_H_

#include "../avgconfig.h"
#undef PACKAGE
#undef PACKAGE_BUGREPORT
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION

#include "Node.h"
#ifdef AVG_ENABLE_GL
#include "OGLSurface.h"
#endif
#include "../graphics/Bitmap.h"

#include <string>
#include <vector>

namespace avg {
    
class SDLDisplayEngine;

class PanoImage : public Node
{
	public:
        PanoImage ();
        PanoImage (const xmlNodePtr xmlNode, DivNode * pParent);
        virtual ~PanoImage ();
        
        virtual void init (DisplayEngine * pEngine, 
                DivNode * pParent, Player * pPlayer);
        virtual void render (const DRect& Rect);
        virtual bool obscures (const DRect& Rect, int Child);
        virtual std::string getTypeStr ();

        const std::string& getFilename () const;
        double getSensorWidth () const;
        double getSensorHeight () const;
        double getFocalLength () const;
        void setFocalLength (double focalLength);
        int getHue () const;
        int getSaturation () const;
        double getRotation () const;
        void setRotation (double rotation);
        double getMaxRotation () const;

    protected:        
        virtual DPoint getPreferredMediaSize();

    private:
        void calcProjection();
        void setupTextures();

        SDLDisplayEngine * getSDLEngine();
    
        std::string m_Filename;
        double m_SensorWidth;
        double m_SensorHeight;
        double m_FocalLength;
        BitmapPtr m_pBmp;
        std::vector<unsigned int> m_TileTextureIDs;
        SDLDisplayEngine * m_pEngine;

        // Derived values calculated in calcProjection
        double m_fovy;
        double m_aspect;
        double m_CylHeight;
        double m_CylAngle;
        double m_SliceAngle;
        double m_MaxRotation;

        double m_Rotation;

        int m_Hue;
        int m_Saturation;
};

}

#endif //_PanoImage_H_

