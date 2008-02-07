//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2006 Ulrich von Zadow
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

#include "TrackerConfig.h"
#include "trackerconfigdtd.h"
#include "DeDistort.h"

#include "../base/XMLHelper.h"
#include "../base/Logger.h"
#include "../base/FileHelper.h"
#include "../base/Exception.h"

#include <libxml/parser.h>
#include <libxml/xmlwriter.h>
#include <libxml/xmlstring.h>

#include <cstring>
#include <sstream>

using namespace std;

namespace avg {

void assureEmptyNode(const char * pNodeName)
{
    if (strcmp(pNodeName, "text") && strcmp(pNodeName, "comment")) {
        AVG_TRACE(Logger::WARNING, "TrackerConfig: Unexpected node " << pNodeName);
    }
}

BlobConfig::BlobConfig(bool bIsTouch)
    : m_bIsTouch(bIsTouch),
      m_Threshold(128),
      m_Similarity(31)
{
      m_AreaBounds[0] = 80;
      m_AreaBounds[1] = 450;
      m_EccentricityBounds[0] = 1; 
      m_EccentricityBounds[1] = 3;
} 

BlobConfig::~BlobConfig()
{
}

void BlobConfig::load(xmlNodePtr pParentNode, const string& sFilename)
{
    xmlNodePtr curXmlChild = pParentNode->xmlChildrenNode;
    while (curXmlChild) {
        const char * pNodeName = (const char *)curXmlChild->name;
        if (!strcmp(pNodeName, "threshold")) {
            m_Threshold = getRequiredIntAttr(curXmlChild, "value");
        } else if (!strcmp(pNodeName, "similarity")) {
            m_Similarity = getRequiredDoubleAttr(curXmlChild, "value");
        } else if (!strcmp(pNodeName, "areabounds")) {
            m_AreaBounds[0] = getRequiredIntAttr(curXmlChild, "min");
            m_AreaBounds[1] = getRequiredIntAttr(curXmlChild, "max");
        } else if (!strcmp(pNodeName, "eccentricitybounds")) {
            m_EccentricityBounds[0] = getRequiredDoubleAttr(curXmlChild, "min");
            m_EccentricityBounds[1] = getRequiredDoubleAttr(curXmlChild, "max");
        } else {
            assureEmptyNode(pNodeName);
        }
        curXmlChild = curXmlChild->next;
    }
}

void BlobConfig::save(xmlTextWriterPtr writer) 
{
    int rc;
    if (m_bIsTouch) {
        rc = xmlTextWriterStartElement(writer, BAD_CAST "touch");
    } else {
        rc = xmlTextWriterStartElement(writer, BAD_CAST "track");
    }
    writeSimpleXMLNode(writer, "threshold", m_Threshold);
    writeSimpleXMLNode(writer, "similarity", m_Similarity);
    writeMinMaxXMLNode(writer, "areabounds", m_AreaBounds);
    writeMinMaxXMLNode(writer, "eccentricitybounds", m_EccentricityBounds);
    rc = xmlTextWriterEndElement(writer);
}

TrackerConfig::TrackerConfig()
    : m_sPixFmt("MONO8"),
      m_Size(640, 480),
      m_Channel(0),
      m_FPS(30),
      m_Brightness(128),
      m_Exposure(128),
      m_Gamma(1),
      m_Gain(128),
      m_Shutter(128),
      m_Prescale(5),
      m_HistoryUpdateInterval(5),
      m_bBrighterRegions(true),
      m_bEventOnMove(true),
      m_ContourPrecision(50),
      m_bCreateDebugImages(false),
      m_bCreateFingerImage(false),
      m_pTrafo(new DeDistort()),
      m_Doc(0)
{
} 

TrackerConfig::TrackerConfig(const TrackerConfig& other)
{
    *this = other;
    if (m_pTouch) {
        *m_pTouch = *(other.m_pTouch);
    }
    if (m_pTrack) {
        *m_pTrack = *(other.m_pTrack);
    }
    *m_pTrafo = *(other.m_pTrafo);
}

TrackerConfig::~TrackerConfig()
{
    // TODO: free complete m_Doc.
}

void TrackerConfig::load(const string& sCustomFilename)
{
    // TODO: There is duplicated code here and in Player::loadFile which belongs
    // in a lower-level xml handling class.
    registerDTDEntityLoader("trackerconfig.dtd", g_pTrackerConfigDTD);
    string sFilename(sCustomFilename);
    if (sCustomFilename.empty()) {
        sFilename = "/etc/avgtrackerrc";
        if (!fileExists(sFilename)) {
            sFilename = getConfigFilename();
        }
    } 
    xmlDtdPtr dtd;
    string sDTDFName = "trackerconfig.dtd";
    dtd = xmlParseDTD(NULL, (const xmlChar*) sDTDFName.c_str());
    if (!dtd) {
        AVG_TRACE(Logger::WARNING, 
                "DTD not found at " << sDTDFName << ". Not validating trackerconfig files.");
    }

    m_Doc = xmlParseFile(sFilename.c_str());
    if (!m_Doc) {
        AVG_TRACE(Logger::ERROR, "Could not open tracker config file " 
                << sFilename << ". Using defaults which will probably not work.");
        return;
    }

    xmlValidCtxtPtr cvp = xmlNewValidCtxt();
    cvp->error = xmlParserValidityError;
    cvp->warning = xmlParserValidityWarning;
    int valid=xmlValidateDtd(cvp, m_Doc, dtd);  
    xmlFreeValidCtxt(cvp);
    if (!valid) {
        throw (Exception(AVG_ERR_XML_PARSE, 
                sFilename + " does not validate."));
    }

    m_pRoot = xmlDocGetRootElement(m_Doc);
    
    xmlFreeDtd(dtd);

    parse(false);
}

void TrackerConfig::parse(bool bOnlyDyn)
{
    xmlNodePtr curXmlChild = m_pRoot->xmlChildrenNode;
    while (curXmlChild) {
        const char * pNodeName = (const char *)curXmlChild->name;
        if (!strcmp(pNodeName, "camera")) {
            loadCamera(curXmlChild, getConfigFilename(), bOnlyDyn);
        } else if (!strcmp(pNodeName, "tracker")) {
            loadTracker(curXmlChild, getConfigFilename());
        } else if (!strcmp(pNodeName, "transform")) {
            m_pTrafo->load(DPoint(m_Size), curXmlChild);
        } else {
            assureEmptyNode(pNodeName);
        }
        curXmlChild = curXmlChild->next;
    }
}

xmlXPathObjectPtr TrackerConfig::findConfigNodes(const xmlChar* xpExpr)
{
    xmlXPathContextPtr xpCtx;
    xmlXPathObjectPtr xpElement;

    xpCtx = xmlXPathNewContext(m_Doc);
    if(!xpCtx) {
        AVG_TRACE(Logger::ERROR, "Unable to create new XPath context");
        return NULL;
    }

    xpElement = xmlXPathEvalExpression(xpExpr, xpCtx);
    if(!xpElement) {
        AVG_TRACE(Logger::ERROR, "Unable to evaluate XPath expression '"
            << xpExpr << "'");
        xmlXPathFreeContext(xpCtx);
        return NULL;
    }
    
    xmlXPathFreeContext(xpCtx);

    return xpElement;
}

void TrackerConfig::setParam(const xmlChar* xpExpr, const xmlChar* Value)
{
    xmlXPathObjectPtr xpElement = findConfigNodes(xpExpr);
    xmlNodeSetPtr nodes = xpElement->nodesetval;
    
    if (!nodes || nodes->nodeNr == 0)
        throw (Exception(AVG_ERR_OPTION_UNKNOWN, 
                    string("setParam(): cannot find requested element ")+string((char *)xpExpr)));
    
    for(int i = nodes->nodeNr - 1; i >= 0; i--) {
        assert(nodes->nodeTab[i]);

        xmlNodeSetContent(nodes->nodeTab[i], Value);
        if (nodes->nodeTab[i]->type != XML_NAMESPACE_DECL)
            nodes->nodeTab[i] = NULL;
    }
    
    xmlXPathFreeObject(xpElement);

    parse(true);        
}

string TrackerConfig::getParam(const xmlChar* xpExpr)
{
    xmlXPathObjectPtr xpElement = findConfigNodes(xpExpr);
    xmlNodeSetPtr nodes = xpElement->nodesetval;
    
    if (!nodes || nodes->nodeNr == 0)
        throw (Exception(AVG_ERR_OPTION_UNKNOWN, 
                    string("getParam(): cannot find requested element ")+string((char *)xpExpr)));
    else if (nodes->nodeNr > 1)
        AVG_TRACE(Logger::WARNING,
            "getParam(): expression selects more than one node. Returning the first.");
    
    xmlChar *xsRc = xmlNodeGetContent(nodes->nodeTab[0]);
    string sValue((char *)xsRc);
    
    xmlFree(xsRc);
    xmlXPathFreeObject(xpElement);

    return sValue;
}

void TrackerConfig::dump() const
{
    cerr << "Tracker config: " << endl;
    cerr << "  Camera: " << endl;
    cerr << "    Source: " << m_sSource << endl;
    cerr << "    Device: " << m_sDevice << endl;
    cerr << "    PixFmt: " << m_sPixFmt << endl;
    cerr << "    Size: " << m_Size << endl;
    cerr << "    Channel: " << m_Channel << endl;
    cerr << "    FPS: " << m_FPS << endl;
    cerr << "    Brightness: " << m_Brightness << endl;
    cerr << "    Exposure: " << m_Exposure << endl;
    cerr << "    Gamma: " << m_Gamma << endl;
    cerr << "    Gain: " << m_Gain << endl;
    cerr << "    Shutter: " << m_Shutter << endl;
    cerr << "  Tracker:" << endl;
    cerr << "    Prescale: " << m_Prescale << endl;
    cerr << "    HistoryUpdateInterval: " << m_HistoryUpdateInterval << endl;
    cerr << "    BrighterRegions: " << m_bBrighterRegions << endl;
    cerr << "    EventOnMove: " << m_bEventOnMove << endl;
    cerr << "    ContourPrecision: " << m_ContourPrecision << endl;
    // TODO: Dump Touch/Track
    m_pTrafo->dump();
}

void TrackerConfig::save(const string& sCustomFilename)
{
    string sFilename(sCustomFilename);
    if (sFilename.empty()) {
        sFilename = getConfigFilename();
    }

    AVG_TRACE(Logger::CONFIG, "Saving tracker configuration to " 
            << sFilename << ".");

    if (m_Doc)
        xmlSaveFileEnc(sFilename.c_str(), m_Doc, "utf-8");
    else
        throw (Exception(AVG_ERR_FILEIO, 
                    "save(): tracker configuration not initialized"));
}

void TrackerConfig::loadCamera(xmlNodePtr pParentNode, const string& sFilename, bool bOnlyDyn)
{
    xmlNodePtr curXmlChild = pParentNode->xmlChildrenNode;
    while (curXmlChild) {
        bool bManaged = true;
        const char * pNodeName = (const char *)curXmlChild->name;
        if (!bOnlyDyn)
        {
            if (!strcmp(pNodeName, "source")) {
                m_sSource = getRequiredStringAttr(curXmlChild, "value");
            } else if (!strcmp(pNodeName, "device")) {
                m_sDevice = getRequiredStringAttr(curXmlChild, "value");
            } else if (!strcmp(pNodeName, "format")) {
                m_sPixFmt = getRequiredStringAttr(curXmlChild, "value");
            } else if (!strcmp(pNodeName, "size")) {
                m_Size.x = getRequiredIntAttr(curXmlChild, "x");
                m_Size.y = getRequiredIntAttr(curXmlChild, "y");
            }
            else bManaged = false;
        }
        
        if (!strcmp(pNodeName, "channel")) {
            // TODO: V4L2 channel is not updated on-the-fly
            m_Channel = getRequiredIntAttr(curXmlChild, "value");
        } else if (!strcmp(pNodeName, "fps")) {
            m_FPS = getRequiredIntAttr(curXmlChild, "value");
        } else if (!strcmp(pNodeName, "brightness")) {
            m_Brightness = getRequiredIntAttr(curXmlChild, "value");
        } else if (!strcmp(pNodeName, "exposure")) {
            m_Exposure = getRequiredIntAttr(curXmlChild, "value");
        } else if (!strcmp(pNodeName, "gamma")) {
            m_Gamma = getRequiredIntAttr(curXmlChild, "value");
        } else if (!strcmp(pNodeName, "gain")) {
            m_Gain = getRequiredIntAttr(curXmlChild, "value");
        } else if (!strcmp(pNodeName, "shutter")) {
            m_Shutter = getRequiredIntAttr(curXmlChild, "value");
        } else {
            if (!bOnlyDyn && !bManaged) {
                assureEmptyNode(pNodeName);
            }
        }
        curXmlChild = curXmlChild->next;
    }
}

void TrackerConfig::loadTracker(xmlNodePtr pParentNode, const string& sFilename)
{
    xmlNodePtr curXmlChild = pParentNode->xmlChildrenNode;
    while (curXmlChild) {
        const char * pNodeName = (const char *)curXmlChild->name;
        if (!strcmp(pNodeName, "prescale")) {
            m_Prescale = getRequiredIntAttr(curXmlChild, "value");
        } else if (!strcmp(pNodeName, "historyupdateinterval")) {
            m_HistoryUpdateInterval = getRequiredIntAttr(curXmlChild, "value");
        } else if (!strcmp(pNodeName, "brighterregions")) {
            m_bBrighterRegions = getRequiredBoolAttr(curXmlChild, "value");
        } else if (!strcmp(pNodeName, "eventonmove")) {
            m_bEventOnMove = getRequiredBoolAttr(curXmlChild, "value");
        } else if (!strcmp(pNodeName, "contourprecision")) {
            m_ContourPrecision = getRequiredIntAttr(curXmlChild, "value");
        } else if (!strcmp(pNodeName, "touch")) {
            m_pTouch = BlobConfigPtr(new BlobConfig(true));
            m_pTouch->load(curXmlChild, sFilename);
        } else if (!strcmp(pNodeName, "track")) {
            m_pTrack = BlobConfigPtr(new BlobConfig(false));
            m_pTrack->load(curXmlChild, sFilename);
        } else {
            assureEmptyNode(pNodeName);
        }
        curXmlChild = curXmlChild->next;
    }
}

std::string TrackerConfig::getConfigFilename()
{
    char * pHome = getenv("HOME");
    if (pHome) {
        return string(pHome)+"/.avgtrackerrc"; 
    } else {
        return "";
    }
}

}
