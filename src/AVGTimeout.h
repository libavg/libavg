//
// $Id$
//

#ifndef _AVGTimeout_H_
#define _AVGTimeout_H_

#include "AVGJSScript.h"

#include <xpcom/nsCOMPtr.h>
#include <jsapi.h>

#include <string>

class AVGTimeout
{
    public:
        AVGTimeout (int time, std::string code, bool isInterval, JSContext * pContext);
        virtual ~AVGTimeout ();

        bool IsReady() const;
        bool IsInterval() const;
        void Fire(JSContext * pJSContext);
        int GetID() const;
        bool operator <(const AVGTimeout& other) const;

    private:
        int m_Interval;
        int m_NextTimeout;
        std::string m_Code;
        bool m_IsInterval;
        int m_ID;
        static int s_LastID;
        AVGJSScript m_Script;
};

#endif //_AVGTimeout_H_
