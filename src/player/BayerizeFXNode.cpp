

#include "BayerizeFXNode.h"
#include "SDLDisplayEngine.h"

#include "../base/ObjectCounter.h"
#include "../graphics/ShaderRegistry.h"

#include <string>

using namespace std;

namespace avg {

BayerizeFXNode::BayerizeFXNode() 
    : FXNode()
{
    ObjectCounter::get()->incRef(&typeid(*this));
}

BayerizeFXNode::~BayerizeFXNode()
{
    ObjectCounter::get()->decRef(&typeid(*this));
}

void BayerizeFXNode::disconnect()
{
    m_pFilter = GPUBayerizePtr();
    FXNode::disconnect();
}

GPUFilterPtr BayerizeFXNode::createFilter(const IntPoint& size)
{
    m_pFilter = GPUBayerizePtr(new GPUBayerize(size, B8G8R8A8, B8G8R8A8, false));
    return m_pFilter;
}

}


