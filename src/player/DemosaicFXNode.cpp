

#include "DemosaicFXNode.h"
#include "SDLDisplayEngine.h"

#include "../base/ObjectCounter.h"
#include "../graphics/ShaderRegistry.h"

#include <string>

using namespace std;

namespace avg {

DemosaicFXNode::DemosaicFXNode() 
    : FXNode()
{
    ObjectCounter::get()->incRef(&typeid(*this));
}

DemosaicFXNode::~DemosaicFXNode()
{
    ObjectCounter::get()->decRef(&typeid(*this));
}

void DemosaicFXNode::disconnect()
{
    m_pFilter = GPUDemosaicPtr();
    FXNode::disconnect();
}

GPUFilterPtr DemosaicFXNode::createFilter(const IntPoint& size)
{
    m_pFilter = GPUDemosaicPtr(new GPUDemosaic(size, B8G8R8A8, B8G8R8A8, false));
    return m_pFilter;
}

}


