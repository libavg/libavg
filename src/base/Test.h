//
// $Id$
//

#ifndef _Test_H_ 
#define _Test_H_

#include <iostream>
#include <sstream>

namespace avg {
class Test
{
public:
    Test();
    virtual ~Test();

    bool isOk ();
    virtual void runTests () = 0;

    void test (bool b, const char * pszFile, int Line);
    void setFailed ();

    int getNumSucceeded () const;
    int getNumFailed () const;

    void aggregateStatistics (const Test& ChildTest);
    void printResults();

private:
    bool m_bOk;
    int m_NumSucceeded;
    int m_NumFailed;

};

#define TEST(b)                            \
    cerr << "  TEST(" << #b << ")" << endl;  \
    test(b, __FILE__, __LINE__);
    
}
#endif

