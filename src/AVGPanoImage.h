//
// $Id$
// 

#ifndef _AVGPANOIMAGE_H_
#define _AVGPANOIMAGE_H_

#include "AVGNode.h"
#include "AVGOGLSurface.h"

#include <paintlib/planybmp.h>

#include <string>
#include <vector>

//f7b9b25d-608b-4ed5-996a-3e210919ae16
#define AVGPANOIMAGE_CID \
{ 0xf7b9b25d, 0x608b, 0x4ed5, { 0x99, 0x6a, 0x3e, 0x21, 0x09, 0x19, 0xae, 0x16 } }

#define AVGPANOIMAGE_CONTRACTID "@c-base.org/avgpanoimage;1"

class AVGSDLDisplayEngine;

class AVGPanoImage : public AVGNode
{
	public:
        NS_DECL_ISUPPORTS

        static AVGPanoImage * create();

        AVGPanoImage ();
        virtual ~AVGPanoImage ();
        
        NS_IMETHOD GetType(PRInt32 *_retval);

        virtual void init (const std::string& id, const std::string& filename,
                double SensorWidth, double SensorHeight, double FocalLength, 
                IAVGDisplayEngine * pEngine, AVGContainer * pParent, 
                AVGPlayer * pPlayer);
        virtual void render (const AVGDRect& Rect);
        virtual bool obscures (const AVGDRect& Rect, int z);
        virtual std::string getTypeStr ();

    protected:        
        virtual AVGDPoint getPreferredMediaSize();

    private:
        void setupTextures();
    
        std::string m_Filename;
        double m_SensorWidth;
        double m_SensorHeight;
        double m_FocalLength;
        PLAnyBmp m_Bmp;
        std::vector<unsigned int> m_TileTextureIDs;
        AVGSDLDisplayEngine * m_pEngine;
};

#endif //_AVGPanoImage_H_

