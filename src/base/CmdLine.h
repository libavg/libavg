//
// $Id$
// 

#ifndef _CmdLine_H_
#define _CmdLine_H_

#include <string>
#include <vector>
#include <map>

namespace avg {

typedef std::map<std::string, std::string> OptionMap;

class CmdLine {
public:
    CmdLine(int argc, char **argv);

    const OptionMap& getOptions() const;
    const std::string* getOption(const std::string& sName) const;
    int getNumArgs() const;
    const std::string* getArg(unsigned int i) const;

private:
    OptionMap m_Options;
    std::vector<std::string> m_Args;
};

}
#endif 

