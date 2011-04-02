#ifndef _BilinDemosaicFXNode_H_
#define _BilinDemosaicFXNode_H_

#include "../api.h"

#include "FXNode.h"
#include "../graphics/GPUBilinDemosaic.h"

#include <boost/shared_ptr.hpp>

namespace avg {

class SDLDisplayEngine;

class AVG_API BilinDemosaicFXNode: public FXNode {
public:
    BilinDemosaicFXNode();
    virtual ~BilinDemosaicFXNode();

    virtual void disconnect();

private:
    virtual GPUFilterPtr createFilter(const IntPoint& size);

    GPUBilinDemosaicPtr m_pFilter;
};

typedef boost::shared_ptr<BilinDemosaicFXNode> BilinDemosaicFXNodePtr;

}

#endif

