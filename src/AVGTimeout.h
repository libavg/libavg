//
// $Id$
//

#ifndef _AVGTimeout_H_
#define _AVGTimeout_H_

#include <string>

class IJSEvalKruecke;

class AVGTimeout
{
    public:
        AVGTimeout (int time, std::string code, bool isInterval);
        virtual ~AVGTimeout ();

        bool IsReady() const;
        bool IsInterval() const;
        void Fire(IJSEvalKruecke * pKruecke);
        int GetID() const;
        bool operator <(const AVGTimeout& other) const;

    private:
        int m_Interval;
        int m_NextTimeout;
        std::string m_Code;
        bool m_IsInterval;
        int m_ID;
        static int s_LastID;
};

#endif //_AVGTimeout_H_
