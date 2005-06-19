//
// $Id$
//

#ifndef _Timeout_H_
#define _Timeout_H_

#include <string>

namespace avg {

class Timeout
{
    public:
        Timeout (int time, std::string code, bool isInterval);
        virtual ~Timeout ();

        bool IsReady() const;
        bool IsInterval() const;
        void Fire();
        int GetID() const;
        bool operator <(const Timeout& other) const;

    private:
        long long m_Interval;
        long long m_NextTimeout;
        std::string m_Code;
        bool m_IsInterval;
        int m_ID;
        static int s_LastID;
};

}

#endif //_Timeout_H_
