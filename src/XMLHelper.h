//
// $Id$
// 

#ifndef _XMLHelper_H_
#define _XMLHelper_H_

#include <libxml/parser.h>
#include <string>

bool getDefaultedBoolAttr (const xmlNodePtr& xmlNode, 
        const char * attr, bool def);

int getDefaultedIntAttr (const xmlNodePtr& xmlNode, 
        const char * attr, int def);

double getDefaultedDoubleAttr (const xmlNodePtr& xmlNode, 
       const char * attr, double def);

std::string getDefaultedStringAttr (const xmlNodePtr& xmlNode, 
       const char * attr, const std::string & def);

std::string getRequiredStringAttr (const xmlNodePtr& xmlNode, 
       const char * attr);

#endif //_XMLHelper_H_

