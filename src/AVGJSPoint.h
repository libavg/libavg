//
// $Id$
// 

#ifndef _AVGJSPoint_H_
#define _AVGJSPoint_H_

#include "AVGPoint.h"
#include "IAVGJSPoint.h"

//41bd1be6-e7c6-453c-a13b-811becd81837
#define AVGJSPOINT_CID \
{ 0x41bd1be6, 0xe7c6, 0x453c, { 0xa1, 0x3b, 0x81, 0x1b, 0xec, 0xd8, 0x18, 0x37 } }

#define AVGJSPOINT_CONTRACTID "@c-base.org/avgjspoint;1"

class AVGJSPoint: public IAVGJSPoint, public AVGDPoint
{
    public:
        NS_DECL_ISUPPORTS
        NS_DECL_IAVGJSPOINT

        AVGJSPoint ();
        virtual ~AVGJSPoint ();

        AVGJSPoint & operator =(const AVGJSPoint & Pt);
        AVGJSPoint & operator =(const AVGDPoint & Pt);
};

#endif //_AVGJSPoint_H_

