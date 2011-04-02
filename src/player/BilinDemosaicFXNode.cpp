

#include "BilinDemosaicFXNode.h"
#include "SDLDisplayEngine.h"

#include "../base/ObjectCounter.h"
#include "../graphics/ShaderRegistry.h"

#include <string>

using namespace std;

namespace avg {

BilinDemosaicFXNode::BilinDemosaicFXNode() 
    : FXNode()
{
    ObjectCounter::get()->incRef(&typeid(*this));
}

BilinDemosaicFXNode::~BilinDemosaicFXNode()
{
    ObjectCounter::get()->decRef(&typeid(*this));
}

void BilinDemosaicFXNode::disconnect()
{
    m_pFilter = GPUBilinDemosaicPtr();
    FXNode::disconnect();
}

GPUFilterPtr BilinDemosaicFXNode::createFilter(const IntPoint& size)
{
    m_pFilter = GPUBilinDemosaicPtr(new GPUBilinDemosaic(size, B8G8R8A8, B8G8R8A8, false));
    return m_pFilter;
}

}


