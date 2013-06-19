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

#ifndef _XMLHelper_H_
#define _XMLHelper_H_

#include "../api.h"

#include <libxml/parser.h>
#include <libxml/xmlwriter.h>
#include <libxml/xmlschemas.h>

#include <string>
#include <map>
#include <sstream>

namespace avg {

std::string getXmlChildrenAsString(const xmlDocPtr xmlDoc, const xmlNodePtr& xmlNode);

void registerDTDEntityLoader(const std::string& sID, const std::string& sDTD);

class XMLParser
{
public:
    XMLParser();
    virtual ~XMLParser();

    void setSchema(const std::string& sSchema, const std::string& sSchemaName);
    void setDTD(const std::string& sDTD, const std::string& sDTDName);
    void parse(const std::string& sXML, const std::string& sXMLName);

    xmlDocPtr getDoc();
    xmlNodePtr getRootNode();

private:
    static void errorOutputFunc(void * ctx, const char * msg, ...);
    void internalErrorHandler(const char * msg, va_list args);

    void checkError(bool bError, const std::string& sXMLName);

    xmlSchemaParserCtxtPtr m_SchemaParserCtxt;
    xmlSchemaPtr m_Schema;
    xmlSchemaValidCtxtPtr m_SchemaValidCtxt;

    xmlDtdPtr m_DTD;
    xmlValidCtxtPtr m_DTDValidCtxt;
    
    xmlDocPtr m_Doc;

    std::string m_sError;
};

void validateXml(const std::string& sXML, const std::string& sSchema,
        const std::string& sXMLName, const std::string& sSchemaName);

}

#endif

