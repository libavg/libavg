//
// $Id$
// 

#include "AVGException.h"

using namespace std;

AVGException::AVGException (int Code, const std::string& sErr)
    : m_Code (Code),
      m_sErr (sErr)
{
}

AVGException::AVGException (const AVGException& ex)
    : m_Code (ex.GetCode ()),
      m_sErr (ex.GetStr ())
{
}


AVGException::~AVGException()
{
}


int AVGException::GetCode () const
{
    return m_Code;
}

const string& AVGException::GetStr () const
{
    return m_sErr;
}


