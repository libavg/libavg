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

#include "Test.h"

#include "../base/OSHelper.h"

#include <iostream>
#include <stdlib.h>

using namespace std;

namespace avg {

string Test::s_sSrcDirName;

Test::Test(const string & sName, int indentLevel)
    : m_IndentLevel(indentLevel),
      m_NumSucceeded(0),
      m_NumFailed(0),
      m_sName(sName)
{
}

Test::~Test()
{
}

void Test::test (bool b, const char * pszFile, int line)
{
    if (b) {
        m_NumSucceeded++;
    } else {
        cerr << string(m_IndentLevel, ' ') << "    ---->> failed at " << pszFile
                << ", " << line << endl;
        m_NumFailed++;
    }
}


bool Test::isOk()
{
    return m_NumFailed == 0;
}

void Test::setFailed()
{
    m_NumFailed++;
}

int Test::getNumSucceeded() const
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

void Test::aggregateStatistics(const Test& ChildTest)
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

const string& Test::getSrcDirName()
{
    if (s_sSrcDirName == "") {
        bool bInEnviron = getEnv("srcdir", s_sSrcDirName);
        if (!bInEnviron) {
            s_sSrcDirName = ".";
        }
        s_sSrcDirName += "/";
    }
    return s_sSrcDirName;
}

}

