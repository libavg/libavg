//
// $Id$
// 

#ifndef _FileHelper_H_
#define _FileHelper_H_

#include <string>

std::string getPath(const std::string& Filename);

bool fileExists(const std::string& FileName);

std::string findFile (const std::string & Filename, const std::string & SearchPath);



#endif 

