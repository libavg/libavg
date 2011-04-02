#ifndef _GPUBayerize_H_
#define _GPUBayerize_H_

#include "../api.h"
#include "GPUFilter.h"
#include "GLTexture.h"

namespace avg {

class AVG_API GPUBayerize: public GPUFilter
{
public:
    GPUBayerize(const IntPoint& size, PixelFormat pfSrc, PixelFormat pfDest, 
            bool bStandalone=true);
    virtual ~GPUBayerize();
    
    void setParam(double stdDev);
    virtual void applyOnGPU(GLTexturePtr pSrcTex);

private:
    void initShaders();
    void dumpKernel();
    void calcKernel();
};

typedef boost::shared_ptr<GPUBayerize> GPUBayerizePtr;

} // namespace
#endif

