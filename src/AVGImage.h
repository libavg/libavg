//
// $Id$
// 

#ifndef _AVGImage_H_
#define _AVGImage_H_

#include "AVGVisibleNode.h"

#include <string>

class PLBmp;

class AVGImage : 	
	public AVGVisibleNode
{
	public:
        AVGImage (const string& id, int x, int y, int z, 
           int width, int height, double opacity, const string& filename, 
           AVGSDLDisplayEngine * pEngine, AVGContainer * pParent);
        virtual ~AVGImage ();
		virtual void render ();
		virtual void getDirtyRect ();
        virtual std::string getTypeStr ();

    private:
        std::string m_Filename;
        PLBmp * m_pBmp;
};

#endif //_AVGImage_H_

