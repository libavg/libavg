#ifndef _LaplaceDemosaicFXNode_H_
#define _LaplaceDemosaicFXNode_H_

#include "../api.h"

#include "FXNode.h"
#include "../graphics/GPULaplaceDemosaic.h"

#include <boost/shared_ptr.hpp>

namespace avg {

class SDLDisplayEngine;

class AVG_API LaplaceDemosaicFXNode: public FXNode {
public:
    LaplaceDemosaicFXNode();
    virtual ~LaplaceDemosaicFXNode();

    virtual void disconnect();

private:
    virtual GPUFilterPtr createFilter(const IntPoint& size);

    GPULaplaceDemosaicPtr m_pFilter;
};

typedef boost::shared_ptr<LaplaceDemosaicFXNode> LaplaceDemosaicFXNodePtr;

}

#endif

