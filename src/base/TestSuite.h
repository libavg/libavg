//
// $Id$
//

#ifndef _TestSuite_H_ 
#define _TestSuite_H_

#include "Test.h"

#include <iostream>
#include <sstream>
#include <vector>

namespace avg {
class TestSuite: public Test
{
public:
    TestSuite(const std::string& sName);
    virtual ~TestSuite();

    void addTest(TestPtr newTest);

    virtual void runTests ();

private:
    std::vector<TestPtr> m_Tests;
};
    
}
#endif

