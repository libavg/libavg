

#include "LaplaceDemosaicFXNode.h"
#include "SDLDisplayEngine.h"

#include "../base/ObjectCounter.h"
#include "../graphics/ShaderRegistry.h"

#include <string>

using namespace std;

namespace avg {

LaplaceDemosaicFXNode::LaplaceDemosaicFXNode() 
    : FXNode()
{
    ObjectCounter::get()->incRef(&typeid(*this));
}

LaplaceDemosaicFXNode::~LaplaceDemosaicFXNode()
{
    ObjectCounter::get()->decRef(&typeid(*this));
}

void LaplaceDemosaicFXNode::disconnect()
{
    m_pFilter = GPULaplaceDemosaicPtr();
    FXNode::disconnect();
}

GPUFilterPtr LaplaceDemosaicFXNode::createFilter(const IntPoint& size)
{
    m_pFilter = GPULaplaceDemosaicPtr(new GPULaplaceDemosaic(size, B8G8R8A8, B8G8R8A8, false));
    return m_pFilter;
}

}


