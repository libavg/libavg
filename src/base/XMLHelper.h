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
//

#ifndef _XMLHelper_H_
#define _XMLHelper_H_

#include <libxml/parser.h>
#include <libxml/xmlwriter.h>

#include <string>
#include <map>
#include <sstream>

namespace avg {

void xmlAttrToBool(const xmlNodePtr& xmlNode, const char * attr,
        bool * pBool);

void xmlAttrToInt(const xmlNodePtr& xmlNode, const char * attr,
        int * pInt);

void xmlAttrToDouble(const xmlNodePtr& xmlNode, const char * attr,
        double * pDouble);

void xmlAttrToString(const xmlNodePtr& xmlNode, const char * attr,
        std::string * pString);

std::string getXmlChildrenAsString(const xmlDocPtr xmlDoc,
        const xmlNodePtr& xmlNode);

bool getDefaultedBoolAttr (const xmlNodePtr& xmlNode,
        const char * attr, bool def);

int getDefaultedIntAttr (const xmlNodePtr& xmlNode,
        const char * attr, int def);

int getRequiredIntAttr (const xmlNodePtr& xmlNode,
       const char * attr);

double getDefaultedDoubleAttr (const xmlNodePtr& xmlNode,
       const char * attr, double def);

double getRequiredDoubleAttr (const xmlNodePtr& xmlNode,
       const char * attr);

std::string getDefaultedStringAttr (const xmlNodePtr& xmlNode,
       const char * attr, const std::string & def);

std::string getRequiredStringAttr (const xmlNodePtr& xmlNode,
       const char * attr);

void writeMinMaxXMLNode(xmlTextWriterPtr writer, std::string sName, double Val[2]);

template<class T>
void writeAttribute(xmlTextWriterPtr writer, std::string sName, T Value)
{
    int rc;
    std::stringstream strs;
    strs << Value;
    rc = xmlTextWriterWriteAttribute(writer, BAD_CAST sName.c_str(), 
            BAD_CAST strs.str().c_str());
}

template<class T>
void writeSimpleXMLNode(xmlTextWriterPtr writer, std::string sName, T Value)
{
    int rc;
    rc = xmlTextWriterStartElement(writer, BAD_CAST sName.c_str());
    writeAttribute(writer, "value", Value);
    rc = xmlTextWriterEndElement(writer);
}
        
void registerDTDEntityLoader(const std::string& sID, const std::string& sDTD);

}

#endif //_XMLHelper_H_

