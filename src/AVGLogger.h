//
// $Id$
//

#ifndef _AVGLogger_H_ 
#define _AVGLogger_H_

#include <iostream>

class AVGLogger {
public:
    static AVGLogger* get();
    ~AVGLogger();
   
    void setDestination(std::ostream * pDest);
    void setCategories(int flags);
    void trace(int category, const std::string& msg);

private:    
    AVGLogger();
   
    static AVGLogger* m_pLogger;

    std::ostream * m_pDest;
    int m_Flags;
};
#define AVG_TRACE(category, msg) { \
    std::stringstream tmp(std::stringstream::in | std::stringstream::out); \
    tmp << msg; \
    AVGLogger::get()->trace(category, tmp.str()); \
}

#endif
