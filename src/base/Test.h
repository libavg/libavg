//
// $Id$
//

#ifndef _Test_H_ 
#define _Test_H_

#include "CountedPointer.h"

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
    int m_IndentLevel;
    
private:
    bool m_bOk;
    int m_NumSucceeded;
    int m_NumFailed;
    std::string m_sName;
};

typedef CountedPointer<Test> TestPtr;

#define TEST(b)                            \
    cerr << string(m_IndentLevel+4, ' ') << "  TEST(" << #b << ")" << endl;  \
    test(b, __FILE__, __LINE__);
    
}
#endif

