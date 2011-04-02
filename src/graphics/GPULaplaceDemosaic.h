#ifndef _GPULaplaceDemosaic_H_
#define _GPULaplaceDemosaic_H_

#include "../api.h"
#include "GPUFilter.h"
#include "GLTexture.h"

namespace avg {

class AVG_API GPULaplaceDemosaic: public GPUFilter
{
public:
    GPULaplaceDemosaic(const IntPoint& size, PixelFormat pfSrc, PixelFormat pfDest, 
            bool bStandalone=true);
    virtual ~GPULaplaceDemosaic();
    
    void setParam(double stdDev);
    virtual void applyOnGPU(GLTexturePtr pSrcTex);

private:
    void initShaders();
    void dumpKernel();
    void calcKernel();
};

typedef boost::shared_ptr<GPULaplaceDemosaic> GPULaplaceDemosaicPtr;

} // namespace
#endif

