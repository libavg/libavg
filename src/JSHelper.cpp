//
// $Id$
// 

#include "JSHelper.h"

#include <jsdbgapi.h>

using namespace std;


JSStackFrame * getStackFrame(int i, JSContext *cx) {
    JSStackFrame* fp;
    JSStackFrame* iter = 0;
    int num = 0;

    while(0 != (fp = JS_FrameIterator(cx, &iter)))
    {
        if (num == i) {
            return fp;
        }
        ++num;
    }
    return 0;
}

bool
getJSFileLine(JSContext *cx, 
        const char * & filename, int & lineno) 
{
    JSStackFrame * fp = getStackFrame(1,cx);

    if (fp) {
        if(!JS_IsNativeFrame(cx, fp)) {
            JSScript* script = JS_GetFrameScript(cx, fp);
            jsbytecode* pc = JS_GetFramePC(cx, fp);
            if(script && pc)
            {
                filename = JS_GetScriptFilename(cx, script);
                lineno =  (PRInt32) JS_PCToLineNumber(cx, script, pc);
                return true;
            }
        }
    }
    return false;
}



