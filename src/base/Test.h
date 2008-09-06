//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2008 Ulrich von Zadow
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  Current versions can be found at www.libavg.de
//

#ifndef _Test_H_ 
#define _Test_H_

#include <boost/shared_ptr.hpp>

#include <iostream>
#include <sstream>
#include <string>

namespace avg {
class Test
{
public:
    Test(const std::string & sName, int IndentLevel);
    virtual ~Test();

    bool isOk();
    virtual void runTests() = 0;

    void test(bool b, const char * pszFile, int Line);
    void setFailed();

    int getNumSucceeded() const;
    int getNumFailed() const;
    const std::string& getName() const;

    void aggregateStatistics(const Test& ChildTest);
    virtual void printResults();

protected:
    static const std::string& getSrcDirName();
    static std::string s_sSrcDirName;

    int m_IndentLevel;

private:
    bool m_bOk;
    int m_NumSucceeded;
    int m_NumFailed;
    std::string m_sName;
};

typedef boost::shared_ptr<Test> TestPtr;

#define TEST_FAILED(s)                     \
    cerr << string(m_IndentLevel+6, ' ') << s << endl;  \
    test(false, __FILE__, __LINE__);

#define TEST(b)                            \
    cerr << string(m_IndentLevel+4, ' ') << "  TEST(" << #b << ")" << endl;  \
    test(b, __FILE__, __LINE__);

#define QUIET_TEST(b)                      \
    if(!b) {                               \
        cerr << string(m_IndentLevel+4, ' ') << "  TEST(" << #b << ")" << endl;  \
    }                                      \
    test(b, __FILE__, __LINE__);
}
#endif

