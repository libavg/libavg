//
// $Id$
// 

#ifndef _Camera_H_
#define _Camera_H_

#include "../avgconfig.h"

#include "VideoBase.h"
#include "Rect.h"

#include <paintlib/plbitmap.h>

#ifdef AVG_ENABLE_1394
#include <libraw1394/raw1394.h>
#include <libdc1394/dc1394_control.h>
#endif

#include <string>

namespace avg {

class Camera : public VideoBase
{
    public:
        Camera ();
        virtual ~Camera ();
        
        virtual void init (IDisplayEngine * pEngine, Container * pParent, 
            Player * pPlayer);
        virtual std::string getTypeStr ();
        virtual JSFactoryBase* getFactory();

        unsigned int getFeature (const std::string& sFeature);
        void setFeature (const std::string& sFeature, int Value);

    private:
        virtual bool renderToSurface(ISurface * pSurface);
        virtual bool canRenderToBackbuffer(int BitsPerPixel);
        virtual double getFPS();
        virtual void open(int* pWidth, int* pHeight);
        virtual void close();

        std::string m_sDevice;
        double m_FrameRate;
        int m_FrameRateConstant;  // libdc1394 constant for framerate.
        std::string m_sMode;
        int m_Mode;               // libdc1394 constant for mode.
        
#ifdef AVG_ENABLE_1394
        void YUV411toBGR24Line(PLBYTE* pSrc, int y, PLPixel24 * pDestLine);
        void YUV411toBGR24(PLBYTE* pSrc, PLBmpBase * pBmp);
       
        bool findCameraOnPort(int port, raw1394handle_t& FWHandle);
        
        void checkDC1394Error(int Code, const std::string & sMsg);
        void fatalError(const std::string & sMsg);
        void dumpCameraInfo();
        int getFeatureID(const std::string& sFeature);
        
        dc1394_cameracapture m_Camera;
        raw1394handle_t m_FWHandle;
        dc1394_feature_set m_FeatureSet;
        long long m_LastFrameTime;
        bool m_bCameraAvailable;

        static void initCameraSupport();
        static void initYUV2RGBConversionMatrix();
        static bool m_bInitialized;
#endif

};

}

#endif 

