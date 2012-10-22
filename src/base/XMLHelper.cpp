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
//

#include "XMLHelper.h"
#include "Exception.h"

#include <libxml/parserInternals.h>
#include <cstring>
#include <iostream>

using namespace std;

namespace avg {

string getXmlChildrenAsString(const xmlDocPtr xmlDoc, const xmlNodePtr& xmlNode)
{
    string s;
    xmlBufferPtr pBuffer = xmlBufferCreate();
    xmlNodeDump(pBuffer, xmlDoc, xmlNode, 0, 0);

    s = (const char *)xmlBufferContent(pBuffer);
    size_t StartPos = s.find('>')+1;
    size_t EndPos = s.rfind('<')-1;
    if (StartPos > EndPos) {
        s = "";
    } else {
        s = s.substr(StartPos, EndPos-StartPos+1);
    }
    xmlBufferFree(pBuffer);
    return s;
}

static xmlExternalEntityLoader DefaultLoaderProc = 0;
static std::map<string, string> g_DTDMap;

xmlParserInputPtr
DTDExternalEntityLoader(const char *pURL, const char *pID, xmlParserCtxtPtr ctxt) 
{
    xmlParserInputPtr ret;
    /* lookup for the fileID depending on ID */
    std::map<string, string>::iterator it = g_DTDMap.find(pURL);

    if (it != g_DTDMap.end()) {
        ret = xmlNewStringInputStream(ctxt, (const xmlChar *)(it->second.c_str()));
        return(ret);
    } else {
        ret = DefaultLoaderProc(pURL, pID, ctxt);
        return(ret);
    }
}

void registerDTDEntityLoader(const string& sID, const string& sDTD)
{
    g_DTDMap[sID] = sDTD;
    if (!DefaultLoaderProc) {
        DefaultLoaderProc = xmlGetExternalEntityLoader();
    }
    xmlSetExternalEntityLoader(DTDExternalEntityLoader);
}


XmlValidator::XmlValidator(const std::string& sSchema)
    : m_ParserCtxt(0),
      m_Schema(0),
      m_ValidCtxt(0)
{
    xmlPedanticParserDefault(1);

    // Schema setup
    m_ParserCtxt = xmlSchemaNewMemParserCtxt(sSchema.c_str(), sSchema.length());
    if (!m_ParserCtxt) {
        throw (Exception(AVG_ERR_XML_PARSE, ""));
    }
    m_Schema = xmlSchemaParse(m_ParserCtxt);
    if (!m_Schema) {
        throw (Exception(AVG_ERR_XML_PARSE, ""));
    }

    m_ValidCtxt = xmlSchemaNewValidCtxt(m_Schema);
    if (!m_ValidCtxt) {
        throw (Exception(AVG_ERR_XML_PARSE, ""));
    }
}

XmlValidator::~XmlValidator()
{
    if (m_Schema) {
        xmlSchemaFree(m_Schema);
    }
    if (m_ParserCtxt) {
        xmlSchemaFreeParserCtxt(m_ParserCtxt);
    }
    if (m_ValidCtxt) {
        xmlSchemaFreeValidCtxt(m_ValidCtxt);
    }
}

void XmlValidator::validate(const std::string& sXML)
{
    xmlDocPtr doc;
    doc = xmlParseMemory(sXML.c_str(), int(sXML.length()));
    if (!doc) {
        throw (Exception(AVG_ERR_XML_PARSE, ""));
    }
    int err = xmlSchemaValidateDoc(m_ValidCtxt, doc);
    AVG_ASSERT(err != -1);
    if (err) {
        xmlFreeDoc(doc);
        throw (Exception(AVG_ERR_XML_PARSE, ""));
    }
    xmlFreeDoc(doc);
}

void validateXml(const std::string& sXML, const std::string& sSchema)
{
    XmlValidator validator(sSchema);

    validator.validate(sXML);
}

}
