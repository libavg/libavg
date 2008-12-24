// Copyright (C) 2008 Archimedes Solutions GmbH,
// Saarbrücker Str. 24b, Berlin, Germany
//
// This file contains proprietary source code and confidential
// information. Its contents may not be disclosed or distributed to
// third parties unless prior specific permission by Archimedes
// Solutions GmbH, Berlin, Germany is obtained in writing. This applies
// to copies made in any form and using any medium. It applies to
// partial as well as complete copies.

#ifndef _IteratingGPUFilter_H_
#define _IteratingGPUFilter_H_

#include "../api.h"

#include "../base/Point.h"
#include "../base/ScopeTimer.h"

#include "../graphics/Filter.h"
#include "../graphics/PBOImage.h"

using namespace std;

namespace avg {

class AVG_API IteratingGPUFilter: public Filter
{
public:
    IteratingGPUFilter(const IntPoint& size, int numIterations);
    virtual ~IteratingGPUFilter();
    virtual BitmapPtr apply(BitmapPtr pImage);
    BitmapPtr getImage() const;

private:
    void createFBO();
    void activate() const;
    void deactivate() const;
    void checkError() const;

    void applyOnGPU();
    virtual void applyOnce(PBOImagePtr pSrc) = 0;
   
    IntPoint m_Size;
    int m_NumIterations;
    PBOImagePtr m_pSrcPBO;
    PBOImagePtr m_pDestPBO;
    unsigned m_FBO;
};

}


#endif 
