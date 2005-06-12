//
// $Id$
//

#include "CmdLine.h"

#include <iostream>

using namespace std;
 
namespace avg {

CmdLine::CmdLine(int argc, char **argv) 
{
    for (int i = 1; i < argc; ++i) {
        string sArg(argv[i]);
        if (sArg.substr(0, 2) == "--") {
            unsigned int DelimPos = sArg.find('=');
            string sOptName;
            string sOptVal;
            if (DelimPos == sArg.npos) {
                sOptName = sArg.substr(2);
                sOptVal = "";
            } else {
                sOptName = sArg.substr(2, DelimPos-2);
                sOptVal = sArg.substr(DelimPos+1);
            }
            m_Options[sOptName] = sOptVal;
        } else {
            m_Args.push_back(sArg);
        }
    }
}

const OptionMap& CmdLine::getOptions() const 
{
    return m_Options;
}

const string* CmdLine::getOption(const string& sName) const
{
    OptionMap::const_iterator it = m_Options.find(sName);
    if (it == m_Options.end()) {
        return 0;
    } else {
        return &(*it).second;
    }
}

int CmdLine::getNumArgs() const
{
    return m_Args.size();
}
 
const string* CmdLine::getArg(unsigned int i) const
{
    if (i>=m_Args.size()) {
        return 0;
    } else {
        return &m_Args[i];
    }
}

}

