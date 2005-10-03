//
// $Id$
//

#include "Test.h"

#include <iostream>

using namespace std;

namespace avg {

Test::Test(const string & sName, int IndentLevel)
    : m_bOk(true),
      m_NumSucceeded(0),
      m_NumFailed(0),
      m_IndentLevel(IndentLevel),
      m_sName(sName)
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
        cerr << string(m_IndentLevel, ' ') << "    ---->> failed at " << pszFile
                << ", " << Line << endl;
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

const std::string& Test::getName() const
{
    return m_sName;
}

void Test::aggregateStatistics (const Test& ChildTest)
{
    m_NumSucceeded += ChildTest.getNumSucceeded();
    m_NumFailed += ChildTest.getNumFailed();
}

void Test::printResults()
{
    if (m_NumFailed == 0) {
        cerr << string(m_IndentLevel, ' ') << m_sName << " succeeded." << endl;
    } else {
        cerr << string(m_IndentLevel, ' ') << "######## " << m_sName << 
            " failed. ########" << endl;
    }
        
}

}

