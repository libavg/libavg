//
// $Id$
//

#include "AVGJSScript.h"

#include <iostream>

using namespace std;


AVGJSScript::AVGJSScript(const std::string& code, const std::string& filename, 
        int lineno, JSContext * pJSContext)
    : m_code(code),
      m_filename(filename),
      m_lineno(lineno),
      m_pJSContext(pJSContext),
      m_pScript(0)
{
}

AVGJSScript::~AVGJSScript()
{
    if (m_pScript) {
        JS_DestroyScript(m_pJSContext, m_pScript);
    }
}

extern "C" {
// From mozilla/js/jsexn.h
extern JSBool
js_ReportUncaughtException(JSContext *cx);
}

void AVGJSScript::run()
{
    // TODO: Actually, compile() should be called in the constructor. However, if I do this, 
    // any arguments in the script are null when execute is finally called.
    compile();
    if (m_pScript) {
        jsval Result;
        JSBool ok = JS_ExecuteScript(m_pJSContext, m_pObject, m_pScript, &Result);
        if (!ok) {
            js_ReportUncaughtException(m_pJSContext); 
        }
    }
    
}
        
void AVGJSScript::compile()
{
    m_pObject = JS_GetGlobalObject(m_pJSContext);
    m_pScript = JS_CompileScript(m_pJSContext, m_pObject,
            m_code.c_str(), m_code.length(), m_filename.c_str(), m_lineno);
    if (!m_pScript) {
        js_ReportUncaughtException(m_pJSContext);
    } 
}

