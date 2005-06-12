//
// $Id$
// 

#ifndef _Image_H_
#define _Image_H_

#include "RasterNode.h"

#include <string>

class PLBmp;

namespace avg {

class Image : public RasterNode
{
	public:
        Image ();
        virtual ~Image ();
        
        virtual void init (IDisplayEngine * pEngine, 
                Container * pParent, Player * pPlayer);
        virtual void render (const DRect& Rect);
        virtual bool obscures (const DRect& Rect, int z);
        virtual std::string getTypeStr ();
        virtual JSFactoryBase* getFactory();

    protected:        
        virtual DPoint getPreferredMediaSize();

    private:
        friend class ImageFactory;
       
        std::string m_Filename;
        std::string m_href;
    
        int m_Hue;
        int m_Saturation;
};

}

#endif //_Image_H_

