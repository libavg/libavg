//
// $Id$
// 

#ifndef _AVGException_H_
#define _AVGException_H_

#include <string>

#define AVG_ERR_XML_PARSE 1
#define AVG_ERR_XML_EMPTY 2
#define AVG_ERR_XML_NODE_UNKNOWN 3
#define AVG_ERR_XML_DUPLICATE_ID 4
#define AVG_ERR_VIDEO_INIT_FAILED 5
#define AVG_ERR_VIDEO_GENERAL 6
#define AVG_ERR_NO_KRUECKE 7

class AVGException 
{
	public:
        AVGException (int Code, const std::string& sErr);
        AVGException (const AVGException& ex);
        virtual ~AVGException ();
		virtual int GetCode () const;
		virtual const std::string& GetStr () const;

	private:
        int m_Code;
        std::string m_sErr;
};

#endif //_AVGException_H_

