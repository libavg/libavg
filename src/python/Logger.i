//
// $Id$
//

%module avg
%{
#include "../base/Logger.h"
%}
%include "std_string.i"

namespace avg {

%nodefault Logger;   // Disable creation of constructor/destructor
class Logger {
public:
    static Logger* get();
    virtual ~Logger();
   
    void setDestination(const std::string& sFName);
    void setCategories(int flags);
    void trace(int category, const std::string& msg);
//    void trace(int category, const char * msg);


    /**
     * Turns off debug output.
     */
    static const long NONE=0;
    /**
     * Outputs data about the display subsystem. Useful for timing/performance
     * measurements.
     */
    static const long BLTS=1;
    /**
     * Outputs performance statistics and frames displayed too late.
     */
    static const long PROFILE=2;
    static const long PROFILE_LATEFRAMES=4;
    
    /**
     * Outputs basic event data.
     */
    static const long EVENTS=8;
    /**
     * Outputs all event data available.
     */
    static const long EVENTS2=16;
    /**
     * Outputs configuration data.
     */
    static const long CONFIG=32;  
    /**
     * Outputs warning messages. Default is on.
     */
    static const long WARNING=64;
    /**
     * Outputs error messages. Can't be shut off.
     */
    static const long ERROR=128;  

    static const long MEMORY=256;
    static const long APP=512;
};

}
