//
// $Id$
// 

#include "XMLHelper.h"

#include <paintlib/pldebug.h>

#include <iostream>

using namespace std;

bool getDefaultedBoolAttr (const xmlNodePtr& xmlNode, 
        const xmlChar * attr, bool def)
{
    char * retStr = (char *)xmlGetProp (xmlNode, attr);
    if (!retStr) {
        return def;
    } else {
        bool b = !strcmp(retStr, "true");
        xmlFree(retStr);
        return b;
    }
}

int getDefaultedIntAttr (const xmlNodePtr& xmlNode, 
        const xmlChar * attr, int def)
{
    char * retStr = (char *)xmlGetProp (xmlNode, attr);
    if (!retStr) {
        return def;
    } else {
        char * errStr;
        int ret = strtol (retStr, &errStr, 10);
        if (*errStr == 0) {
            xmlFree(retStr);
            return ret;
        } else {
            xmlFree(retStr);
            return def;
        }
    }
}

double getDefaultedDoubleAttr (const xmlNodePtr& xmlNode, 
       const xmlChar * attr, double def)
{
    char * retStr = (char *)xmlGetProp (xmlNode, attr);
    if (!retStr) {
        return def;
    } else {
        float ret;
        int ok = sscanf (retStr, "%f", &ret);
        xmlFree(retStr);
        if (ok == 1) {
            return ret;
        } else {
            return def;
        }
    }
}

string getDefaultedStringAttr (const xmlNodePtr& xmlNode, 
       const xmlChar * attr, const string & def)
{
    char * retStr = (char *)xmlGetProp (xmlNode, attr);
    if (!retStr) {
        return def;
    } else {
        string s(retStr);
        xmlFree(retStr);
        return s;
    }
}

string getRequiredStringAttr (const xmlNodePtr& xmlNode, 
       const xmlChar * attr)
{
    char * retStr = (char *)xmlGetProp (xmlNode, attr);
    string s(retStr);
    xmlFree(retStr);
    return s;
}

