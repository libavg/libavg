//
// $Id$
//

#include "Test.h"

#include <iostream>

using namespace std;

namespace avg {

Test::Test()
    : m_bOk(true),
      m_NumSucceeded(0),
      m_NumFailed(0)
{
}

Test::~Test()
{
}

void Test::test (bool b, const char * pszFile, int Line)
{
    if (b) {
        m_NumSucceeded++;
    } else {
        cerr << "    ---->> failed at " << pszFile << ", " << Line << endl;
        m_bOk = false;
        m_NumFailed++;
    }
}


bool Test::isOk ()
{
    return m_bOk;
}

void Test::setFailed ()
{
    m_NumFailed++;
    m_bOk = false;
}

int Test::getNumSucceeded () const
{
    return m_NumSucceeded;
}

int Test::getNumFailed() const
{
    return m_NumFailed;
}

void Test::aggregateStatistics (const Test& ChildTest)
{
    m_NumSucceeded += ChildTest.getNumSucceeded();
    m_NumFailed += ChildTest.getNumFailed();
}

void Test::printResults()
{
    if (m_NumFailed == 0) {
        cerr << "Test succeeded." << endl;
    } else {
        cerr << "######## Test failed. ########" << endl;
    }
        
}

}

