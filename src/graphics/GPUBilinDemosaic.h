#ifndef _GPUBilinDemosaic_H_
#define _GPUBilinDemosaic_H_

#include "../api.h"
#include "GPUFilter.h"
#include "GLTexture.h"

namespace avg {

class AVG_API GPUBilinDemosaic: public GPUFilter
{
public:
    GPUBilinDemosaic(const IntPoint& size, PixelFormat pfSrc, PixelFormat pfDest, 
            bool bStandalone=true);
    virtual ~GPUBilinDemosaic();
    
    void setParam(double stdDev);
    virtual void applyOnGPU(GLTexturePtr pSrcTex);

private:
    void initShaders();
    void dumpKernel();
    void calcKernel();
};

typedef boost::shared_ptr<GPUBilinDemosaic> GPUBilinDemosaicPtr;

} // namespace
#endif

