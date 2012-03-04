//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2011 Ulrich von Zadow
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
#include "../base/StringHelper.h"
#include "../base/ConfigMgr.h"

#include <libxml/parser.h>
#include <libxml/xmlwriter.h>
#include <libxml/xmlstring.h>

#include <cstring>
#include <sstream>
#include <iostream>

using namespace std;

namespace avg {

TrackerConfig::TrackerConfig()
    : m_Doc(0)
{
} 

TrackerConfig::TrackerConfig(const TrackerConfig& other)
{
    m_Doc = 0;
    if (other.m_Doc) {
        m_Doc = xmlCopyDoc(other.m_Doc, true);
        m_sFilename = other.m_sFilename;
        m_pRoot = xmlDocGetRootElement(m_Doc);
    }
}

TrackerConfig::~TrackerConfig()
{
    xmlFreeDoc(m_Doc);
}

void TrackerConfig::loadConfigFile(const string& sFilename)
{
    // TODO: There is duplicated code here and in Player::loadFile which belongs
    // in a lower-level xml handling class.
    registerDTDEntityLoader("trackerconfig.dtd", g_pTrackerConfigDTD);
    xmlDtdPtr dtd;
    string sDTDFName = "trackerconfig.dtd";
    dtd = xmlParseDTD(NULL, (const xmlChar*) sDTDFName.c_str());
    if (!dtd) {
        AVG_TRACE(Logger::WARNING, 
                "DTD not found at " << sDTDFName 
                << ". Not validating trackerconfig files.");
    }

    // xmlParseFile crashes for some reason under Lion.
    string sFileContents;
    readWholeFile(sFilename, sFileContents);
    m_Doc = xmlParseMemory(sFileContents.c_str(), sFileContents.length());
    if (!m_Doc) {
        AVG_TRACE(Logger::ERROR, "Could not open tracker config file " 
                << sFilename << ". Using defaults which will probably not work.");
        return;
    }

    xmlValidCtxtPtr cvp = xmlNewValidCtxt();
    cvp->error = xmlParserValidityError;
    cvp->warning = xmlParserValidityWarning;
    int isValid = xmlValidateDtd(cvp, m_Doc, dtd);  
    xmlFreeValidCtxt(cvp);
    if (!isValid) {
        throw (Exception(AVG_ERR_XML_PARSE, sFilename + " does not validate."));
    }

    m_pRoot = xmlDocGetRootElement(m_Doc);
    xmlFreeDtd(dtd);
    m_sFilename = sFilename;

    AVG_TRACE(Logger::CONFIG, "Reading Tracker config file from " << sFilename);
}

void TrackerConfig::load()
{
    // Give precedence to local configuration
    string sFName = "avgtrackerrc";
    if (fileExists(sFName) || !fileExists(getGlobalConfigDir() + sFName)) {
        loadConfigFile(sFName);
    } else {
        loadConfigFile(getGlobalConfigDir() + sFName);
    }
}

xmlXPathObjectPtr TrackerConfig::findConfigNodes(const string& sXPathExpr) const
{
    string sFullPath = string("/trackerconfig"+sXPathExpr);
    xmlXPathContextPtr xpCtx;
    xmlXPathObjectPtr xpElement;

    xpCtx = xmlXPathNewContext(m_Doc);
    if(!xpCtx) {
        AVG_TRACE(Logger::ERROR, "Unable to create new XPath context");
        return NULL;
    }

    xpElement = xmlXPathEvalExpression(BAD_CAST sFullPath.c_str(), xpCtx);
    if(!xpElement) {
        AVG_TRACE(Logger::ERROR, "Unable to evaluate XPath expression '"
            << sFullPath << "'");
        xmlXPathFreeContext(xpCtx);
        return NULL;
    }
    
    xmlXPathFreeContext(xpCtx);

    return xpElement;
}

void TrackerConfig::setParam(const string& sXPathExpr, const string& sValue)
{
    xmlXPathObjectPtr xpElement = findConfigNodes(sXPathExpr);
    xmlNodeSetPtr nodes = xpElement->nodesetval;
    
    if (!nodes || nodes->nodeNr == 0)
        throw (Exception(AVG_ERR_OPTION_UNKNOWN, 
                    string("setParam(): cannot find requested element ")+sXPathExpr));
    
    for (int i = nodes->nodeNr-1; i >= 0; i--) {
        AVG_ASSERT(nodes->nodeTab[i]);

        xmlNodeSetContent(nodes->nodeTab[i], BAD_CAST sValue.c_str());
        if (nodes->nodeTab[i]->type != XML_NAMESPACE_DECL)
            nodes->nodeTab[i] = NULL;
    }
    
    xmlXPathFreeObject(xpElement);
}

string TrackerConfig::getParam(const string& sXPathExpr) const
{
    xmlXPathObjectPtr xpElement = findConfigNodes(sXPathExpr);
    xmlNodeSetPtr nodes = xpElement->nodesetval;
    
    if (!nodes || nodes->nodeNr == 0) {
        throw (Exception(AVG_ERR_OPTION_UNKNOWN, 
                    string("getParam(): cannot find requested element ")+sXPathExpr));
    } else if (nodes->nodeNr > 1) {
        AVG_TRACE(Logger::WARNING,
            "getParam(): expression selects more than one node. Returning the first.");
    }

    xmlChar* xsRc = xmlNodeGetContent(nodes->nodeTab[0]);
    string sValue((char *)xsRc);
    
    xmlFree(xsRc);
    xmlXPathFreeObject(xpElement);

    return sValue;
}

bool TrackerConfig::getBoolParam(const std::string& sXPathExpr) const
{
    return stringToBool(getParam(sXPathExpr));
}

int TrackerConfig::getIntParam(const std::string& sXPathExpr) const
{
    return stringToInt(getParam(sXPathExpr));
}

float TrackerConfig::getFloatParam(const std::string& sXPathExpr) const
{
    return stringToFloat(getParam(sXPathExpr));
}

glm::vec2 TrackerConfig::getPointParam(const std::string& sXPathExpr) const
{
    return glm::vec2(getFloatParam(sXPathExpr+"@x"), getFloatParam(sXPathExpr+"@y"));
}

FRect TrackerConfig::getRectParam(const std::string& sXPathExpr) const
{
     glm::vec2 pos1 = glm::vec2(getFloatParam(sXPathExpr+"@x1"), 
            getFloatParam(sXPathExpr+"@y1"));
     glm::vec2 pos2 = glm::vec2(getFloatParam(sXPathExpr+"@x2"),
            getFloatParam(sXPathExpr+"@y2"));
     return FRect(pos1, pos2);
}

xmlNodePtr TrackerConfig::getXmlNode(const std::string& sXPathExpr) const
{
    xmlXPathObjectPtr xpElement = findConfigNodes(sXPathExpr);
    xmlNodeSetPtr nodes = xpElement->nodesetval;
    
    if (!nodes || nodes->nodeNr == 0) {
        throw (Exception(AVG_ERR_OPTION_UNKNOWN, 
                string("getParam(): cannot find requested element ")+sXPathExpr));
    } else if (nodes->nodeNr > 1) {
        AVG_TRACE(Logger::WARNING,
            "getXmlNode(): expression selects more than one node. Returning the first.");
    }
    return nodes->nodeTab[0];
}

DeDistortPtr TrackerConfig::getTransform() const
{
    glm::vec2 CameraExtents = getPointParam("/camera/size/");
    DeDistortPtr pDD(new DeDistort);
    pDD->load(CameraExtents, *this);
    return pDD;
}

void TrackerConfig::setTransform(DeDistortPtr pDeDistort)
{
    pDeDistort->save(*this);
}

void TrackerConfig::dump() const
{
    string s;
    xmlBufferPtr pBuffer = xmlBufferCreate();
    xmlNodeDump(pBuffer, m_Doc, m_pRoot, 0, 0);
    cerr << xmlBufferContent(pBuffer) << endl;
}

void TrackerConfig::save()
{
    AVG_TRACE(Logger::CONFIG, "Saving tracker configuration to " 
            << m_sFilename << ".");

    if (m_Doc) {
        if (fileExists(m_sFilename)) {
            string sBakFile = m_sFilename + ".bak";
            unlink(sBakFile.c_str());
            if (rename(m_sFilename.c_str(), sBakFile.c_str())) {
                AVG_TRACE(Logger::WARNING, "Cannot create tracker config backup. Backing "
                        "it up on current workdir.");
                copyFile(m_sFilename, "avgtrackerrc.bak");
            }
        }
        xmlSaveFileEnc(m_sFilename.c_str(), m_Doc, "utf-8");
    } else
        throw (Exception(AVG_ERR_FILEIO, 
                    "save(): tracker configuration not initialized"));
}

}
