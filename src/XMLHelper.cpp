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
    const char * retStr = (const char *)xmlGetProp (xmlNode, attr);
    if (!retStr) {
        return def;
    } else {
        return (!strcmp(retStr, "true"));
    }
}

int getDefaultedIntAttr (const xmlNodePtr& xmlNode, 
        const xmlChar * attr, int def)
{
    const char * retStr = (const char *)xmlGetProp (xmlNode, attr);
    if (!retStr) {
        return def;
    } else {
        char * errStr;
        int ret = strtol (retStr, &errStr, 10);
        if (*errStr == 0) {
            return ret;
        } else {
            return def;
        }
    }
}

double getDefaultedDoubleAttr (const xmlNodePtr& xmlNode, 
       const xmlChar * attr, double def)
{
    const char * retStr = (const char *)xmlGetProp (xmlNode, attr);
    if (!retStr) {
        return def;
    } else {
        float ret;
        int ok = sscanf (retStr, "%f", &ret);
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
    const char * retStr = (const char *)xmlGetProp (xmlNode, attr);
    if (!retStr) {
        return def;
    } else {
        return retStr;
    }
}

string getRequiredStringAttr (const xmlNodePtr& xmlNode, 
       const xmlChar * attr)
{
    const char * retStr = (const char *)xmlGetProp (xmlNode, attr);
    PLASSERT(retStr);  // This should have been found during validation.
    return retStr;
}

