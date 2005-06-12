//
// $Id$
// 

#ifndef _Exception_H_
#define _Exception_H_

#include <string>

#define AVG_ERR_UNKNOWN -1
#define AVG_ERR_XML_PARSE 1
#define AVG_ERR_XML_EMPTY 2
#define AVG_ERR_XML_NODE_UNKNOWN 3
#define AVG_ERR_XML_DUPLICATE_ID 4
#define AVG_ERR_VIDEO_INIT_FAILED 5
#define AVG_ERR_VIDEO_GENERAL 6
#define AVG_ERR_JS 7
#define AVG_ERR_JS_TYPE 8
#define AVG_ERR_DFB 9
#define AVG_ERR_FONT_INIT_FAILED 10
#define AVG_ERR_VIDEO_LOAD_FAILED 11
#define AVG_ERR_UNSUPPORTED 12
#define AVG_ERR_OPTION_SUBSYS_UNKNOWN 13
#define AVG_ERR_OPTION_UNKNOWN 14
#define AVG_ERR_FILEIO 15
#define AVG_ERR_NOT_IN_SCENE 16
#define AVG_ERR_OUT_OF_RANGE 17

namespace avg {
 
class Exception 
{
    public:
        Exception (int Code, const std::string& sErr);
        Exception (const Exception& ex);
        virtual ~Exception ();
        virtual int GetCode () const;
        virtual const std::string& GetStr () const;

	private:
        int m_Code;
        std::string m_sErr;
};

void fatalError(const std::string& sMsg);

}

#endif
