//=============================================================================
//
// Original code Copyright (C) 2003, ART+COM AG Berlin
//
// Released under LGPL.
//
//=============================================================================
//
//   $RCSfile$
//   $Author$
//   $Revision$
//   $Date$
//
//=============================================================================

#include "acIJSContextPublisher.h"

#include <jsapi.h>

#define ACJSCONTEXTPUBLISHER_CID \
{ 0x389811f1, 0x4159, 0x403f, { 0x9f, 0x94, 0x33, 0x73, 0x54, 0xcd, 0xf1, 0x9a } }

#define ACJSCONTEXTPUBLISHER_CONTRACTID "@artcom.com/jscontextpublisher;1"

class acJSContextPublisher : public acIJSContextPublisher {
    public:
        NS_DECL_ISUPPORTS;
        NS_DECL_ACIJSCONTEXTPUBLISHER;

        acJSContextPublisher();
        virtual ~acJSContextPublisher();
    private:
        static JSContext * _myContext_;

};
