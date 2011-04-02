#ifndef _GPUFuzzyDemosaic_H_
#define _GPUFuzzyDemosaic_H_

#include "../api.h"
#include "GPUFilter.h"
#include "GLTexture.h"

namespace avg {

class AVG_API GPUFuzzyDemosaic: public GPUFilter
{
public:
    GPUFuzzyDemosaic(const IntPoint& size, PixelFormat pfSrc, PixelFormat pfDest, 
            bool bStandalone=true);
    virtual ~GPUFuzzyDemosaic();
    
    void setParam(double stdDev);
    virtual void applyOnGPU(GLTexturePtr pSrcTex);

private:
    void initShaders();
    void dumpKernel();
    void calcKernel();
};

typedef boost::shared_ptr<GPUFuzzyDemosaic> GPUFuzzyDemosaicPtr;

} // namespace
#endif

