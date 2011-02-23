
#ifndef _GPUDemosaic_H_
#define _GPUDemosaic_H_

#include "../api.h"
#include "GPUFilter.h"
#include "GLTexture.h"

namespace avg {

class AVG_API GPUDemosaic: public GPUFilter
{
public:
    GPUDemosaic(const IntPoint& size, PixelFormat pfSrc, PixelFormat pfDest, 
            double stdDev, bool bStandalone=true);
    virtual ~GPUDemosaic();
    
    void setParam(double stdDev);
    virtual void applyOnGPU(GLTexturePtr pSrcTex);

private:
    void initShaders();
    void dumpKernel();
    void calcKernel();

    double m_StdDev;

    GLTexturePtr m_pGaussCurveTex;
};

typedef boost::shared_ptr<GPUDemosaic> GPUDemosaicPtr;

} // namespace
#endif

