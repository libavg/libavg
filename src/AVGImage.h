//
// $Id$
// 

#ifndef _AVGImage_H_
#define _AVGImage_H_

#include "AVGVisibleNode.h"

#include <string>

class PLBmp;

//5a938e63-42c6-4218-9cbd-a7afeb4fbd36
#define AVGIMAGE_CID \
{ 0x5a938e63, 0x42c6, 0x4218, { 0x9c, 0xbd, 0xa7, 0xaf, 0xeb, 0x4f, 0xbd, 0x36 } }

#define AVGIMAGE_CONTRACTID "@c-base.org/avgimage;1"

class AVGImage : 	
	public AVGVisibleNode
{
	public:
        NS_DECL_ISUPPORTS

        static AVGImage * create();

        AVGImage ();
        virtual ~AVGImage ();
        
        NS_IMETHOD GetType(PRInt32 *_retval);

        virtual void init (const std::string& id, int x, int y, int z, 
           int width, int height, double opacity, const std::string& filename, 
           AVGDFBDisplayEngine * pEngine, AVGContainer * pParent);
		virtual void render (const PLRect& Rect);
        virtual std::string getTypeStr ();

    private:
        std::string m_Filename;
        PLBmp * m_pBmp;
};

#endif //_AVGImage_H_

