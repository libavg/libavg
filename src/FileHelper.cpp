//
// $Id$
//

#include "FileHelper.h"

#include <libgen.h>
#include <stdio.h>
#include <sys/stat.h>

using namespace std;
 
string getPath(const string& Filename)
{
    if (Filename.at(Filename.length()-1) == '/') {
        return Filename;
    }
    char * pszBuffer = strdup(Filename.c_str());

    string DirName(dirname(pszBuffer));
    free(pszBuffer);

    DirName += "/";
    
    return DirName;
}

bool fileExists(const std::string& FileName) {
    struct stat64 myStat;
    return stat64(FileName.c_str(), &myStat) != -1;
}

string findFile (const string & Filename, const string & SearchPath)
{
    string SearchPathLeft = SearchPath;
    static const char * pDelimiters = ";:";

    string::size_type EndPos = SearchPathLeft.find_first_of(pDelimiters);

    while (EndPos != std::string::npos) {        
        string Path = SearchPathLeft.substr(0, EndPos);

        if (Path.at(Path.size() - 1) != '/') {
            Path += "/";
        }
        string FileWithPath = Path + Filename;            
        if (fileExists(FileWithPath)) {
            return FileWithPath;
        }

        SearchPathLeft = SearchPathLeft.substr(EndPos + 1);
        EndPos = SearchPathLeft.find_first_of(pDelimiters);
    }                

    if (SearchPathLeft.size() && SearchPathLeft.at(SearchPathLeft.size() - 1) != '/') {
        SearchPathLeft += "/";
    }

    string FileWithPath = SearchPathLeft + Filename;
    if (fileExists(FileWithPath)) {
        return FileWithPath;
    }

    return "";
}

