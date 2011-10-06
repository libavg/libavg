//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2011 Ulrich von Zadow
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

#include "TestSuite.h"
#include "Exception.h"

using namespace std;

namespace avg {

TestSuite::TestSuite(const string& sName)
    : Test(sName, 0)
{
}

TestSuite::~TestSuite()
{
}

void TestSuite::addTest(TestPtr pNewTest)
{
    m_Tests.push_back(pNewTest);
}

void TestSuite::runTests()
{
    cerr << string(m_IndentLevel, ' ') << "Running suite " << getName() << endl;
    for (unsigned i = 0; i < m_Tests.size(); ++i) {
        cerr << string(m_IndentLevel, ' ') << "  Running " 
                << m_Tests[i]->getName() << endl;
        try {
            m_Tests[i]->runTests();
            aggregateStatistics(*m_Tests[i]);
            m_Tests[i]->printResults();
        } catch (Exception& ex) {
            cerr << string(m_IndentLevel, ' ') << ex.getStr() << endl;
            setFailed();
        } catch (...) {
            cerr << string(m_IndentLevel, ' ') <<
                "    ---->> failed, exception caught" << endl;
            setFailed();
        }
    }
    
    printResults();
}


}

