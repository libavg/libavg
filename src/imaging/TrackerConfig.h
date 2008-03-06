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

#include "DeDistort.h"

#include "../base/Rect.h"

#include <boost/shared_ptr.hpp>
#include <string>

#include <libxml/xpath.h>

namespace avg {

struct BlobConfig
{
    BlobConfig(bool bIsTouch);
    virtual ~BlobConfig();
    void load(xmlNodePtr pParentNode);
    void save(xmlTextWriterPtr writer);

    bool m_bIsTouch;
    int m_Threshold; //min pixel val for detection
    double m_Similarity; //max distance for tracking blobs
    double m_AreaBounds[2]; //min, max for area in percent of screen size
    double m_EccentricityBounds[2]; //min, max for Eccentricity
};

typedef boost::shared_ptr<struct BlobConfig> BlobConfigPtr;

struct TrackerConfig
{
    TrackerConfig();
    TrackerConfig(const TrackerConfig& other);
    virtual ~TrackerConfig();
    
    void load(const std::string& sFilename);
    void save(const std::string& sFilename);
    void setParam(const xmlChar* xpElement, const xmlChar* Value);
    std::string getParam(const xmlChar* xpExpr);
    void dump() const;

    // Camera params
    std::string m_sSource;
    std::string m_sDevice;
    std::string m_sPixFmt;
    IntPoint m_Size;
    int m_Channel;
    int m_FPS;
    int m_Brightness;
    int m_Exposure;
    int m_Gamma;
    int m_Gain;
    int m_Shutter;

    std::string m_sCameraMaskFName;
    int m_Prescale;
    int m_HistoryUpdateInterval;
    bool m_bBrighterRegions; // detect brighter or darker pixels rel. to the background.
    bool m_bEventOnMove;
    int m_ContourPrecision;
    BlobConfigPtr m_pTouch;
    BlobConfigPtr m_pTrack;

    bool m_bCreateDebugImages;
    bool m_bCreateFingerImage;

    DeDistortPtr m_pTrafo;

private:
    void parse(bool bOnlyDyn);
    xmlXPathObjectPtr findConfigNodes(const xmlChar* xpExpr);
    void loadCamera(xmlNodePtr pParentNode, bool bOnlyDyn);
    void loadTracker(xmlNodePtr pParentNode);
    xmlDocPtr m_Doc;
    xmlNodePtr m_pRoot;

    std::string m_sFilename;
};

}
#endif
