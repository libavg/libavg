//
// $Id$
// 

#ifndef _AVGCamera_H_
#define _AVGCamera_H_

#include "AVGNode.h"
#include "IAVGCamera.h"
#include "AVGVideoBase.h"
#include "AVGRect.h"

#include <paintlib/plbitmap.h>

#include <string>

#define AVGCAMERA_CID \
{ 0x08ced825, 0x6506, 0x41b4, { 0x87, 0xd4, 0xc8, 0x83, 0x4d, 0xb0, 0x81, 0x92 } }

#define AVGCAMERA_CONTRACTID "@c-base.org/avgcamera;1"

class AVGCamera : public AVGVideoBase /*, public IAVGCamera*/
{
    public:
        NS_DECL_ISUPPORTS

        static AVGCamera * create();

        AVGCamera ();
        virtual ~AVGCamera ();
        
        NS_IMETHOD GetType(PRInt32 *_retval);

        virtual void init (const std::string& id, const std::string& sDevice, 
            double frameRate, const std::string& sMode, bool bOverlay, 
            IAVGDisplayEngine * pEngine, AVGContainer * pParent, 
            AVGPlayer * pPlayer);
        virtual std::string getTypeStr ();

    private:
        virtual bool renderToBmp(PLBmp * pBmp);
        virtual void renderToBackbuffer(PLBYTE * pSurfBits, int Pitch, 
                int BytesPerPixel, const AVGDRect& vpt);
       
        virtual bool open(int* pWidth, int* pHeight);
        virtual void close();
        virtual double getFPS();

        std::string m_sDevice;
        double m_FrameRate;
        std::string m_sMode;

        static void initCameraSupport();
        static bool m_bInitialized;
};

#endif 

