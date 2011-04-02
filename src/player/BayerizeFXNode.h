#ifndef _BayerizeFXNode_H_
#define _BayerizeFXNode_H_

#include "../api.h"

#include "FXNode.h"
#include "../graphics/GPUBayerize.h"

#include <boost/shared_ptr.hpp>

namespace avg {

class SDLDisplayEngine;

class AVG_API BayerizeFXNode: public FXNode {
public:
    BayerizeFXNode();
    virtual ~BayerizeFXNode();

    virtual void disconnect();

private:
    virtual GPUFilterPtr createFilter(const IntPoint& size);

    GPUBayerizePtr m_pFilter;
};

typedef boost::shared_ptr<BayerizeFXNode> BayerizeFXNodePtr;

}

#endif

