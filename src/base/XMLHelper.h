//
// $Id$
//

#ifndef _XMLHelper_H_
#define _XMLHelper_H_

#include <libxml/parser.h>
#include <string>

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

double getDefaultedDoubleAttr (const xmlNodePtr& xmlNode,
       const char * attr, double def);

double getRequiredDoubleAttr (const xmlNodePtr& xmlNode,
       const char * attr);

std::string getDefaultedStringAttr (const xmlNodePtr& xmlNode,
       const char * attr, const std::string & def);

std::string getRequiredStringAttr (const xmlNodePtr& xmlNode,
       const char * attr);

}

#endif //_XMLHelper_H_

