

#include "FuzzyDemosaicFXNode.h"
#include "SDLDisplayEngine.h"

#include "../base/ObjectCounter.h"
#include "../graphics/ShaderRegistry.h"

#include <string>

using namespace std;

namespace avg {

FuzzyDemosaicFXNode::FuzzyDemosaicFXNode() 
    : FXNode()
{
    ObjectCounter::get()->incRef(&typeid(*this));
}

FuzzyDemosaicFXNode::~FuzzyDemosaicFXNode()
{
    ObjectCounter::get()->decRef(&typeid(*this));
}

void FuzzyDemosaicFXNode::disconnect()
{
    m_pFilter = GPUFuzzyDemosaicPtr();
    FXNode::disconnect();
}

GPUFilterPtr FuzzyDemosaicFXNode::createFilter(const IntPoint& size)
{
    m_pFilter = GPUFuzzyDemosaicPtr(new GPUFuzzyDemosaic(size, B8G8R8A8, B8G8R8A8, false));
    return m_pFilter;
}

}


