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
#include "Logger.h"

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


XMLParser::XMLParser()
    : m_SchemaParserCtxt(0),
      m_Schema(0),
      m_SchemaValidCtxt(0),
      m_DTD(0),
      m_DTDValidCtxt(0),
      m_Doc(0)
{
    xmlPedanticParserDefault(1);
    xmlSetGenericErrorFunc(this, errorOutputFunc);
    xmlDoValidityCheckingDefaultValue = 0;
}

XMLParser::~XMLParser()
{
    if (m_Schema) {
        xmlSchemaFree(m_Schema);
    }
    if (m_SchemaParserCtxt) {
        xmlSchemaFreeParserCtxt(m_SchemaParserCtxt);
    }
    if (m_SchemaValidCtxt) {
        xmlSchemaFreeValidCtxt(m_SchemaValidCtxt);
    }
    if (m_DTD) {
        xmlFreeDtd(m_DTD);
    }
    if (m_DTDValidCtxt) {
        xmlFreeValidCtxt(m_DTDValidCtxt);
    }
    if (m_Doc) {
        xmlFreeDoc(m_Doc);
    }
    xmlSetGenericErrorFunc(0, 0);
}

void XMLParser::setSchema(const string& sSchema, const string& sSchemaName)
{
    AVG_ASSERT(!m_SchemaParserCtxt);
    AVG_ASSERT(!m_Schema);
    AVG_ASSERT(!m_SchemaValidCtxt);
    AVG_ASSERT(!m_DTD);
    AVG_ASSERT(!m_DTDValidCtxt);

    m_SchemaParserCtxt = xmlSchemaNewMemParserCtxt(sSchema.c_str(), sSchema.length());
    checkError(!m_SchemaParserCtxt, sSchemaName);

    m_Schema = xmlSchemaParse(m_SchemaParserCtxt);
    checkError(!m_Schema, sSchemaName);

    m_SchemaValidCtxt = xmlSchemaNewValidCtxt(m_Schema);
    checkError(!m_SchemaValidCtxt, sSchemaName);
}

void XMLParser::setDTD(const std::string& sDTD, const std::string& sDTDName)
{
    AVG_ASSERT(!m_SchemaParserCtxt);
    AVG_ASSERT(!m_Schema);
    AVG_ASSERT(!m_SchemaValidCtxt);
    AVG_ASSERT(!m_DTD);
    AVG_ASSERT(!m_DTDValidCtxt);

    registerDTDEntityLoader("memory.dtd", sDTD.c_str());
    string sDTDFName = "memory.dtd";
    m_DTD = xmlParseDTD(NULL, (const xmlChar*) sDTDFName.c_str());
    checkError(!m_DTD, sDTDName);

    m_DTDValidCtxt = xmlNewValidCtxt();
    checkError(!m_DTDValidCtxt, sDTDName);
    m_DTDValidCtxt->error = xmlParserValidityError;
    m_DTDValidCtxt->warning = xmlParserValidityWarning;
}

void XMLParser::parse(const string& sXML, const string& sXMLName)
{
    if (m_Doc) {
        xmlFreeDoc(m_Doc);
    }
    m_Doc = xmlParseMemory(sXML.c_str(), int(sXML.length()));
    checkError(!m_Doc, sXMLName);

    bool bOK = true;
    if (m_SchemaValidCtxt) {
        int err = xmlSchemaValidateDoc(m_SchemaValidCtxt, m_Doc);
        AVG_ASSERT(err != -1);
        bOK = (err == 0);
    }
    if (m_DTD) {
        int err = xmlValidateDtd(m_DTDValidCtxt, m_Doc, m_DTD);
        bOK = (err != 0);
    }
    if (!bOK) {
        xmlFreeDoc(m_Doc);
        m_Doc = 0;
        checkError(true, sXMLName);
    }
}

xmlDocPtr XMLParser::getDoc()
{
    AVG_ASSERT(m_Doc);
    return m_Doc;
}

xmlNodePtr XMLParser::getRootNode()
{
    AVG_ASSERT(m_Doc);
    return xmlDocGetRootElement(m_Doc);
}

void XMLParser::errorOutputFunc(void * ctx, const char * msg, ...)
{
    va_list args;
    va_start(args, msg);
    ((XMLParser*)ctx)->internalErrorHandler(msg, args);
    va_end(args);
}

void XMLParser::internalErrorHandler(const char * msg, va_list args)
{
    char psz[1024];
    vsnprintf(psz, 1024, msg, args);
    m_sError += psz;
}

void XMLParser::checkError(bool bError, const string& sXMLName)
{
    if (bError) {
        string sError = "Error parsing "+sXMLName+".\n";
        sError += m_sError;
        m_sError = "";
        throw (Exception(AVG_ERR_XML_PARSE, sError));
    }
}

void validateXml(const string& sXML, const string& sSchema, const string& sXMLName,
        const string& sSchemaName)
{
    XMLParser parser;
    parser.setSchema(sSchema, sSchemaName);

    parser.parse(sXML, sXMLName);
}

}
