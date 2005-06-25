//
// $Id$
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
        *pBool = !strcmp(retStr, "true");
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
