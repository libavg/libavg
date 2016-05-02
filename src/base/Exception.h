//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2014 Ulrich von Zadow
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

#ifndef _Exception_H_
#define _Exception_H_

#include "../api.h"
#include <string>
#include <exception>

#define AVG_ERR_UNKNOWN -1
#define AVG_ERR_XML_PARSE 1
#define AVG_ERR_XML_VALID 2
#define AVG_ERR_XML_EMPTY 3
#define AVG_ERR_XML_NODE_UNKNOWN 4
#define AVG_ERR_XML_DUPLICATE_ID 5
#define AVG_ERR_VIDEO_INIT_FAILED 6
#define AVG_ERR_VIDEO_GENERAL 7
#define AVG_ERR_FONT_INIT_FAILED 10
#define AVG_ERR_VIDEO_LOAD_FAILED 11
#define AVG_ERR_UNSUPPORTED 12
#define AVG_ERR_OPTION_SUBSYS_UNKNOWN 13
#define AVG_ERR_OPTION_UNKNOWN 14
#define AVG_ERR_FILEIO 15
#define AVG_ERR_NOT_IN_SCENE 16
#define AVG_ERR_OUT_OF_RANGE 17
#define AVG_ERR_ALREADY_CONNECTED 18
#define AVG_ERR_LOAD_DURING_PLAYBACK 19
#define AVG_ERR_CANT_PARSE_STRING 20
#define AVG_ERR_INVALID_CAPTURE 21

#define AVG_ERR_NO_NODE 23
#define AVG_ERR_NO_ARG 24
#define AVG_ERR_INVALID_ARGS 25
#define AVG_ERR_NO_BUILDER 26
#define AVG_ERR_TYPE 27
#define AVG_ERR_CORRUPT_PLUGIN 28
#define AVG_ERR_CAMERA_FATAL 29
#define AVG_ERR_CAMERA_NONFATAL 30
#define AVG_ERR_DEPRECATED 31
#define AVG_ERR_ASSERT_FAILED 32
#define AVG_ERR_MT_INIT 33

namespace avg {
 
class AVG_API Exception: public std::exception
{
    public:
        Exception(int code, const std::string& sErr = "");
        Exception(const Exception& ex);
        virtual ~Exception() throw();
        virtual int getCode() const;
        virtual const std::string& getStr() const;
        virtual const char* what() const throw();

    private:
        int m_Code;
        std::string m_sErr;
};

void AVG_API debugBreak();
void AVG_API avgAssert(bool b, const char * pszFile, int line, const char * pszReason=0);

#define AVG_ASSERT(b) avgAssert((b) != 0, __FILE__, __LINE__);
#define AVG_ASSERT_MSG(b, pszReason) avgAssert((b) != 0, __FILE__, __LINE__, pszReason);

}

#endif
