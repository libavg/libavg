//
// $Id$
//

#ifndef _AVGJSScript_H_
#define _AVGJSScript_H_

#include <xpcom/nsCOMPtr.h>
#include <jsapi.h>

#include <string>

class AVGJSScript
{
    public:
        AVGJSScript (const std::string& code, const std::string& filename, int lineno,
                JSContext * pJSContext);
        virtual ~AVGJSScript ();

        void run();
        
    private:
        void compile();

        std::string m_code;
        std::string m_filename;
        int m_lineno;
        
        JSContext * m_pJSContext;
        JSObject * m_pObject;
        JSScript * m_pScript;
};

#endif //_AVGJSScript_H_
