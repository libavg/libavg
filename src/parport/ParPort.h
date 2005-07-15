//
// $Id$
//

#ifndef _ParPort_H_
#define _ParPort_H_

#include <string>

namespace avg {
    
class ParPort
{
    public:
        ParPort();
        virtual ~ParPort();
        void init(const std::string& theDevice);
        bool setControlLine(int theLine, bool theValue);
        bool getStatusLine(int theLine);
        bool isAvailable();

    private:
        bool frob(int theLines, int theStatus);
        void deinit();
        bool writeControlRegister(unsigned char theStatus);
        
        int         _myFileDescriptor;
        std::string _myDeviceName;
        bool        _isOpen;
};

}

#endif 
