#ifndef _FuzzyDemosaicFXNode_H_
#define _FuzzyDemosaicFXNode_H_

#include "../api.h"

#include "FXNode.h"
#include "../graphics/GPUFuzzyDemosaic.h"

#include <boost/shared_ptr.hpp>

namespace avg {

class SDLDisplayEngine;

class AVG_API FuzzyDemosaicFXNode: public FXNode {
public:
    FuzzyDemosaicFXNode();
    virtual ~FuzzyDemosaicFXNode();

    virtual void disconnect();

private:
    virtual GPUFilterPtr createFilter(const IntPoint& size);

    GPUFuzzyDemosaicPtr m_pFilter;
};

typedef boost::shared_ptr<FuzzyDemosaicFXNode> FuzzyDemosaicFXNodePtr;

}

#endif

