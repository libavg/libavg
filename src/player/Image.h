//
// $Id$
// 

#ifndef _Image_H_
#define _Image_H_

#include "RasterNode.h"

#include <string>

namespace avg {

class Image : public RasterNode
{
	public:
        Image ();
        Image (const xmlNodePtr xmlNode, Container * pParent);
        virtual ~Image ();
        virtual void init (IDisplayEngine * pEngine, 
                Container * pParent, Player * pPlayer);

        const std::string& getHRef() const;
        int getHue() const
        {
            return m_Hue;
        }
        int getSaturation() const
        {
            return m_Saturation;
        }
        
        virtual void render (const DRect& Rect);
        virtual bool obscures (const DRect& Rect, int z);
        virtual std::string getTypeStr ();

    protected:        
        virtual DPoint getPreferredMediaSize();

    private:
        std::string m_Filename;
        std::string m_href;
    
        int m_Hue;
        int m_Saturation;
};

}

#endif

