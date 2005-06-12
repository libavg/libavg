//
// $Id$
// 

#ifndef _FileHelper_H_
#define _FileHelper_H_

#include <string>

namespace avg {
    
std::string getPath(const std::string& Filename);

bool fileExists(const std::string& FileName);

std::string findFile(const std::string & sFilename,
        const std::string & sPathOptionSubsys,
        const std::string & sPathOptionName, 
        const std::string & sCurJSFilename);

void readWholeFile(const std::string& sFilename, std::string& sContents);

void writeWholeFile(const std::string& sFilename, const std::string& sContent);

}

#endif 

