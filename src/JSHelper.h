//
// $Id$
// 

#ifndef _JSHelper_H_
#define _JSHelper_H_

#include <prenv.h>
#include <jsapi.h>

bool getJSFileLine(JSContext *cx, 
        const char * & filename, int & lineno);

#endif 
