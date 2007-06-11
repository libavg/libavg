
#include "V4LCamera.h"
using namespace avg;

V4LCamera::V4LCamera(std::string sDevice, double FrameRate, std::string sMode, bool bColor)
    /*: m_sDevice(sDevice),
      m_FrameRate(FrameRate),
      m_sMode(sMode),
      m_bColor(bColor),
      m_bCameraAvailable(false) */
{

}

V4LCamera::~V4LCamera() 
{}

void V4LCamera::open()
{}

void V4LCamera::close()
{}

IntPoint V4LCamera::getImgSize()
{}

BitmapPtr V4LCamera::getImage(bool bWait)
{}

bool V4LCamera::isCameraAvailable()
{
    return false;
}

const std::string& V4LCamera::getDevice() const
{
    return "";
}

double V4LCamera::getFrameRate() const
{
    return 0;
}

const std::string& V4LCamera::getMode() const
{
    return "";
}


unsigned int V4LCamera::getFeature(const std::string& sFeature) const
{
    return 0;
}

void V4LCamera::setFeature(const std::string& sFeature, int Value)
{}
