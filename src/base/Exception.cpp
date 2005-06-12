//
// $Id$
// 

#include "Exception.h"
#include "Logger.h"

using namespace std;

namespace avg {

Exception::Exception (int Code, const std::string& sErr)
    : m_Code (Code),
      m_sErr (sErr)
{
}

Exception::Exception (const Exception& ex)
    : m_Code (ex.GetCode ()),
      m_sErr (ex.GetStr ())
{
}


Exception::~Exception()
{
}


int Exception::GetCode () const
{
    return m_Code;
}

const string& Exception::GetStr () const
{
    return m_sErr;
}

void fatalError(const std::string& sMsg)
{
    AVG_TRACE(Logger::ERROR, "Internal error: "+sMsg+" Aborting.");
    exit(-1);
}


}

