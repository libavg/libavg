//
// $Id$
// 

#ifndef _AVGWords_H_
#define _AVGWords_H_

#include "AVGNode.h"
#include "IAVGWords.h"

#include <paintlib/plpixel32.h>
#include <paintlib/plbitmap.h>

#include <directfb/directfb.h>

#include <string>

//db87cf71-f379-4ba8-a5c8-e0309467007b
#define AVGWORDS_CID \
{ 0xdb87cf71, 0xf379, 0x4ba8, { 0xa5, 0xc8, 0xe0, 0x30, 0x94, 0x67, 0x00, 0x7b } } 

#define AVGWORDS_CONTRACTID "@c-base.org/avgwords;1"

class AVGWords : public AVGNode, IAVGWords
{
	public:
        NS_DECL_ISUPPORTS
        NS_DECL_IAVGWORDS

        static AVGWords * create();

        AVGWords ();
        virtual ~AVGWords ();
        
        NS_IMETHOD GetType(PRInt32 *_retval);

        virtual void init (const std::string& id, int x, int y, int z, 
           double opacity, int size, const std::string& font, 
           const std::string& str, const std::string& color,
           AVGDFBDisplayEngine * pEngine, AVGContainer * pParent, AVGPlayer * pPlayer);
		virtual void render (const PLRect& Rect);
        virtual std::string getTypeStr ();

    protected:        
        virtual PLPoint getPreferredMediaSize();
    
    private:
        void changeFont();
        void drawString();
        PLPixel32 colorStringToColor(const std::string & colorString);

        std::string m_FontName;
        std::string m_Str;
        std::string m_ColorName;
        PLPixel32 m_Color;
        int m_Size;
        IDirectFBFont * m_pFont;
        PLBmp * m_pBmp;
        PLPoint m_StringExtents;
};

#endif //_AVGWords_H_

