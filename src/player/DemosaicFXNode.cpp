

#include "DemosaicFXNode.h"
#include "SDLDisplayEngine.h"

#include "../base/ObjectCounter.h"
#include "../graphics/ShaderRegistry.h"

#include <string>

using namespace std;

namespace avg {

DemosaicFXNode::DemosaicFXNode() 
    : FXNode(),
      m_StdDev(1)
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

void DemosaicFXNode::setParam(double stdDev)
{
    m_StdDev = stdDev;
    if (m_pFilter) {
        m_pFilter->setParam(stdDev);
    }
}

GPUFilterPtr DemosaicFXNode::createFilter(const IntPoint& size)
{
    m_pFilter = GPUDemosaicPtr(new GPUDemosaic(size, B8G8R8A8, B8G8R8A8, m_StdDev, 
            false));
    return m_pFilter;
}

}


