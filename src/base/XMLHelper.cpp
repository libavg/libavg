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

#include "XMLHelper.h"

#include <iostream>

using namespace std;

namespace avg {

void xmlAttrToBool(const xmlNodePtr& xmlNode, const char * attr,
        bool * pBool)
{
    char * retStr = (char *)xmlGetProp(xmlNode, (const xmlChar *)attr);
    if (retStr) {
        // avg usually wants xml attributes in lowercase, but python only
        // sees 'True' as true, so we'll accept that too. Also, python 2.3
        // has 1 as true, so that has to be ok too.
        *pBool = !strcmp(retStr, "True") || !strcmp(retStr, "true") 
                || !strcmp(retStr, "1");
        xmlFree(retStr);
    }    
}

void xmlAttrToInt(const xmlNodePtr& xmlNode, const char * attr,
        int * pInt)
{
    char * retStr = (char *)xmlGetProp(xmlNode, (const xmlChar *)attr);
    if (retStr) {
        char * errStr;
        int ret = strtol(retStr, &errStr, 10);
        if (*errStr == 0) {
            *pInt = ret;
        }
        xmlFree(retStr);
    }
}

void xmlAttrToDouble(const xmlNodePtr& xmlNode, const char * attr,
        double * pDouble)
{
    char * retStr = (char *)xmlGetProp(xmlNode, (const xmlChar *)attr);
    if (retStr) {
        float ret;
        int ok = sscanf (retStr, "%f", &ret);
        if (ok == 1) {
            *pDouble = ret;
        } 
        xmlFree(retStr);
    }
}

void xmlAttrToString(const xmlNodePtr& xmlNode, const char * attr,
        string * pString)
{
    char * retStr = (char *)xmlGetProp(xmlNode, (const xmlChar *)attr);
    if (retStr) {
        *pString = retStr;
        xmlFree(retStr);
    }
}

string getXmlChildrenAsString(const xmlDocPtr xmlDoc, 
        const xmlNodePtr& xmlNode)
{
    string s;
    xmlBufferPtr pBuffer = xmlBufferCreate();
    xmlNodeDump(pBuffer, xmlDoc, xmlNode, 0, 0);

    s = (const char *)xmlBufferContent(pBuffer);
    int StartPos = s.find('>')+1;
    int EndPos = s.rfind('<')-1;
    s = s.substr(StartPos, EndPos-StartPos+1);
    xmlBufferFree(pBuffer);
    return s;
}

bool getDefaultedBoolAttr (const xmlNodePtr& xmlNode, 
        const char * attr, bool def)
{
    bool RVal = def;
    xmlAttrToBool(xmlNode, attr, &RVal);
    return RVal;
}

int getDefaultedIntAttr (const xmlNodePtr& xmlNode, 
        const char * attr, int def)
{
    int RVal = def;
    xmlAttrToInt(xmlNode, attr, &RVal);
    return RVal;
}

double getDefaultedDoubleAttr (const xmlNodePtr& xmlNode, 
       const char * attr, double def)
{
    double RVal = def;
    xmlAttrToDouble(xmlNode, attr, &RVal);
    return RVal;
}

double getRequiredDoubleAttr (const xmlNodePtr& xmlNode, 
       const char * attr)
{
    double RVal;
    xmlAttrToDouble(xmlNode, attr, &RVal);
    return RVal;
}

string getDefaultedStringAttr (const xmlNodePtr& xmlNode, 
       const char * attr, const string & def)
{
    string RVal = def;
    xmlAttrToString(xmlNode, attr, &RVal);
    return RVal;
}

string getRequiredStringAttr (const xmlNodePtr& xmlNode, 
       const char * attr)
{
    char * retStr = (char *)xmlGetProp (xmlNode, (const xmlChar *)attr);
    string s(retStr);
    xmlFree(retStr);
    return s;
}

}
