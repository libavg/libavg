//
// $Id$
//

#include "TestSuite.h"

using namespace std;

namespace avg {

TestSuite::TestSuite(const string& sName)
    : Test(sName, 0)
{
}

TestSuite::~TestSuite()
{
}

void TestSuite::addTest(TestPtr newTest)
{
    m_Tests.push_back(newTest);
}

void TestSuite::runTests()
{
    cerr << string(m_IndentLevel, ' ') << "Running suite " << getName() << endl;
    for (int i=0; i<m_Tests.size(); ++i) {
        cerr << string(m_IndentLevel, ' ') << "  Running " 
                << m_Tests[i]->getName() << endl;
        m_Tests[i]->runTests();
        aggregateStatistics(*m_Tests[i]);
        m_Tests[i]->printResults();
    }
    
    printResults();
}


}

