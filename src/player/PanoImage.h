//
// $Id$
// 

#ifndef _PanoImage_H_
#define _PanoImage_H_

#include "../avgconfig.h"
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
        PanoImage (const xmlNodePtr xmlNode, Container * pParent);
        virtual ~PanoImage ();
        
        virtual void init (IDisplayEngine * pEngine, 
                Container * pParent, Player * pPlayer);
        virtual void render (const DRect& Rect);
        virtual bool obscures (const DRect& Rect, int z);
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

