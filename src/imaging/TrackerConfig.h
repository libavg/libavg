//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  Current versions can be found at www.libavg.de
//
//  Original author of this file is igor@c-base.org.
//

#ifndef _TrackerConfig_H_
#define _TrackerConfig_H_

#include "../base/Rect.h"

#include <boost/shared_ptr.hpp>
#include <string>

#include <libxml/xpath.h>

namespace avg {

class DeDistort;
typedef boost::shared_ptr<DeDistort> DeDistortPtr;

struct TrackerConfig
{
    TrackerConfig();
    TrackerConfig(const TrackerConfig& Other);
    virtual ~TrackerConfig();
    
    void load(const std::string& sFilename);
    void save(const std::string& sFilename);
    void setParam(const std::string& sXPathExpr, const std::string& sValue);
    std::string getParam(const std::string& sXPathExpr) const;
    bool getBoolParam(const std::string& sXPathExpr) const;
    int getIntParam(const std::string& sXPathExpr) const;
    double getDoubleParam(const std::string& sXPathExpr) const;
    DPoint getPointParam(const std::string& sXPathExpr) const;
    xmlNodePtr getXmlNode(const std::string& sXPathExpr) const;

    DeDistortPtr getTransform() const;
    void setTransform(DeDistortPtr pDeDistort);
    
    void dump() const;

private:
    xmlXPathObjectPtr findConfigNodes(const std::string& sXPathExpr) const;
    
    xmlDocPtr m_Doc;
    xmlNodePtr m_pRoot;

    std::string m_sFilename;
};
typedef boost::shared_ptr<TrackerConfig> TrackerConfigPtr;

}
#endif
